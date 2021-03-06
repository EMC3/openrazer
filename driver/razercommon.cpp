/*
 * Copyright (c) 2015 Tim Theede <pez2001@voyagerproject.de>
 *               2015 Terry Cain <terry@terrys-home.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */
#include "libusb-1.0/libusb.h"
#include <logger/log.h>
#include <unistd.h>
#include "razercommon.h"

void* memdup(const void* mem, size_t size) {
   void* out = malloc(size);

   if(out != NULL)
       memcpy(out, mem, size);

   return out;
}

int razer_get_report(libusb_device_handle *usb_dev, struct razer_report *request_report, struct razer_report *response_report)
{
    uint report_index;
    uint response_index;

    report_index = 0x01;
    response_index = 0x01;

    return razer_get_usb_response(usb_dev, report_index, request_report, response_index, response_report, RAZER_BLACKWIDOW_CHROMA_WAIT_MIN_US, RAZER_BLACKWIDOW_CHROMA_WAIT_MAX_US);
}

struct razer_report razer_send_payload(libusb_device_handle *usb_dev, struct razer_report *request_report)
{
    int retval = -1;
    struct razer_report response_report;

    request_report->crc = razer_calculate_crc(request_report);

    retval = razer_get_report(usb_dev, request_report, &response_report);

    if(retval == 0) {
        // Check the packet number, class and command are the same
        if(response_report.remaining_packets != request_report->remaining_packets ||
           response_report.command_class != request_report->command_class ||
           response_report.command_id.id != request_report->command_id.id) {
            print_erroneous_report(&response_report, "razerkbd", "Response doesnt match request");
//		} else if (response_report.status == RAZER_CMD_BUSY) {
//			print_erroneous_report(&response_report, "razerkbd", "Device is busy");
        } else if (response_report.status == RAZER_CMD_FAILURE) {
            print_erroneous_report(&response_report, "razerkbd", "Command failed");
        } else if (response_report.status == RAZER_CMD_NOT_SUPPORTED) {
            print_erroneous_report(&response_report, "razerkbd", "Command not supported");
        } else if (response_report.status == RAZER_CMD_TIMEOUT) {
            print_erroneous_report(&response_report, "razerkbd", "Command timed out");
        }
    } else {
        print_erroneous_report(&response_report, "razerkbd", "Invalid Report Length");
    }

    return response_report;
}

/**
 * Send USB control report to the keyboard
 * USUALLY index = 0x02
 * FIREFLY is 0
 */
int razer_send_control_msg(libusb_device_handle *usb_dev,void const *data, uint report_index, ulong wait_min, ulong wait_max)
{
    uint request = 0x09;
    uint request_type = (0x01 << 5) | 1 | 0; // 0x21
    uint value = 0x300;
    uint size = RAZER_USB_REPORT_LEN;
    char *buf;
    int len;

    buf = (char*)memdup(data, size);
    if (buf == NULL)
        return -ENOMEM;

    // Send usb control message
    len = libusb_control_transfer(usb_dev,
                                  request_type, // RequestType
                                  request,      // Request
                                  value,        // Value
                                  report_index, // Index
                                  (uchar*)buf,          // Data
                                  size,         // Length
                                  100);

    // Wait
    usleep(wait_min);

    free(buf);
    if(len!=size)
        WARN<< "razer driver: Device data transfer failed."<<len;

    return ((len < 0) ? len : ((len != size) ? -EIO : 0));
}

/**
 * Get a response from the razer device
 *
 * Makes a request like normal, this must change a variable in the device as then we
 * tell it give us data and it gives us a report.
 *
 * Supported Devices:
 *   Razer Chroma
 *   Razer Mamba
 *   Razer BlackWidow Ultimate 2013*
 *   Razer Firefly*
 *
 * Request report is the report sent to the device specifing what response we want
 * Response report will get populated with a response
 *
 * Returns 0 when successful, 1 if the report length is invalid.
 */
