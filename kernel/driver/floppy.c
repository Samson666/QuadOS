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
#include "floppy.h"

// standard base address of the primary floppy controller
static const int floppy_base = 0x03f0;
// standard IRQ number for floppy controllers
static const int floppy_irq = 6;

enum { floppy_motor_off = 0, floppy_motor_on, floppy_motor_wait };

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
    FLOPPYDISKIRQ = 1;
    kernel_log("Floppy interrupt");
}

void init_floppy()
{
    //FLOPPYDISKIRQ = 0;
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
    kernel_log("floppy write cmd");
    int i; // do timeout, 60 seconds
    for(i = 0; i < 600; i++) {
        for(int i=0; i<1000000; i++); // sleep 10 ms
        if(0x80 & inb(base+FLOPPY_MSR)) {
            return (void) outb(base+FLOPPY_FIFO, cmd);
        }
    }
    kernel_log("floppy_write_cmd: timeout");    
}

unsigned char floppy_read_data(int base) {

    int i; // do timeout, 60 seconds
    for(i = 0; i < 600; i++) {
        for(int i=0; i<1000000; i++); // sleep 10 ms
        if(0x80 & inb(base+FLOPPY_MSR)) {
            return inb(base+FLOPPY_FIFO);
        }
    }
    kernel_log("floppy_read_data: timeout");
    return 0; // not reached
}

void floppy_check_interrupt(int base, int *st0, int *cyl) {
    
    floppy_write_cmd(base, CMD_SENSE_INTERRUPT);

    *st0 = floppy_read_data(base);
    *cyl = floppy_read_data(base);
}

// Move to cylinder 0, which calibrates the drive..
int floppy_calibrate(int base) {

    int i, st0, cyl = -1; // set to bogus cylinder

    floppy_motor(base, floppy_motor_on);

    for(i = 0; i < 10; i++) {
        // Attempt to positions head to cylinder 0
        floppy_write_cmd(base, CMD_RECALIBRATE);
        floppy_write_cmd(base, 0); // argument is drive, we only support 0

        floppydisk_wait_irq();
        floppy_check_interrupt(base, &st0, &cyl);
        
        if(st0 & 0xC0) {
            static const char * status[] =
            { 0, "error", "invalid", "drive" };
            kernel_log("floppy_calibrate: status = %s\n", status[st0 >> 6]);
            continue;
        }

        if(!cyl) { // found cylinder 0 ?
            floppy_motor(base, floppy_motor_off);
            return 0;
        }
    }

    kernel_log("floppy_calibrate: 10 retries exhausted\n");
    floppy_motor(base, floppy_motor_off);
    return -1;
}


int floppy_reset(int base) {

    outb(base + FLOPPY_DOR, 0x00); // disable controller
    outb(base + FLOPPY_DOR, 0x0C); // enable controller

    floppydisk_wait_irq(); // sleep until interrupt occurs

    {
        int st0, cyl; // ignore these here..
        floppy_check_interrupt(base, &st0, &cyl);
    }

    // set transfer speed 500kb/s
    outb(base + FLOPPY_CCR, 0x00);

    //  - 1st byte is: bits[7:4] = steprate, bits[3:0] = head unload time
    //  - 2nd byte is: bits[7:1] = head load time, bit[0] = no-DMA
    // 
    //  steprate    = (8.0ms - entry*0.5ms)*(1MB/s / xfer_rate)
    //  head_unload = 8ms * entry * (1MB/s / xfer_rate), where entry 0 -> 16
    //  head_load   = 1ms * entry * (1MB/s / xfer_rate), where entry 0 -> 128
    //
    floppy_write_cmd(base, CMD_SPECIFY);
    floppy_write_cmd(base, 0xdf); /* steprate = 3ms, unload time = 240ms */
    floppy_write_cmd(base, 0x02); /* load time = 16ms, no-DMA = 0 */

    // it could fail...
    if(floppy_calibrate(base)) return -1;
    
}

//
// The DOR byte: [write-only]
// -------------
//
//  7    6    5    4    3   2    1   0
// MOTD MOTC MOTB MOTA DMA NRST DR1 DR0
//
// DR1 and DR0 together select "current drive" = a/00, b/01, c/10, d/11
// MOTA, MOTB, MOTC, MOTD control motors for the four drives (1=on)
//
// DMA line enables (1 = enable) interrupts and DMA
// NRST is "not reset" so controller is enabled when it's 1
//

static volatile int floppy_motor_ticks = 0;
static volatile int floppy_motor_state = 0;

void floppy_motor(int base, int onoff) {

    kernel_log("floppy motor");
    if(onoff) {
        if(!floppy_motor_state) {
            // need to turn on
            outb(base + FLOPPY_DOR, 0x1c);
            for(int i=0; i<10000000; i++); // wait 500 ms = hopefully enough for modern drives
        }
        floppy_motor_state = floppy_motor_on;
    } else {
        if(floppy_motor_state == floppy_motor_wait) {
            kernel_log("floppy_motor: strange, fd motor-state already waiting..\n");
        }
        floppy_motor_ticks = 300; // 3 seconds, see floppy_timer() below
        floppy_motor_state = floppy_motor_wait;
    }
    kernel_log("floppy motor end");
}

