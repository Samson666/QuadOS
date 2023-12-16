// Title:	 		log.h
// Description:
// First Created: 	16.12.2023
// Last Change:
 
#ifndef __LOG_H
#define __LOG_H
#ifdef __cplusplus
extern "C" {
#endif

#include "printf.h"
#include "types.h"

void init_logging(bool serial);
void kernel_log(const char* format, ...);

#ifdef __cplusplus
}
#endif
#endif //__LOG_H




