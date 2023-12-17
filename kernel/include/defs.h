// Title:	 		defs.h
// Description:     provides global definitions
// First Created: 	12.12.2023
// Last Change:
 
#ifndef __DEFS_H
#define __DEFS_H
#ifdef __cplusplus
extern "C" {
#endif

//Colors
#define COLOR_WHITE         0xFFFFFFFF
#define COLOR_BLACK         0xFF000000
#define COLOR_TRANSPARENT   0x00FFFFFF

#define COLOR_LIGHT_GREY    0xFFC0C0C0
#define COLOR_GREY          0xFF808080
#define COLOR_DARK_GREY     0xFF404040


#define COLOR_RED           0xFFFF0000
#define COLOR_LIGHT_RED     0xFFC00000
#define COLOR_DARK_RED      0xFF400000

#define COLOR_GREEN         0xFF00FF00
#define COLOR_LIGHT_GREEN   0xFF00C000
#define COLOR_DARK_GREEN    0xFF004000

#define COLOR_BLUE          0xFF0000FF
#define COLOR_LIGHT_BLUE    0xFF0000C0
#define COLOR_DARK_BLUE     0xFF000040

//System configs
#define COLOR_WINDOW                COLOR_DARK_GREY
#define COLOR_ACTIVE_WINDOW         COLOR_LIGHT_GREY
#define COLOR_FRAME_BACKGROUND      COLOR_LIGHT_GREY
#define COLOR_WINDOW_BACKGROUND     COLOR_DARK_BLUE
#define COLOR_TITLE_BAR_ACTIVE      COLOR_LIGHT_BLUE
#define COLOR_TITLE_BAR_INACTIVE    COLOR_DARK_BLUE
#define COLOR_STATUS_BAR_ACTIVE     COLOR_LIGHT_GREEN
#define COLOR_STATUS_BAR_INACTIVE   COLOR_DARK_GREEN

//window stuff
#define WINDOW_CONTENT_XOFFSET 1                //x-offset of the window. Used to give a distance between the windowborder and its content  
#define WINDOW_TITLE_BAR_HEIGHT 20              //height of the window title bar
#define WINDOW_STATUS_BAR_HEIGHT 20             //height of the window status bar

#define CLOSE_BUTTON_WIDTH 18                   //x dimension of the window close button
#define CLOSE_BUTTON_HEIGHT 18                  //y dimension of the window close button
#define GRIP_SIZE WINDOW_STATUS_BAR_HEIGHT      //The size of the resizing grip. For now it has the same size (width and height) as the status bar height.                           

#define WINDOW_TITLE_MAX_LENGTH 64              //The maximum length of the window title
#define WINDOW_FLAG_FULLSCREEN (1 << 0)         //Window is opened in fullscreen size
#define WINDOW_FLAG_DOUBLE_BUFFERED (1 << 1)    //Window is double buffered
#define WINDOW_FLAG_RESIZABLE (1<<2)            //Window has a resize grip
#define WINDOW_FLAG_TITLE_BAR (1<<3)            //Window has a title bar (at the top of the window)
#define WINDOW_FLAG_STATUSBAR (1<<4)            //Window has a status bar (at the bottom of the window)
#define WINDOW_FLAG_IS_WORKBENCH (1<<5)         //This window is the workbench window. Allways stay in the back

#ifdef __cplusplus
}
#endif
#endif //__DEFS_H