void floppy_motor_kill(int base) {
    outb(base + FLOPPY_DOR, 0x0c);
    floppy_motor_state = floppy_motor_off;
}

//
// THIS SHOULD BE STARTED IN A SEPARATE THREAD.
//
//
void floppy_timer() {
    while(1) {
        // sleep for 500ms at a time, which gives around half
        // a second jitter, but that's hardly relevant here.
        for(int i=0; i<50000000; i++);
        if(floppy_motor_state == floppy_motor_wait) {
            floppy_motor_ticks -= 50;
            if(floppy_motor_ticks <= 0) {
                floppy_motor_kill(floppy_base);
            }
        }
    }
}

// Seek for a given cylinder, with a given head
int floppy_seek(int base, unsigned cyli, int head) {

    kernel_log("floppy seek");

    unsigned i, st0, cyl = -1; // set to bogus cylinder

    floppy_motor(base, floppy_motor_on);

    for(i = 0; i < 10; i++) {
        // Attempt to position to given cylinder
        // 1st byte bit[1:0] = drive, bit[2] = head
        // 2nd byte is cylinder number
        floppy_write_cmd(base, CMD_SEEK);
        floppy_write_cmd(base, head<<2);
        floppy_write_cmd(base, cyli);

        kernel_log("floppy seek before wait irq");
        kernel_log("floppy seek FLOPPYDISKIRQ = %d", FLOPPYDISKIRQ);
        floppydisk_wait_irq();
        kernel_log("floppy seek after wait irq");
        floppy_check_interrupt(base, &st0, &cyl);

        if(st0 & 0xC0) {
            static const char * status[] =
            { "normal", "error", "invalid", "drive" };
            kernel_log("floppy_seek: status = %s\n", status[st0 >> 6]);
            continue;
        }

        if(cyl == cyli) {
            floppy_motor(base, floppy_motor_off);
            return 0;
        }

    }

    kernel_log("floppy_seek: 10 retries exhausted\n");
    floppy_motor(base, floppy_motor_off);
    return -1;
}

// Used by floppy_dma_init and floppy_do_track to specify direction
typedef enum {
    floppy_dir_read = 1,
    floppy_dir_write = 2
} floppy_dir;


// we statically reserve a totally uncomprehensive amount of memory
// must be large enough for whatever DMA transfer we might desire
// and must not cross 64k borders so easiest thing is to align it
// to 2^N boundary at least as big as the block
#define floppy_dmalen 0x4800
static const char floppy_dmabuf[floppy_dmalen]
                  __attribute__((aligned(0x8000)));

static void floppy_dma_init(floppy_dir dir) {

    union {
        unsigned char b[4]; // 4 bytes
        unsigned long l;    // 1 long = 32-bit
    } a, c; // address and count

    a.l = (unsigned) &floppy_dmabuf;
    c.l = (unsigned) floppy_dmalen - 1; // -1 because of DMA counting

    // check that address is at most 24-bits (under 16MB)
    // check that count is at most 16-bits (DMA limit)
    // check that if we add count and address we don't get a carry
    // (DMA can't deal with such a carry, this is the 64k boundary limit)
    if((a.l >> 24) || (c.l >> 16) || (((a.l&0xffff)+c.l)>>16)) {
        kernel_log("floppy_dma_init: static buffer problem\n");
    }

    unsigned char mode;
    switch(dir) {
        // 01:0:0:01:10 = single/inc/no-auto/to-mem/chan2
        case floppy_dir_read:  mode = 0x46; break;
        // 01:0:0:10:10 = single/inc/no-auto/from-mem/chan2
        case floppy_dir_write: mode = 0x4a; break;
        default: kernel_log("floppy_dma_init: invalid direction");
                 return; // not reached, please "mode user uninitialized"
    }

    outb(0x0a, 0x06);   // mask chan 2

    outb(0x0c, 0xff);   // reset flip-flop
    outb(0x04, a.b[0]); //  - address low byte
    outb(0x04, a.b[1]); //  - address high byte

    outb(0x81, a.b[2]); // external page register

    outb(0x0c, 0xff);   // reset flip-flop
    outb(0x05, c.b[0]); //  - count low byte
    outb(0x05, c.b[1]); //  - count high byte

    outb(0x0b, mode);   // set mode (see above)

    outb(0x0a, 0x02);   // unmask chan 2
}

