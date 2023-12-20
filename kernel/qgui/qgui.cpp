//Filename			: qgui.cpp
//First created: 16.12.2023
//Last changed		: 
//Description		: Manage the workbench and the windows
 

#include "qgui.h"
#include "printf.h"
#include "log.h"
#include "mouse.h"
#include "events.h"
#include "tasks.h"
#include "console.h"
#include "graphics.h"
#include "gui.h"
#include "time.h"
#include "physalloc.h"
#include "defs.h"

#ifdef NEWGUI
   #include "res/cursor_img.h"
#endif

qgui* QGUI;          //qgui object. Exists exactly one time!
int32_t qwindow_count = 0; //The window counter. Needed to give a new window an ID. For the time beeing have to be used
                           //as an interface (SYSCALL) of the userspace api.

//-----------------------------------------------C Moniker----------------------------------------------------

//C Moniker for gui initialization 
void cinit_qgui(int32_t w, int32_t h)
{
   debug_log("Function cinit_qgui w:%d, h:%d", w, h);
   QGUI = new qgui;
   QGUI->init(w,h);
   debug_log("cinit_qgui w:%d, h:%d", QGUI->width, QGUI->height);
}

//C Moniker for creating a new window
int32_t cqcreate_window(int16_t width, int16_t height, uint32_t flags)
{
   
   qwindow* qw = new qwindow;
   qw->init(width, height, flags);
   qw->id = qwindow_count++;
   qw->x = 30 + 30 * qwindow_count;
   qw->y = 40 + 40 * qwindow_count;
   QGUI->append_window(qw);
   debug_log("Function cqcreate_window after inserting to list id:%d", qw->id);
   debug_log("Function cqcreate_window id:%d, adress:%x, widht=%d, height=%d, flags=%x, guiw:%d, guih:%d", qw->id, qw->adress, height, flags, QGUI->width, QGUI->height);

   QGUI->find_window_from_pos(100,100);
   QGUI->draw_qwindows();
   qw->draw();
   return(qw->id);
}

void qhandle_event(const Event* event)
{
   QGUI->handle_event(event);
}
//C Moniker for drawing our window list
void cdraw_qwindows()
{
   QGUI->draw_qwindows();
}

//task loop for our gui
void qgui_task() 
{
// disable_interrupts();
   while (1) 
   {
      //debug_log("qgui task");
      if (QGUI->needs_redraw) 
      {
            QGUI->needs_redraw = false;
            gui_draw_frame();
            //gui_draw_frame();
      }

      
      disable_interrupts();
      QGUI->handle_gui_events();
      enable_interrupts();
   }
}
//-------------------------------------qgui---------------------------------------------------------
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
   debug_log("Function qgui::init");
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

   testbutton_x = graphics.width - 100;
   testbutton_y = graphics.height - 100;
   testbutton_w = 50;
   testbutton_h = 50;

   register_syscall(SYSCALL_cqcreate_window, (void*)cqcreate_window);

// init_windows();

// testbutton_x = graphics.width - 100;
// testbutton_y = graphics.height - 100;
// testbutton_w = 50;
// testbutton_h = 50;
};

//draw out window list
void qgui::draw_qwindows()
{
   list_node* l;
   l = list_tail->next;
   while(l!=list_head)
   {
      if(l)
      {
            qwindow* w = (qwindow*) l->data;
            if(!l->data)assert_msg(l->data==0,"draw window list: no data!");
            else if(w)
            {
               w->draw();
               //debug_log("draw window %d:",w->id);
            }
            else
               debug_log("draw window: no window found!");
      }
      else
      {
            debug_log("list window: no node found!");
      }
      l = l->next;
   }
};

