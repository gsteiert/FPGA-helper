#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define CRYSTALLESS    1

#define VENDOR_NAME "Adafruit Industries"
#define PRODUCT_NAME "ItsyBitsy M0 Express"
#define VOLUME_LABEL "ITSYLOADER"
#define INDEX_URL "http://adafru.it/3727"
#define BOARD_ID "SAMD21G18A-ItsyBitsy-v0"

#define USB_VID 0x239A
#define USB_PID 0x000F

#define LED_PIN PIN_PA17

#define BOARD_RGBLED_CLOCK_PIN            PIN_PA00
#define BOARD_RGBLED_DATA_PIN             PIN_PA01

#define BOARD_FLASH_MOSI_PIN     PIN_PB22
#define BOARD_FLASH_MISO_PIN     PIN_PB03
#define BOARD_FLASH_SCK_PIN      PIN_PB23
#define BOARD_FLASH_CS_PIN       PIN_PA27

#define BOARD_VUSB_PIN           PIN_PB02

#endif
