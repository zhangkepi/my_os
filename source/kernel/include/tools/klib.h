#ifndef KLIB_H
#define KLIB_H

#include <stdarg.h>
#include "comm/types.h"


static inline uint32_t down2(uint32_t size, uint32_t bound) {
    return size & ~(bound - 1);
}

static inline uint32_t up2(uint32_t size, uint32_t bound) {
    return (size + bound - 1) & ~(bound - 1);
}

void kernel_strcpy(char * src, char * dest);
void kernel_strncpy(char * src, char * dest, int size);
int kernel_strncmp(const char * s1, const char * s2, int size);
int kernel_strlen(const char * str);

void kernel_memcpy(void * src, void * dest, int size);
void kernel_memset(void * dest, uint8_t v, int size);
int kernel_memcmp(void * d1, void * d2, int size);

void kernel_sprintf(char * buf, const char * fmt, ...);
void kernel_vsprintf(char * buf, const char * fmt, va_list args);

int strings_count(char ** start);
char * get_file_name(char * path_name);


#ifndef RELEASE
#define ASSERT(expr) \
    if(!(expr)) pannic(__FILE__, __LINE__, __func__, #expr)
void pannic(const char * file, int line, const char * func, const char * cond);
#else
#define ASSERT(expr)    ((void)0)
#endif


#endif