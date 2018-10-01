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

/*
 * Bit-banging SPI Driver
 */

uint8_t shift_spi_byte(uint8_t x) {
    uint8_t y = 0;
    for (uint8_t i = 0x80; i != 0; i >>= 1) {
        if (x & i)
            PINOP(BOARD_FLASH_MOSI_PIN, OUTSET);
        else
            PINOP(BOARD_FLASH_MOSI_PIN, OUTCLR);

        PINOP(BOARD_FLASH_SCK_PIN, OUTSET);
        // for (uint8_t j=0; j<25; j++) /* 0.1ms */
        //  __asm__ __volatile__("");

        y <<= 1;
        if ( PINVAL(BOARD_FLASH_MISO_PIN) ) {
            y |= 1;
        }

        PINOP(BOARD_FLASH_SCK_PIN, OUTCLR);
        // for (uint8_t j=0; j<25; j++) /* 0.1ms */
        //  __asm__ __volatile__("");
    }
    return y;
}

int32_t spi_m_sync_transfer(const struct spi_xfer *xfer) {
	int32_t     rc   = 0;
    uint8_t     read_data;
    for (uint32_t i = 0; i < xfer->size; i++) {
        if (xfer->txbuf) {
            read_data = shift_spi_byte(xfer->txbuf[i]);
        } else {
            read_data = shift_spi_byte(0xFF);
        }
        if (xfer->rxbuf) {
            xfer->rxbuf[i] = read_data;
        }
        rc += 1;
    }
	return rc;
}
