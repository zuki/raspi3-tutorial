/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include "uart.h"
#include "mbox.h"

void main()
{
    // set up serial console
    uart_init();
    // get the board's unique serial number with a mailbox call
    // getserial
    mbox[0] = 4*8;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    //mbox[2] = MBOX_TAG_GETSERIAL;   // get serial number command
    //mbox[2] = MBOX_TAG_GETMODEL;    // get board model
    //mbox[2] = MBOX_TAG_GETREVISION;  // get clock rate
    //mbox[2] = MBOX_TAG_GETCLOCKRATE; // get clock rate
    //mbox[2] = 0x00030004; 
    mbox[2] = 0x00020001; 
    mbox[3] = 8;                    // buffer size
    mbox[4] = 0;                    // reqeust
    mbox[5] = 2;                    // for response
    mbox[6] = 0;
    mbox[7] = MBOX_TAG_LAST;

    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP)) {
        uart_puts("Power state: 0x");
        uart_hex(mbox[5]);
        uart_puts(" 0x");
        uart_hex(mbox[6]);
        uart_puts("\n");
    } else {
        uart_puts("request failed with code: 0x");
        uart_hex(mbox[1]);
        uart_puts("\n");
    }

/*
    mbox[0] = 4*9;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    //mbox[2] = MBOX_TAG_GETSERIAL;   // get serial number command
    //mbox[2] = MBOX_TAG_GETMODEL;    // get board model
    //mbox[2] = MBOX_TAG_GETCLOCKRATE;  // get clock rate
    mbox[2] = MBOX_TAG_SETCLOCKRATE;  // get clock rate
    mbox[3] = 12;                    // buffer size
    mbox[4] = 0;                    // buffer size
    mbox[5] = 2;                    // buffer size
    mbox[6] = 4000000;
    mbox[7] = 1;                    // clear output buffer
    mbox[8] = MBOX_TAG_LAST;

    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP)) {
        uart_hex(mbox[0]);
        uart_puts(" ");
        uart_hex(mbox[1]);
        uart_puts(" ");
        uart_hex(mbox[2]);
        uart_puts(" ");
        uart_hex(mbox[3]);
        uart_puts(" ");
        uart_hex(mbox[4]);
        uart_puts(" ");
        uart_hex(mbox[5]);
        uart_puts(" ");
        uart_hex(mbox[6]);
        uart_puts("\n");
    } else {
        uart_puts("request failed with code: 0x");
        uart_hex(mbox[1]);
        uart_puts("\n");
    }

    mbox[0] = 4*8;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    mbox[2] = MBOX_TAG_GETCLOCKRATEMS; // get clock rate
    mbox[3] = 8;                    // buffer size
    mbox[4] = 0;
    mbox[5] = 2;                    // clear output buffer
    mbox[6] = 0;                    // clear output buffer
    mbox[7] = MBOX_TAG_LAST;

    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP)) {
        uart_hex(mbox[0]);
        uart_puts(" ");
        uart_hex(mbox[1]);
        uart_puts(" ");
        uart_hex(mbox[2]);
        uart_puts(" ");
        uart_hex(mbox[3]);
        uart_puts(" ");
        uart_hex(mbox[4]);
        uart_puts(" ");
        uart_hex(mbox[5]);
        uart_puts(" ");
        uart_hex(mbox[6]);
        uart_puts("\n");
    } else {
        uart_puts("request failed with code: 0x");
        uart_hex(mbox[1]);
        uart_puts("\n");
    }
*/
    // echo everything back
    while(1) {
        uart_send(uart_getc());
    }
}
