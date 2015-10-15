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


#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#ifndef __SHARP_LCD_HPP__
#define __SHARP_LCD_HPP__

/**
 * This driver is meant for the monochrome LCD display (model
 * no: LS013B4DN04) from Sharp.
 *
 * The LCD has the following pixel dimensions: width=96pixels,
 * height=96pixels. This is a monochrome display with an inbuilt
 * memory of 1 bit per pixel. If a pixel-bit is set to one, the
 * corresponding pixel will show as black.
 *
 * The LCD memory is accessible to the micro-controller only through a
 * serial interface; and <i>only for write operations</i>. It is
 * necessary for the application to maintain its own frame-buffer
 * memory in the micro-controller's SRAM (see fb_alloc())---the
 * application is not restricted to a single framebuffer; if SRAM size
 * permits, multiple buffers may be employed. In order to update the
 * LCD, the application first draws (bitmaps or text) into some
 * framebuffer memory, and then flushes the framebuffer to the LCD
 * over the serial interface.
 */

class SharpLCD {
public:
    SharpLCD();

    void clearScreen(void);

    void writeLine(uint8_t line_number, uint8_t tx[]);

    void writeMultipleLines();

    void changeVCOM(void);

    void print_usage(const char *prog);

private:
    /**
     * Helper function to write out a buffer onto the LCD's SPI channel.
     */
    void send(uint8_t *buffer, int len);

    void init(void);
    void parse_opts();

    // LCD commands - Note: the bits are reversed per the memory LCD data
    // sheets because of the order the bits are shifted out in the SPI
    // port.
    static const uint8_t MLCD_WR = 0x80; //MLCD write line command
    static const uint8_t MLCD_CM = 0x20; //MLCD clear memory command
    static const uint8_t MLCD_NO = 0x0; //MLCD NOP command (used to switch VCOM)

    static const uint16_t TRAILER_LONG = 0x0000;
    static const uint8_t TRAILER_SHORT = 0x00;

    //LCD resolution
    static const int MLCD_XRES = 96; //pixels per horizontal line
    static const int MLCD_YRES = 96; //pixels per vertical line
    static const int MLCD_BYTES_LINE = MLCD_XRES / 8; //number of bytes in a line
    static const int MLCD_BUF_SIZE = MLCD_YRES * MLCD_BYTES_LINE;

    //defi nes the VCOM bit in the command word that goes to the LCD
    static const uint8_t VCOM_HI = 0x40;
    static const uint8_t VCOM_LO = 0x00;

    bool frameInversion;
    uint8_t bits;
    uint32_t speed;
    uint16_t delay;
    uint8_t mode;
    uint8_t lsb;

    const char *device;

    int fd;
};

#endif /* #ifndef __SHARP_LCD_HPP__ */