//TODO - complete the function
//draw the whole frame
void qgui::draw_frame()
{
   debug_log("Function qgui::draw_frame");
   graphics_fill(COLOR_FRAME_BACKGROUND);  //Frame with given color (00RRGGBB)
   draw_debug_console(0);

   QGUI->draw_qwindows();             //drawing the windows

    //graphics_fill_rect(testbutton_x, testbutton_y, testbutton_w, testbutton_h, 0xFFFF00FF); //Drawing the testbutton to start file.exe

    //graphics_fill_rect(graphics.width - 10, 2, 8, 8, redraw_indicator ? 0xFF00FF : 0); //Drawing the indicator rectangle
    //redraw_indicator ^= 1; //Invers the indicator color

    //get time and save it time_str
    uint64_t time = get_system_time_millis(); //Get the system time in milliseconds
    char time_str[128];

    int phys_mem = pmm_get_total_allocated_pages() * 4; //get used physical memory

    sprintf(time_str, "phys used: %dKiB   systime: %u", phys_mem, time);
    graphics_draw_string(time_str, 3, graphics.height - 15, 0); //Print the system time at the bottom of the frame

   #ifdef NEWGUI
    graphics_copy_rect(gui.cursor_x, gui.cursor_y, 12, 19, 0, 0, (uint32_t*) res_cursor_raw); //draw the cursor
    #endif

    //Since we draw everything to the backbuffer, we need to copy the backbuffer to the actual framebuffer
    graphics_copy_backbuffer();
};

qwindow* qgui::find_window_from_pos(int32_t x, int32_t y) 
{
   //walk through window list
   list_node* l;
   l = list_tail->next;
   while(l!=list_head)
   {
      if(l)
      {
         qwindow* w = (qwindow*) l->data;

         if(!l->data)assert_msg(l->data==0,"draw window list: no data!");
         else if(w)
         {
            w->inside_content = false;
            debug_log("Function find_window_from_pos w->x: %d, w->y: %d, x: %d, y: %d", w->x, w->y, x, y);
            if (w->state == 0)
            {
               l=l->next;
               continue;
            }

            //is the mouse cursor inside the window?
            if ((w->x > x) || (w->y > y) || (w->x + w->actual_width < x) || (w->y + w->actual_height < y))
            {
               l=l->next;
               continue;
            }

            //is the mouse cursor in between of both window bars?
            if ((y > w->y + WINDOW_TITLE_BAR_HEIGHT) && (y < w->y + w->height))
               w->inside_content = true;
            
            debug_log("Mouse cursor is inside the window");
            return w;
         }
         else
            debug_log("draw window: no window found!");
      }
      else
      {
            debug_log("list window: no node found!");
      }
      l = l->next;
   }
   return NULL;
}

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

//----------------------------------------------qwindow----------------------------------------

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

bool qwindow::check_window_close(int32_t x, int32_t y) {

    int32_t close_button_x = x + width + 2 - CLOSE_BUTTON_WIDTH - 1;
    int32_t close_button_y = y + 10;

    if (x < close_button_x) return false;
    if (y < close_button_y) return false;
    if (x >= close_button_x + CLOSE_BUTTON_WIDTH) return false;
    if (y >= close_button_y + CLOSE_BUTTON_HEIGHT) return false;

    return true;
}

bool qwindow::check_window_resize(int32_t x, int32_t y)
{
    int32_t resize_grip_x = x + width - GRIP_SIZE;
    int32_t resize_grip_y = y + height + WINDOW_TITLE_BAR_HEIGHT - GRIP_SIZE;
    if (x < resize_grip_x)return false;
    if (y < resize_grip_y)return false;
    if (x >= resize_grip_x + GRIP_SIZE) return false;
    if (y >= resize_grip_y + GRIP_SIZE) return false;

    //kernel_log("Function check_window_resize grip_x: %d, grip_y %d, w->x: %d, w->y: %d, x: %d, y: %d, return %d", resize_grip_x, resize_grip_y,w->x, w->y,x,y,ret);
    return true;
}

void qwindow::destroy_window() {

    sharedmem_destroy(fb_shmem_id);
    delete(this);

    //!SECTION(&windows[window_id], 0, sizeof(Window));

   //  int index = z_order_find_index(window_id);
   //  if (index != -1) {
   //      z_order_remove_at(index);
   //  }

    QGUI->needs_redraw = true;
}
//---------------------------------------qgui--events------------------------------------------


