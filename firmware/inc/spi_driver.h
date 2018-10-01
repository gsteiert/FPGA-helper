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

#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H



/** \brief Transfer descriptor for SPI
 *  Transfer descriptor holds TX and RX buffers
 */
struct spi_xfer {
	/** Pointer to data buffer to TX */
	uint8_t *txbuf;
	/** Pointer to data buffer to RX */
	uint8_t *rxbuf;
	/** Size of data characters to TX & RX */
	uint32_t size;
};


/** \brief Perform the SPI data transfer (TX and RX) in polling way
 *
 *  Activate CS, do TX and RX and deactivate CS. It blocks.
 *
 *  \param[in, out] spi Pointer to the HAL SPI instance.
 *  \param[in] xfer Pointer to the transfer information (\ref spi_xfer).
 *
 *  \retval size Success.
 *  \retval >=0 Timeout, with number of characters transferred.
 *  \retval ERR_BUSY SPI is busy
 */
int32_t spi_m_sync_transfer(struct spi_m_sync_descriptor *spi, const struct spi_xfer *xfer);

#endif // _SPI_DRIVER_H_