// This monster does full cylinder (both tracks) transfer to
// the specified direction (since the difference is small).
//
// It retries (a lot of times) on all errors except write-protection
// which is normally caused by mechanical switch on the disk.
//
int floppy_do_track(int base, unsigned cyl, floppy_dir dir) {
    
    kernel_log("floppy do track");
    // transfer command, set below
    unsigned char cmd;

    // Read is MT:MF:SK:0:0:1:1:0, write MT:MF:0:0:1:0:1
    // where MT = multitrack, MF = MFM mode, SK = skip deleted
    // 
    // Specify multitrack and MFM mode
    static const int flags = 0xC0;
    switch(dir) {
        case floppy_dir_read:
            cmd = CMD_READ_DATA | flags;
            break;
        case floppy_dir_write:
            cmd = CMD_WRITE_DATA | flags;
            break;
        default: 

            kernel_log("floppy_do_track: invalid direction");
            return 0; // not reached, but pleases "cmd used uninitialized"
    }

    // seek both heads
    if(floppy_seek(base, cyl, 0)) return -1;
    if(floppy_seek(base, cyl, 1)) return -1;

    int i;
    for(i = 0; i < 20; i++) {
        floppy_motor(base, floppy_motor_on);

        // init dma..
        floppy_dma_init(dir);

        //timer_sleep(10); // give some time (100ms) to settle after the seeks
        kernel_log("floppy do track before sleep");
        for(int i=0; i<10000000; i++);
        kernel_log("floppy do track after sleep");

        floppy_write_cmd(base, cmd);  // set above for current direction
        floppy_write_cmd(base, 0);    // 0:0:0:0:0:HD:US1:US0 = head and drive
        floppy_write_cmd(base, cyl);  // cylinder
        floppy_write_cmd(base, 0);    // first head (should match with above)
        floppy_write_cmd(base, 1);    // first sector, strangely counts from 1
        floppy_write_cmd(base, 2);    // bytes/sector, 128*2^x (x=2 -> 512)
        floppy_write_cmd(base, 18);   // number of tracks to operate on
        floppy_write_cmd(base, 0x1b); // GAP3 length, 27 is default for 3.5"
        floppy_write_cmd(base, 0xff); // data length (0xff if B/S != 0)
        
        kernel_log("floppy do track before wait irq");
        kernel_log("floppy do track FLOPPYDISKIRQ = %d", FLOPPYDISKIRQ);

        floppydisk_wait_irq(); // don't SENSE_INTERRUPT here!

        kernel_log("floppy do track after wait irq");

        // first read status information
        unsigned char st0, st1, st2, rcy, rhe, rse, bps;
        st0 = floppy_read_data(base);
        st1 = floppy_read_data(base);
        st2 = floppy_read_data(base);
        /*
         * These are cylinder/head/sector values, updated with some
         * rather bizarre logic, that I would like to understand.
         *
         */
        rcy = floppy_read_data(base);
        rhe = floppy_read_data(base);
        rse = floppy_read_data(base);
        // bytes per sector, should be what we programmed in
        bps = floppy_read_data(base);

        int error = 0;

        if(st0 & 0xC0) {
            static const char * status[] =
            { 0, "error", "invalid command", "drive not ready" };
            kernel_log("floppy_do_sector: status = %s\n", status[st0 >> 6]);
            error = 1;
        }
        if(st1 & 0x80) {
            kernel_log("floppy_do_sector: end of cylinder\n");
            error = 1;
        }
        if(st0 & 0x08) {
            kernel_log("floppy_do_sector: drive not ready\n");
            error = 1;
        }
        if(st1 & 0x20) {
            kernel_log("floppy_do_sector: CRC error\n");
            error = 1;
        }
        if(st1 & 0x10) {
            kernel_log("floppy_do_sector: controller timeout\n");
            error = 1;
        }
        if(st1 & 0x04) {
            kernel_log("floppy_do_sector: no data found\n");
            error = 1;
        }
        if((st1|st2) & 0x01) {
            kernel_log("floppy_do_sector: no address mark found\n");
            error = 1;
        }
        if(st2 & 0x40) {
            kernel_log("floppy_do_sector: deleted address mark\n");
            error = 1;
        }
        if(st2 & 0x20) {
            kernel_log("floppy_do_sector: CRC error in data\n");
            error = 1;
        }
        if(st2 & 0x10) {
            kernel_log("floppy_do_sector: wrong cylinder\n");
            error = 1;
        }
        if(st2 & 0x04) {
            kernel_log("floppy_do_sector: uPD765 sector not found\n");
            error = 1;
        }
        if(st2 & 0x02) {
            kernel_log("floppy_do_sector: bad cylinder\n");
            error = 1;
        }
        if(bps != 0x2) {
            kernel_log("floppy_do_sector: wanted 512B/sector, got %d", (1<<(bps+7)));
            error = 1;
        }
        if(st1 & 0x02) {
            kernel_log("floppy_do_sector: not writable\n");
            error = 2;
        }

        if(!error) {
            floppy_motor(base, floppy_motor_off);
            return 0;
        }
        if(error > 1) {
            kernel_log("floppy_do_sector: not retrying..\n");
            floppy_motor(base, floppy_motor_off);
            return -2;
        }
    }

    kernel_log("floppy_do_sector: 20 retries exhausted\n");
    floppy_motor(base, floppy_motor_off);
    return -1;

}

int floppy_read_track(int base, unsigned cyl) {
    kernel_log("floppy read track");
    return floppy_do_track(base, cyl, floppy_dir_read);
}

int floppy_write_track(int base, unsigned cyl) {
    return floppy_do_track(base, cyl, floppy_dir_write);
}

void task_test_floppy()
{
    kernel_log("task test floppy");
    floppy_read_track(floppy_base, 0);
    while(1);
}