void qgui::handle_gui_events() 
{
   //get the movement of the mousepointer
    //mouse is a global structure!
    cursor_x += mouse.x_acc;
    mouse.x_acc = 0;
    cursor_y += mouse.y_acc;
    mouse.y_acc = 0;

    //check if mousepointer is in the boundaries.
    if (cursor_x < 0)
        cursor_x = 0;
    if (cursor_y < 0)
        cursor_y = 0;
    if (cursor_x >= graphics.width)
        cursor_x = graphics.width - 1;
    if (cursor_y >= graphics.height)
        cursor_y = graphics.height - 1;

    //get the window which is currently under the cursor
    if(cursor_x != prev_cursor_x && cursor_y != prev_cursor_y)
        window_under_cursor = find_window_from_pos(cursor_x, cursor_y);

    static bool prev_mouse_left_button = 0;
    static bool prev_mouse_right_button = 0;

    if (prev_mouse_left_button != left_click || prev_mouse_right_button != right_click)
    {
        if (left_click) {
            handle_left_click();
        }
        else if (right_click){
            handle_right_click();
        } 
        else {
            currently_dragging_window = NULL;
        }

        prev_mouse_left_button = left_click;
        prev_mouse_right_button = right_click;
    }
    
    left_click = mouse.left_button;
    right_click = mouse.right_button;

    int32_t dx = cursor_x - prev_cursor_x;
    int32_t dy = cursor_y - prev_cursor_y;

    if (dx != 0 || dy != 0) {
        needs_redraw = true;

        if (currently_dragging_window) {
            currently_dragging_window->x += dx;
            currently_dragging_window->y += dy;
        } else if (window_under_cursor_inside_content) { // && focused_window == window_under_cursor
            Event move;
            move.type = EVENT_MOUSE_MOVE;
            move.data0 = cursor_x - window_under_cursor->x - WINDOW_CONTENT_XOFFSET;
            move.data1 = cursor_y - window_under_cursor->y - WINDOW_TITLE_BAR_HEIGHT;
            move.data2 = 0;
            handle_event(&move);
        }

        prev_cursor_x = cursor_x;
        prev_cursor_y = cursor_y;
    }
}



void qgui::handle_event(const Event* event) {

   //debug_log("Function qhandle_event()");
   push_cli();

   if (event->type == EVENT_MOUSE_MOVE) {
      if (window_under_cursor)
            send_event_to_task(window_under_cursor->owner_task_id, event);
   }
   
   if (event->type == EVENT_MOUSE_CLICK || event->type == EVENT_KEYBOARD) 
   {
      if (focused_window) 
      {
         send_event_to_task(focused_window->owner_task_id, event);
      } 
      else if (event->type == EVENT_KEYBOARD) 
      {
         if (event->data0 >> 7 == 0 && event->data1 != 0)
               console_key_typed(event->data1);
      }
   }
   // todo: send events to tasks listening in the background

   pop_cli();
}

void qgui::handle_left_click() {
    //Testbutton was clicked -> start files.exe in a new usertask
    if (cursor_x >= testbutton_x && cursor_x < testbutton_x + testbutton_w
        && cursor_y >= testbutton_y && cursor_y < testbutton_y + testbutton_h) {
        create_user_task("files.exe");
    }

    focused_window = window_under_cursor;

    if (window_under_cursor) {
        // we are clicking on a window
        //move_window_to_front(window_under_cursor);      //bring the window to front

        if (window_under_cursor_inside_content) {
            Event click;
            click.type = EVENT_MOUSE_CLICK;
            click.data0 = gui.cursor_x - window_under_cursor->x - WINDOW_CONTENT_XOFFSET;
            click.data1 = gui.cursor_y - window_under_cursor->y - WINDOW_TITLE_BAR_HEIGHT;
            click.data2 = 1;
            handle_event(&click);
        } else {
            //we are clicking on its border
            
            if(window_under_cursor->check_window_resize (cursor_x, cursor_y))
            {
                kernel_log("Window resize grip clicked");
            }
            
            else if (window_under_cursor->check_window_close (cursor_x, cursor_y))
            {
                uint32_t task_id = window_under_cursor->owner_task_id;
                // todo: allow for multiple windows
                window_under_cursor->destroy_window();
                kill_task(task_id);
                window_under_cursor = NULL;
                focused_window = NULL;
            }

            currently_dragging_window = window_under_cursor;
            
        }
        
    }

    gui.needs_redraw = true;
}

void qgui::handle_right_click()
{


}
//--------------------------------------------------lists-----------------------------------------

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


