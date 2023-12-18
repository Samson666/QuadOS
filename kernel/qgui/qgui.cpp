//Filename			: qgui.cpp
//First created: 16.12.2023
//Last changed		: 
//Description		: Manage the workbench and the windows
 

#include "qgui.h"
#include "printf.h"
#include "log.h"

qgui* QGUI;                //qgui object. Exists exactly one time!
int32_t qwindow_count = 0; //The window counter. Needed to give a new window an ID. For the time beeing have to be used
                           //for the interface (SYSCALL) of the userspace api.


//C Moniker for gui initialization 
void init_qgui(int32_t w, int32_t h)
{
   debug_log("Function init_qgui w:%d, h:%d", w, h);
   QGUI = new qgui;
   QGUI->init(w,h);
   debug_log("init_qgui w:%d, h:%d", QGUI->width, QGUI->height);
}

//C Moniker for creating a new window
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

//print the window list to the console. For debbuging purposes only!
void qgui::print_window_list()
{
   list_node* l;
   l = list_tail->next;
   while(l!=list_head)
   {
      if(l)
      {
            qwindow* w = (qwindow*) l->data;
            if(!l->data)assert_msg(l->data==0,"print window list: no data!");
            else if(w)
               debug_log("list window %d:",w->id);
            else
               debug_log("list window: no window found!");
      }
      else
      {
            debug_log("list window: no node found!");
      }
      l = l->next;
   }
}

//append a window object at the end of the window list
void qgui::append_window(qwindow* w)
{
   
   list_node* newnode = new list_node;
   newnode->data = w;

   list_node* node;
   node = list_tail->next;

   //we have an empty list!
   if(list_tail->next == list_head)
   {
      list_tail->next = newnode;
      newnode->next = list_head;
   }
   else
   {
      node = list_tail->next;
      while(node->next != list_head)
            node = node->next;
      node->next=newnode;
      newnode->next = list_head;
   }
}

//initialize the gui
void qgui::init(int32_t w, int32_t h)
{
   width = w;
   height = h;
   cursor_x = w / 2;
   cursor_y = h / 2;
   prev_cursor_x = cursor_x;
   prev_cursor_y = cursor_y;
   needs_redraw = true;
   list_head = new list_node;
   list_tail = new list_node;
   list_tail->next = list_head;
   list_head->next = NULL;

   register_syscall(SYSCALL_QCREATE_WINDOW, (void*)qcreate_window);

// init_windows();

// testbutton_x = graphics.width - 100;
// testbutton_y = graphics.height - 100;
// testbutton_w = 50;
// testbutton_h = 50;
};

//task loop for our gui
void qgui::gui_task() 
{
// disable_interrupts();
   while (1) 
   {
      if (needs_redraw) 
      {
            needs_redraw = false;
            draw_frame();
      }

      disable_interrupts();
      handle_events();
      enable_interrupts();
   }
};

//draw out window list
void qgui::draw_qwindows()
{

};

//handles the gui events
void qgui::handle_events()
{

};

//draw the whole frame
void qgui::draw_frame()
{
   graphics_fill(COLOR_FRAME_BACKGROUND);  //Frame with given color (00RRGGBB)
   draw_debug_console(0);
};

//draw the debug console
void qgui::draw_debug_console(uint32_t color) 
{
   int index = 0;
   for (int y = 0; y < 50; y++) {
      for (int x = 0; x < 80; x++) 
      {
            u16 c = fake_console_buffer[index];
            graphics_draw_char(c & 0xFF, x * 10, y * 10, color);
            index++;
      }
   }
};

//Initialize the window
void qwindow::init(int16_t w, int16_t h, uint32_t f)
{
   uint32_t fb_bytes = w * h * 4; //size of framebuffer in bytes
   state = 1;
   y = 70;
   x = 85;
   width = w;
   height = h;
   flags = f;
   actual_width = width + 2;
   actual_height = height + WINDOW_TITLE_BAR_HEIGHT + 1;
   framebuffer_size_bytes = fb_bytes;
   fb_shmem_id = sharedmem_create(fb_bytes * (flags & WINDOW_FLAG_DOUBLE_BUFFERED ? 2 : 1), 0);
   framebuffer = (uint32_t*) sharedmem_map(fb_shmem_id, 0);
   shown_buffer = 0;
   owner_task_id = current_task->id;
   adress = (int32_t) this;
   //debug_log("This pointer: %x", id);
};

//draw the window
void qwindow::draw()
{
   // window outline
   graphics_draw_hline(x, y, width + WINDOW_CONTENT_XOFFSET * 2, COLOR_BLACK);
   graphics_draw_hline(x, y + WINDOW_TITLE_BAR_HEIGHT + height, width + WINDOW_CONTENT_XOFFSET * 2, COLOR_BLACK);
   graphics_draw_vline(x, y, height + WINDOW_TITLE_BAR_HEIGHT, COLOR_BLACK);
   graphics_draw_vline(x + width + 1, y, height + WINDOW_TITLE_BAR_HEIGHT, COLOR_BLACK);
};

//insert a new node in the list
list_node* list_node::insert(void* d)
{
   qwindow* w = (qwindow*)d;
   debug_log("insert node right id:%d", w->id);
   list_node* node = new list_node;            //create a new node
   node->data = d;                             //assign data to new node
   node->next = next;                          //point next of the new node to next of the header                     
   next = node;                                //point next of the header to the new node
   return node;                                //return pointer of the new node
};


