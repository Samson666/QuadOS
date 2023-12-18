//Filename			: qgui.cpp
//First created: 16.12.2023
//Last changed		: 
//Description		: Manage the workbench and the windows
 

#include "qgui.h"
#include "printf.h"
#include "log.h"
qgui* QGUI;
int32_t qwindow_count = 0;


void test_qwindow_class()
{
   qwindow* qw;
   qw = new qwindow;
   qw->init(200,300,0);
   debug_log("Function new qwindow class w:%d,  h:%d, f:%d", qw->width, qw->height, qw->flags);
   init_qgui(1024,768); 
}

// Functionname 	: qcreate_window
// Parameters		: width, height, flags of the new window
// Returns			: id of the new window
// Description		: creates a new qwindow
// Note				: 

void init_qgui(int32_t w, int32_t h)
{
   debug_log("Function init_qgui w:%d, h:%d", w, h);
   QGUI = new qgui;
   QGUI->init(w,h);
   debug_log("init_qgui w:%d, h:%d", QGUI->width, QGUI->height);
}

int32_t qcreate_window(int16_t width, int16_t height, uint32_t flags)
{
   
   qwindow* qw = new qwindow;
   qw->init(width, height, flags);
   qw->id = qwindow_count++;
   QGUI->append_window(qw);
   debug_log("Function qcreate_window after inserting to list id:%d", qw->id);
   debug_log("Function qcreate_window id:%d, adress:%x, widht=%d, height=%d, flags=%x, guiw:%d, guih:%d", qw->id, qw->adress, height, flags, QGUI->width, QGUI->height);
   QGUI->print_window_list();
   qw->draw();
   return(qw->id);
}


