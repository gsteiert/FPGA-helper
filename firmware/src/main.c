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
 * FPGA Helper Application
 * This is an FPGA programming and communication interface.
 * Programming is done using the UF2 file format through a 
 * drag-n-drop USB mass storage interface.  
 * This is primarily derived from the uf2-samdx1 project:
 * https://github.com/adafruit/uf2-samdx1
 */

#include "uf2.h"

int main(void) {
    led_init();

    logmsg ("Start");

    RGBLED_set_color(0x001010);

    system_init();

    __DMB();
    __enable_irq();

    usart_open();

    RGBLED_set_color(0x100000);
    logmsg("Init USB");

    usb_init();
    
    RGBLED_set_color(0x101010);
    delay(10000);

    while (1) {
        delay(10000);
        RGBLED_set_color(0x100000);
        delay(10000);
        RGBLED_set_color(0x001000);
        delay(10000);
        RGBLED_set_color(0x000010);
        delay(10000);
        RGBLED_set_color(0x000000);
    }
}    