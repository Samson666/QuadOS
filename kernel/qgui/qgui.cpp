//Filename			: qgui.cpp
//First created: 16.12.2023
//Last changed		: 
//Description		: Manage the workbench and the windows
 

#include "qgui.h"
#include "printf.h"
#include "log.h"

void test_qwindow_class()
{
   qwindow* qw = new qwindow;
   qw->init(200,300,0);
   kernel_log("Function new qwindow class w:%d,  h:%d, f:%d", qw->width, qw->height, qw->flags);
   delete qw;   
}

// Functionname 	: qcreate_window
// Parameters		: width, height, flags of the new window
// Returns			: id of the new window
// Description		: creates a new qwindow
// Note				: 

int32_t qcreate_window(int16_t width, int16_t height, uint32_t flags)
{
   kernel_log("Function qcreate_window widht=%d, height=%d, flags=%x", width, height, flags);
   qwindow* qw = new qwindow;
   qw->init(width, height, flags);

}