#pragma once


void floppy_detect_drives();
void init_floppy();
void handle_floppy_interrupt();
void floppy_motor(int base, int onoff);
void task_test_floppy();

