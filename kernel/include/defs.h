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
#define COLOR_WINDOW            COLOR_DARK_GREY
#define COLOR_ACTIVE_WINDOW     COLOR_LIGHT_GREY
#define COLOR_FRAME_BACKGROUND  COLOR_LIGHT_RED
#define COLOR_WINDOW_BACKGROUND COLOR_DARK_BLUE
#define COLOR_TITLE_BAR         COLOR_LIGHT_BLUE
#define COLOR_STATUS_BAR        COLOR_LIGHT_GREEN

#ifdef __cplusplus
}
#endif
#endif //__DEFS_H