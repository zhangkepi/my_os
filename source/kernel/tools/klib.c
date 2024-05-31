#include "tools/klib.h"
#include "comm/cpu_instr.h"
#include "comm/types.h"
#include "tools/log.h"
#include <stdarg.h>

void kernel_strcpy(char * src, char * dest) {
    if (!src || !dest) {
        return;
    }
    while (*dest && *src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}


void kernel_strncpy(char * src, char * dest, int size) {
    if (!src || !dest || (size <= 0)) {
        return;
    }
    char * d = dest;
    const char * s = src;
    while ((size-- > 0) && (*s)) {
        *d++ = *s++;
    }
    if (size == 0) {
        *(d-1) = '\0';
    } else {
        *d = '\0';
    }
}


int kernel_strncmp(const char * s1, const char * s2, int size) {
    if (!s1 || !s2) {
        return -1;
    }
    while (*s1 && *s2 && (*s1 == *s2) && size) {
        s1++;
        s2++;
    }
    return !((*s1 == '\0') || (*s2 == '\0') || (*s1 == *s2));
}


int kernel_strlen(const char * str) {
    if (!str) {
        return 0;
    }
    const char * c = str;
    int len = 0;
    while (*c++) {
        len++;
    }
    return len;
}

void kernel_memcpy(void * src, void * dest, int size) {
    if (!src || !dest || (size <= 0)) {
        return;
    }
    uint8_t * s = (uint8_t *)src;
    uint8_t * d = (uint8_t *)dest;
    while (size--) {
        *d++ = *s++;
    }
}

void kernel_memset(void * dest, uint8_t v, int size) {
    if (!dest || (size <= 0)) {
        return;
    }
    uint8_t * d = (uint8_t *)dest;
    while (size--) {
        *d++ = v;
    }
}


int kernel_memcmp(void * d1, void * d2, int size) {
    if (!d1 || !d2 || !size) {
        return 1;
    }
    uint8_t * p_d1 = (uint8_t *)d1;
    uint8_t * p_d2 = (uint8_t *)d2;
    while (size--) {
        if (*p_d1++ != *p_d2++) {
            return 1;
        }
    }
    return 0;
}

static const char * num2ch = {"FEDCBA9876543210123456789ABCDEF"};
void kernel_itoa(char * buf, int num, int base) {
    char * p = buf;
    int origin_num = num;
    if ((base != 2) && (base != 8) && (base != 10) && (base != 16)) {
        *p = '\0';
        return;
    }
    if ((num < 0) && (base == 10)) {
        *p++ = '-';
    }
    do {
        char ch = num2ch[num % base + 15];
        *p++ = ch;
        num /= base;
    }while (num);
    *p-- = '\0';

    char * start = origin_num > 0 ? buf : buf + 1;
    while (start < p) {
        char ch = *start;
        *start = *p;
        *p = ch;

        start++;
        p--;
    }
}


void kernel_sprintf(char * buf, const char * fmt, ...) {
    va_list args;

    va_start(args, fmt);
    kernel_vsprintf(buf, fmt, args);
    va_end(args);
}

void kernel_vsprintf(char * buf, const char * fmt, va_list args) {

    enum {NORMAL, READ_FMT} state = NORMAL;

    char * curr = buf;
    char ch;
    while ((ch = *fmt++)) {
        switch (state) {
            case NORMAL:
                if (ch == '%') {
                    state = READ_FMT;
                } else {
                    *curr++ = ch;
                }
                break;
            case READ_FMT:
                if (ch == 's'){
                    const char * str = va_arg(args, char *);
                    int len = kernel_strlen(str);
                    while (len--) {
                        *curr++ = *str++;
                    }
                } else if (ch == 'd') {
                    int num = va_arg(args, int);
                    kernel_itoa(curr, num, 10);
                    curr += kernel_strlen(curr);
                } else if (ch == 'x') {
                    int num = va_arg(args, int);
                    kernel_itoa(curr, num, 16);
                    curr += kernel_strlen(curr);
                } else if (ch == 'c') {
                    char c = va_arg(args, int);
                    *curr++ = c;
                }
                state = NORMAL;
                break;
        }
    }
}


void pannic(const char * file, int line, const char * func, const char * cond) {
    log_printf("assert failed! %s", cond);
    log_printf("file: %s", file);
    log_printf("line: %d", line);
    log_printf("func: %s", func);

    for (; ; ) {
        hlt();
    }
}

int strings_count(char ** start) {
    int count = 0;
    if (start) {
        while (*start++) {
            count++;
        }
    }
    return count;
}


char * get_file_name(char * path_name) {
    char * s = path_name;

    while (*s != '\0') {
        s++;
    }

    while ((*s != '/') && (*s != '\\') && (s >= path_name)) {
        s--;
    }

    return s+1;
}

