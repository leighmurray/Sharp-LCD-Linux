/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SharpLCD.hpp"

static const bool test = true;

SharpLCD::SharpLCD()
{
	frameInversion = false;
	device = "/dev/spidev1.0";
	speed = 1000000;
	bits = 8;
	mode = SPI_MODE_0;
	lsb = SPI_LSB_FIRST;
	delay = 0xFFFF;
	parse_opts();
	init();
}

static void pabort(const char *s)
{
	perror(s);
	abort();
}

void
SharpLCD::parse_opts()
{
	printf("Mode: %.2X\n", mode);
}

void
SharpLCD::send(uint8_t *buffer, int len)
{
	int ret;

	uint8_t rx[len];

	struct spi_ioc_transfer tr = {
		tr.tx_buf = (unsigned long)buffer,
		tr.rx_buf = (unsigned long)rx,
		tr.len = len,
		tr.delay_usecs = delay,
		tr.speed_hz = speed,
		tr.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

	if (ret < 1)
		pabort("can't send spi message");

	frameInversion = !frameInversion;
}

void
SharpLCD::init(void)
{
	int ret = 0;

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");


	/*
	 * Little Endian
	 */
	ret = ioctl(fd, SPI_IOC_RD_LSB_FIRST, &lsb);
	if (ret == -1)
		pabort("can't set lsb");

	ret = ioctl(fd, SPI_IOC_WR_LSB_FIRST, &lsb);
	if (ret == -1)
		pabort("can't get lsb");


	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
}

void
SharpLCD::clearScreen(void)
{
		const uint8_t vcom = frameInversion ? VCOM_HI : VCOM_LO;
    uint8_t buf[] = {MLCD_CM | vcom, 0x00};
		send(buf, 2);
}

void
SharpLCD::writeLine(uint8_t line_number, uint8_t tx[])
{
	const uint8_t vcom = frameInversion ? VCOM_HI : VCOM_LO;

	// add 4 bytes for the cmd byte, addr byte, and 2 trailer bytes
	uint8_t line[MLCD_BYTES_LINE + 4];

	memcpy(line + 2, tx, MLCD_BYTES_LINE * sizeof(uint8_t));

	//*line >> 24;
	line[0] = MLCD_WR;
	line[1] = line_number;

	send(line, MLCD_BYTES_LINE + 4);
}

void
SharpLCD::writeMultipleLines(void)
{

}

void
SharpLCD::changeVCOM(void)
{
    uint8_t buf[2] = {0, MLCD_NO};

    uint8_t new_vcom = VCOM_LO;
    if (frameInversion) {
        new_vcom |= VCOM_HI;
    }
    frameInversion = !frameInversion; /* toggle frameInversion in
                                       * preparation for the next call */
    buf[0] = new_vcom;

		send(buf, 2);
}
