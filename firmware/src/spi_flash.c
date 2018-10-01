/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016, 2017 Scott Shawcroft for Adafruit Industries
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
#include "uf2.h"
#include "spi_flash_api.h"
#include "spi_driver.h"

#include <stdint.h>
#include <string.h>

#include "common_commands.h"

// Enable the flash over SPI.
static void flash_enable(void) {
    PINOP(BOARD_FLASH_CS_PIN, OUTCLR);
}

// Disable the flash over SPI.
static void flash_disable(void) {
    PINOP(BOARD_FLASH_CS_PIN, OUTSET);
}


static bool transfer(uint8_t* command, uint32_t command_length, uint8_t* data_in, uint8_t* data_out, uint32_t data_length) {
    struct spi_xfer xfer = { command, NULL, command_length };
    flash_enable();
    int32_t status = spi_m_sync_transfer(&xfer);
    if (status >= 0 && !(data_in == NULL && data_out == NULL)) {
        struct spi_xfer data_xfer = {data_in, data_out, data_length};
        status = spi_m_sync_transfer(&data_xfer);
    }
    flash_disable();
    return status >= 0;
}

static bool transfer_command(uint8_t command, uint8_t* data_in, uint8_t* data_out, uint32_t data_length) {
    return transfer(&command, 1, data_in, data_out, data_length);
}

bool spi_flash_command(uint8_t command) {
    return transfer_command(command, NULL, NULL, 0);
}

bool spi_flash_read_command(uint8_t command, uint8_t* data, uint32_t data_length) {
    return transfer_command(command, NULL, data, data_length);
}

bool spi_flash_write_command(uint8_t command, uint8_t* data, uint32_t data_length) {
    return transfer_command(command, data, NULL, data_length);
}

// Pack the low 24 bits of the address into a uint8_t array.
static void address_to_bytes(uint32_t address, uint8_t* bytes) {
    bytes[0] = (address >> 16) & 0xff;
    bytes[1] = (address >> 8) & 0xff;
    bytes[2] = address & 0xff;
}

bool spi_flash_sector_command(uint8_t command, uint32_t address) {
    uint8_t request[4] = {command, 0x00, 0x00, 0x00};
    address_to_bytes(address, request + 1);
    return transfer(request, 4, NULL, NULL, 0);
}

bool spi_flash_write_data(uint32_t address, uint8_t* data, uint32_t data_length) {
    uint8_t request[4] = {CMD_PAGE_PROGRAM, 0x00, 0x00, 0x00};
    // Write the SPI flash write address into the bytes following the command byte.
    address_to_bytes(address, request + 1);
    struct spi_xfer xfer = { request, NULL, 4 };
    flash_enable();
    int32_t status = spi_m_sync_transfer(&xfer);
    if (status >= 0) {
        struct spi_xfer data_xfer = {data, NULL, data_length};
        status = spi_m_sync_transfer(&data_xfer);
    }
    flash_disable();
    return status >= 0;
}

bool spi_flash_read_data(uint32_t address, uint8_t* data, uint32_t data_length) {
    uint8_t request[4] = {CMD_READ_DATA, 0x00, 0x00, 0x00};
    // Write the SPI flash write address into the bytes following the command byte.
    address_to_bytes(address, request + 1);
    struct spi_xfer xfer = { request, NULL, 4 };
    flash_enable();
    int32_t status = spi_m_sync_transfer(&xfer);
    if (status >= 0) {
        struct spi_xfer data_xfer = {NULL, data, data_length};
        status = spi_m_sync_transfer(&data_xfer);
    }
    flash_disable();
    return status >= 0;
}

void spi_flash_init(void) {
    PINOP(BOARD_FLASH_MOSI_PIN, DIRSET);
    PINOP(BOARD_FLASH_MISO_PIN, DIRCLR);
    PINOP(BOARD_FLASH_SCK_PIN, DIRSET);
    PINOP(BOARD_FLASH_CS_PIN, DIRSET);
    flash_disable();
}

void spi_flash_init_device(const external_flash_device* device) {

}
