/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Steiert Solutions
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define CRYSTALLESS    1

#define VENDOR_NAME "Steiert Solutions"
#define PRODUCT_NAME "FPGA Helper"
#define VOLUME_LABEL "FPGALOADER"
#define INDEX_URL "http://www.steiert.net"
#define BOARD_ID "SAMD21E18A-FPGAhlpr-v0"

#define USB_VID 0x239A
#define USB_PID 0x001E

#define LED_PIN PIN_PA27 // not connected
//#define LED_TX_PIN PIN_PA27
//#define LED_RX_PIN PIN_PB03

#define BOARD_RGBLED_CLOCK_PIN            PIN_PA00
#define BOARD_RGBLED_DATA_PIN             PIN_PA01

#define BOARD_FLASH_MOSI_PIN     PIN_PA08
#define BOARD_FLASH_MISO_PIN     PIN_PA10
#define BOARD_FLASH_SCK_PIN      PIN_PA11
#define BOARD_FLASH_CS_PIN       PIN_PA09

#define BOARD_VUSB_PIN           PIN_PA28

#endif
