#include <QCoreApplication>
#include <logger/log.h>
#include "libusb-1.0/libusb.h"

#include <driver/razerchromacommon.h>
#include <driver/razercommon.h>




int main(int argc, char *argv[])
{
    Logger::init(NULL);

    libusb_context * ctx;
    libusb_init(&ctx);

    libusb_device_handle* dev = libusb_open_device_with_vid_pid(ctx,0x1532,0x0221);
    if(dev == NULL){
        ERR << "Dev is NUL";
        libusb_exit(ctx);
        return 1;
    }

    LOG << "Device opened";

    int r = libusb_claim_interface	(dev, 2);
    if(r != 0){
        LOG << "claim returned "<<r;
        return 1;
    }

    unsigned char direction = '1';
    struct razer_report report;

    uchar rgbdata[21*3];
    uchar randomValues[] = {0,255};
    for(int i = 0; i < 21*3; i++){
        rgbdata[i] = randomValues[i%2];
    }

    /*report = razer_chroma_standard_matrix_effect_spectrum(VARSTORE, BACKLIGHT_LED);
    report.transaction_id.id = 0x3F;

    razer_send_payload(dev, &report);*/

    report = razer_chroma_standard_matrix_effect_custom_frame(NOSTORE); // Possibly could use VARSTORE
    report.transaction_id.id = 0x3F;
    razer_send_payload(dev, &report);

    report = razer_chroma_standard_matrix_set_custom_frame(0,0,21,rgbdata);
    report.transaction_id.id = 0x3F;  // TODO move to a usb_device variable
    razer_send_payload(dev, &report);


    Sleep(10000);

    libusb_release_interface(dev,2);
    libusb_close(dev);
    libusb_exit(ctx);
    // 1532:0221 (
    return 0;
}
