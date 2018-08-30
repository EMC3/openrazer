/*
 * Copyright (c) 2015 Tim Theede <pez2001@voyagerproject.de>
 *               2015 Terry Cain <terry@terrys-home.co.uk>
 *
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef DRIVER_RAZERCOMMON_H_
#define DRIVER_RAZERCOMMON_H_

#include "libusb-1.0/libusb.h"



#define DRIVER_VERSION "2.3.1"
#define DRIVER_LICENSE "GPL v2"
#define DRIVER_AUTHOR "Terry Cain <terry@terrys-home.co.uk>"


// Macro to create device files
#define CREATE_DEVICE_FILE(dev, type) \
do { \
    if(device_create_file(dev, type)) { \
        goto exit_free; \
    } \
} while (0)


/* Each USB report has 90 bytes*/
#define RAZER_USB_REPORT_LEN 0x5A

// LED STATE
#define OFF 0x00
#define ON  0x01

// LED STORAGE Options
#define NOSTORE          0x00
#define VARSTORE         0x01

// LED definitions
#define SCROLL_WHEEL_LED  0x01
#define BATTERY_LED       0x03
#define LOGO_LED          0x04
#define BACKLIGHT_LED     0x05
#define MACRO_LED         0x07
#define GAME_LED          0x08
#define RED_PROFILE_LED   0x0C
#define GREEN_PROFILE_LED 0x0D
#define BLUE_PROFILE_LED  0x0E

// LED Effect definitions
#define LED_STATIC           0x00
#define LED_BLINKING         0x01
#define LED_PULSATING        0x02
#define LED_SPECTRUM_CYCLING 0x04

// Report Responses
#define RAZER_CMD_BUSY          0x01
#define RAZER_CMD_SUCCESSFUL    0x02
#define RAZER_CMD_FAILURE       0x03
#define RAZER_CMD_TIMEOUT       0x04
#define RAZER_CMD_NOT_SUPPORTED 0x05


#define RAZER_BLACKWIDOW_REPORT_LEN 0x5A

#define RAZER_BLACKWIDOW_CHROMA_WAVE_DIRECTION_LEFT 2
#define RAZER_BLACKWIDOW_CHROMA_WAVE_DIRECTION_RIGHT 1

#define RAZER_BLACKWIDOW_CHROMA_CHANGE_EFFECT 0x0A

#define RAZER_BLACKWIDOW_CHROMA_EFFECT_NONE 0
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_WAVE 1
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_REACTIVE 2
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_BREATH 3
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_SPECTRUM 4
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_CUSTOM 5 // draw frame
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_STATIC 6
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_CLEAR_ROW 8

#define RAZER_BLACKWIDOW_ULTIMATE_2016_EFFECT_STARLIGHT 0x19


#define RAZER_BLACKWIDOW_CHROMA_EFFECT_SET_KEYS 9 //update profile needs to be called after setting keys to reflect changes
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_RESET 10
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_UNKNOWN 11
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_UNKNOWN2 12
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_UNKNOWN3 13
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_UNKNOWN4 14


#define RAZER_BLACKWIDOW_CHROMA_ROW_LEN 0x16
#define RAZER_BLACKWIDOW_CHROMA_ROWS_NUM 6

#define RAZER_FIREFLY_ROW_LEN 0x0F
#define RAZER_FIREFLY_ROWS_NUM 1


#define RAZER_STEALTH_ROW_LEN 0x10
#define RAZER_STEALTH_ROWS_NUM 6



#define RAZER_BLACKWIDOW_CHROMA_WAIT_MS 1
#define RAZER_BLACKWIDOW_CHROMA_WAIT_MIN_US 600
#define RAZER_BLACKWIDOW_CHROMA_WAIT_MAX_US 800

#define RAZER_FIREFLY_WAIT_MIN_US 900
#define RAZER_FIREFLY_WAIT_MAX_US 1000

struct razer_report;

struct razer_rgb {
    unsigned char r,g,b;
};

union transaction_id_union {
    unsigned char id;
    struct transaction_parts {
        unsigned char device : 3;
        unsigned char id : 5;
    } parts;
};

union command_id_union {
    unsigned char id;
    struct command_id_parts {
        unsigned char direction : 1;
        unsigned char id : 7;
    } parts;
};

/* Status:
 * 0x00 New Command
 * 0x01 Command Busy
 * 0x02 Command Successful
 * 0x03 Command Failure
 * 0x04 Command No Response / Command Timeout
 * 0x05 Command Not Support
 *
 * Transaction ID used to group request-response, device useful when multiple devices are on one usb
 * Remaining Packets is the number of remaining packets in the sequence
 * Protocol Type is always 0x00
 * Data Size is the size of payload, cannot be greater than 80. 90 = header (8B) + data + CRC (1B) + Reserved (1B)
 * Command Class is the type of command being issued
 * Command ID is the type of command being send. Direction 0 is Host->Device, Direction 1 is Device->Host. AKA Get LED 0x80, Set LED 0x00
 *
 * */

struct razer_report {
    unsigned char status;
    union transaction_id_union transaction_id; /* */
    unsigned short remaining_packets; /* Big Endian */
    unsigned char protocol_type; /*0x0*/
    unsigned char data_size;
    unsigned char command_class;
    union command_id_union command_id;
    unsigned char arguments[80];
    unsigned char crc;/*xor'ed bytes of report*/
    unsigned char reserved; /*0x0*/
};

struct razer_key_translation {
    uint16_t from;
    uint16_t to;
    uint8_t flags;
};

struct razer_report razer_send_payload(libusb_device_handle *usb_dev, struct razer_report *request_report);
int razer_get_report(libusb_device_handle *usb_dev, struct razer_report *request_report, struct razer_report *response_report);

int razer_send_control_msg(libusb_device_handle *usb_dev,void const *data, unsigned int report_index, unsigned long wait_min, unsigned long wait_max);
int razer_get_usb_response(libusb_device_handle *usb_dev, unsigned int report_index, struct razer_report* request_report, unsigned int response_index, struct razer_report* response_report, unsigned long wait_min, unsigned long wait_max);
unsigned char razer_calculate_crc(struct razer_report *report);
struct razer_report get_razer_report(unsigned char command_class, unsigned char command_id, unsigned char data_size);
struct razer_report get_empty_razer_report(void);
void print_erroneous_report(struct razer_report* report, char* driver_name, char* message);

// Convenience functions
unsigned char clamp_u8(unsigned char value, unsigned char min, unsigned char max);
unsigned short clamp_u16(unsigned short value, unsigned short min, unsigned short max);

#endif /* DRIVER_RAZERCOMMON_H_ */
