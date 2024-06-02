#ifndef CONSOLE_H
#define CONSOLE_H

#include "comm/types.h"

#define CONSOLE_DISP_ADDR       0xb8000
#define CONSOLE_DISP_END        (0xb8000 + 32*1024)
#define CONSOLE_ROW_MAX         25
#define CONSOLE_COL_MAX         80
#define ASCII_ESC               0x1b
#define ESC_PARAM_MAX           10

typedef enum {
    COLOR_Black = 0,
    COLOR_Blue,
    COLOR_Green,
    COLOR_Cyan,
    COLOR_Red,
    COLOR_Magenta,
    COLOR_Brown,
    COLOR_DarkGray,
    COLOR_LightBlue,
    COLOR_LightGreen,
    COLOR_LightCyan,
    COLOR_LightRed,
    COLOR_LightMagenta,
    COLOR_Yellow,
    COLOR_White,
} color_t;

typedef union _disp_char_t {
    struct {
        char c;
        char foreground : 4;
        char background : 3;
    };
    uint16_t v;
} disp_char_t;

typedef struct _console_t {
    enum {
        CONSOLE_WRITE_NORMAL,
        CONSOLE_WRITE_ESC,
        CONSOLE_WRITE_SQUARE,
    }write_state;

    disp_char_t * disp_base;
    int cursor_row, cursor_col;
    int display_rows, display_cols;
    color_t foreground, background;
    int old_cursor_col, old_cursor_row;
    int esc_param[ESC_PARAM_MAX];
    int curr_param_index;
} console_t;

int console_init(void);
int console_write(int console, char * data, int size);
void console_close(int console);

#endif