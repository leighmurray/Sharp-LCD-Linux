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
	mode = 3;
	//parse_opts();
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
return;
	while (1) {
		static const struct option lopts[] = {
			{ "device",  1, 0, 'D' },
			{ "speed",   1, 0, 's' },
			{ "delay",   1, 0, 'd' },
			{ "bpw",     1, 0, 'b' },
			{ "loop",    0, 0, 'l' },
			{ "cpha",    0, 0, 'H' },
			{ "cpol",    0, 0, 'O' },
			{ "lsb",     0, 0, 'L' },
			{ "cs-high", 0, 0, 'C' },
			{ "3wire",   0, 0, '3' },
			{ "no-cs",   0, 0, 'N' },
			{ "ready",   0, 0, 'R' },
			{ NULL, 0, 0, 0 },
		};
		int c;

		//c = getopt_long(argc, argv, "D:s:d:b:lHOLC3NR", lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'D':
			device = optarg;
			break;
		case 's':
			speed = atoi(optarg);
			break;
		case 'd':
			delay = atoi(optarg);
			break;
		case 'b':
			bits = atoi(optarg);
			break;
		case 'l':
			mode |= SPI_LOOP;
			break;
		case 'H':
			mode |= SPI_CPHA;
			break;
		case 'O':
			mode |= SPI_CPOL;
			break;
		case 'L':
			mode |= SPI_LSB_FIRST;
			break;
		case 'C':
			mode |= SPI_CS_HIGH;
			break;
		case '3':
			mode |= SPI_3WIRE;
			break;
		case 'N':
			mode |= SPI_NO_CS;
			break;
		case 'R':
			mode |= SPI_READY;
			break;
		default:
		//	print_usage(argv[0]);
			break;
		}
	}
}



void
SharpLCD::print_usage(const char *prog)
{
	{
		printf("Usage: %s [-DsbdlHOLC3]\n", prog);
		puts("  -D --device   device to use (default /dev/spidev1.0)\n"
		     "  -s --speed    max speed (Hz)\n"
		     "  -d --delay    delay (usec)\n"
		     "  -b --bpw      bits per word \n"
		     "  -l --loop     loopback\n"
		     "  -H --cpha     clock phase\n"
		     "  -O --cpol     clock polarity\n"
		     "  -L --lsb      least significant bit first\n"
		     "  -C --cs-high  chip select active high\n"
		     "  -3 --3wire    SI/SO signals shared\n");
		exit(1);
	}
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

	for (ret = 0; ret < len; ret++) {
		if (!(ret % 6))	{
			puts("");
		}
		printf("%.2X ", rx[ret]);
	}
	puts("");
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
