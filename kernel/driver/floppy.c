/******************************************************************************
* Copyright (c) 2007 Teemu Voipio                                             *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL    *
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER  *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
******************************************************************************/

#include "util.h"
#include "log.h"
#include "time.h"
#include "interrupts.h"

// standard base address of the primary floppy controller
static const int floppy_base = 0x03f0;
// standard IRQ number for floppy controllers
static const int floppy_irq = 6;

// The registers of interest. There are more, but we only use these here.
enum floppy_registers {
   FLOPPY_DOR  = 2,  // digital output register
   FLOPPY_MSR  = 4,  // master status register, read only
   FLOPPY_FIFO = 5,  // data FIFO, in DMA operation for commands
   FLOPPY_CCR  = 7   // configuration control register, write only
};

// The commands of interest. There are more, but we only use these here.
enum floppy_commands {
   CMD_SPECIFY = 3,            // SPECIFY
   CMD_WRITE_DATA = 5,         // WRITE DATA
   CMD_READ_DATA = 6,          // READ DATA
   CMD_RECALIBRATE = 7,        // RECALIBRATE
   CMD_SENSE_INTERRUPT = 8,    // SENSE INTERRUPT
   CMD_SEEK = 15,              // SEEK
};

static const char * drive_types[8] = {
    "none",
    "360kB 5.25\"",
    "1.2MB 5.25\"",
    "720kB 3.5\"",

    "1.44MB 3.5\"",
    "2.88MB 3.5\"",
    "unknown type",
    "unknown type"
};

// Obviously you'd have this return the data, start drivers or something.
void floppy_detect_drives() {

   outb(0x70, 0x10);
   unsigned drives = inb(0x71);

   kernel_log(" - Floppy drive 0: %s", drive_types[drives >> 4]);
   kernel_log(" - Floppy drive 1: %s", drive_types[drives & 0xf]);

}

void handle_floppy_interrupt()
{
    kernel_log("Floppy interrupt");
}

void init_floppy()
{
    register_isr(IRQ_OFFSET + floppy_irq, handle_floppy_interrupt);
}



//
// The MSR byte: [read-only]
// -------------
//
//  7   6   5    4    3    2    1    0
// MRQ DIO NDMA BUSY ACTD ACTC ACTB ACTA
//
// MRQ is 1 when FIFO is ready (test before read/write)
// DIO tells if controller expects write (1) or read (0)
//
// NDMA tells if controller is in DMA mode (1 = no-DMA, 0 = DMA)
// BUSY tells if controller is executing a command (1=busy)
//
// ACTA, ACTB, ACTC, ACTD tell which drives position/calibrate (1=yes)
//
//

void floppy_write_cmd(int base, char cmd) {
    int i; // do timeout, 60 seconds
    for(i = 0; i < 600; i++) {
        sleep(10); // sleep 10 ms
        if(0x80 & inb(base+FLOPPY_MSR)) {
            return (void) outb(base+FLOPPY_FIFO, cmd);
        }
    }
    kernel_log("floppy_write_cmd: timeout");    
}

unsigned char floppy_read_data(int base) {

    int i; // do timeout, 60 seconds
    for(i = 0; i < 600; i++) {
        sleep(10); // sleep 10 ms
        if(0x80 & inb(base+FLOPPY_MSR)) {
            return inb(base+FLOPPY_FIFO);
        }
    }
    kernel_log("floppy_read_data: timeout");
    return 0; // not reached
}


