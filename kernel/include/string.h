/*
    Header file for string.cpp
    Last change 2023/03/17
*/

#ifndef __STRING_H
#define __STRING_H
#include "types.h"

#ifdef __cplusplus
 extern "C" {
    
#endif

size_t strlen(const char *str);
char* strncpy(char *dest, const char *src, size_t n);
char* strcpy(char* dest, const char* src);
int   strcmp(const char* str1, const char* str2);
void *memset(void *dest, char val, size_t count);
void *memcpy(void *dest, const void *src, size_t count);
unsigned short *memsetw(unsigned short *dest, unsigned short val, size_t count);

#ifdef __cplusplus
 }
#endif

#endif