int razer_get_usb_response(libusb_device_handle *usb_dev, uint report_index, struct razer_report* request_report, uint response_index, struct razer_report* response_report, ulong wait_min, ulong wait_max)
{
    uint request = 1; // 0x01
    uint request_type =  0xA1;
    uint value = 0x300;

    uint size = RAZER_USB_REPORT_LEN; // 0x90
    int len;
    int retval;
    int result = 0;
    char *buf;

    buf = (char*)malloc(sizeof(struct razer_report));
    if (buf == NULL)
        return -ENOMEM;

    // Send the request to the device.
    // TODO look to see if index needs to be different for the request and the response
    retval = razer_send_control_msg(usb_dev, request_report, report_index, wait_min, wait_max);

    // Now ask for reponse
    len = libusb_control_transfer(usb_dev,
                          request_type,    // RequestType
                                  request,         // Request
                          value,           // Value
                          response_index,  // Index
                          (uchar*)buf,             // Data
                          size,
                          100);

    usleep(wait_min);

    memcpy(response_report, buf, sizeof(struct razer_report));

    free(buf);

    // Error if report is wrong length
    if(len != 90) {
        WARN << "razer driver: Invalid USB repsonse. USB Report length: "<<len;
        result = 1;
    }

    return result;
}

/**
 * Calculate the checksum for the usb message
 *
 * Checksum byte is stored in the 2nd last byte in the messages payload.
 * The checksum is generated by XORing all the bytes in the report starting
 * at byte number 2 (0 based) and ending at byte 88.
 */
unsigned char razer_calculate_crc(struct razer_report *report)
{
    /*second to last byte of report is a simple checksum*/
    /*just xor all bytes up with overflow and you are done*/
    unsigned char crc = 0;
    unsigned char *_report = (unsigned char*)report;

    unsigned int i;
    for(i = 2; i < 88; i++) {
        crc ^= _report[i];
    }

    return crc;
}

/**
 * Get initialised razer report
 */
struct razer_report get_razer_report(unsigned char command_class, unsigned char command_id, unsigned char data_size)
{
    struct razer_report new_report;
    memset(&new_report, 0, sizeof(struct razer_report));

    new_report.status = 0x00;
    new_report.transaction_id.id = 0xFF;
    new_report.remaining_packets = 0x00;
    new_report.protocol_type = 0x00;
    new_report.command_class = command_class;
    new_report.command_id.id = command_id;
    new_report.data_size = data_size;

    return new_report;
}

/**
 * Get empty razer report
 */
struct razer_report get_empty_razer_report(void)
{
    struct razer_report new_report;
    memset(&new_report, 0, sizeof(struct razer_report));

    return new_report;
}

/**
 * Print report to syslog
 */
void print_erroneous_report(struct razer_report* report, char* driver_name, char* message)
{
    printf("%s: %s. Start Marker: %02x id: %02x Num Params: %02x Reserved: %02x Command: %02x Params: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x .\n",
           driver_name,
           message,
           report->status,
           report->transaction_id.id,
           report->data_size,
           report->command_class,
           report->command_id.id,
           report->arguments[0], report->arguments[1], report->arguments[2], report->arguments[3], report->arguments[4], report->arguments[5],
           report->arguments[6], report->arguments[7], report->arguments[8], report->arguments[9], report->arguments[10], report->arguments[11],
           report->arguments[12], report->arguments[13], report->arguments[14], report->arguments[15]);
}

/**
 * Clamp a value to a min,max
 */
unsigned char clamp_u8(unsigned char value, unsigned char min, unsigned char max)
{
    if(value > max)
        return max;
    if(value < min)
        return min;
    return value;
}
unsigned short clamp_u16(unsigned short value, unsigned short min, unsigned short max)
{
    if(value > max)
        return max;
    if(value < min)
        return min;
    return value;
}



















