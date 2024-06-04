#include "dev/keyboard.h"
#include "comm/cpu_instr.h"
#include "cpu/irq.h"
#include "comm/types.h"
#include "dev/tty.h"
#include "sys/_intsup.h"
#include "tools/klib.h"
#include "tools/log.h"

static keyboard_state_t keyboard_state; 

static const key_map_t map_table[] = {
        [0x2] = {'1', '!'},
        [0x3] = {'2', '@'},
        [0x4] = {'3', '#'},
        [0x5] = {'4', '$'},
        [0x6] = {'5', '%'},
        [0x7] = {'6', '^'},
        [0x08] = {'7', '&'},
        [0x09] = {'8', '*' },
        [0x0A] = {'9', '('},
        [0x0B] = {'0', ')'},
        [0x0C] = {'-', '_'},
        [0x0D] = {'=', '+'},
        [0x0E] = {0x7F, 0x7F},
        [0x0F] = {'\t', '\t'},
        [0x10] = {'q', 'Q'},
        [0x11] = {'w', 'W'},
        [0x12] = {'e', 'E'},
        [0x13] = {'r', 'R'},
        [0x14] = {'t', 'T'},
        [0x15] = {'y', 'Y'},
        [0x16] = {'u', 'U'},
        [0x17] = {'i', 'I'},
        [0x18] = {'o', 'O'},
        [0x19] = {'p', 'P'},
        [0x1A] = {'[', '{'},
        [0x1B] = {']', '}'},
        [0x1C] = {'\n', '\n'},
        [0x1E] = {'a', 'A'},
        [0x1F] = {'s', 'B'},
        [0x20] = {'d',  'D'},
        [0x21] = {'f', 'F'},
        [0x22] = {'g', 'G'},
        [0x23] = {'h', 'H'},
        [0x24] = {'j', 'J'},
        [0x25] = {'k', 'K'},
        [0x26] = {'l', 'L'},
        [0x27] = {';', ':'},
        [0x28] = {'\'', '"'},
        [0x29] = {'`', '~'},
        [0x2B] = {'\\', '|'},
        [0x2C] = {'z', 'Z'},
        [0x2D] = {'x', 'X'},
        [0x2E] = {'c', 'C'},
        [0x2F] = {'v', 'V'},
        [0x30] = {'b', 'B'},
        [0x31] = {'n', 'N'},
        [0x32] = {'m', 'M'},
        [0x33] = {',', '<'},
        [0x34] = {'.', '>'},
        [0x35] = {'/', '?'},
        [0x39] = {' ', ' '},
};

static inline int make_code(uint8_t key_code) {
    return !(key_code & 0x80);
}

static inline char get_key(uint8_t key_code) {
    return key_code & 0x7f;
}

static void do_fx_key(int key) {
    int idx = key - KEY_F1;
    if (keyboard_state.lctrl_press || keyboard_state.rctrl_press) {
        tty_select(idx);
    }
}

static void do_normal_key(uint8_t raw_code) {
    char key = get_key(raw_code);
    int is_make = make_code(raw_code);
    switch (key) {
        case KEY_RSHIFT:
            keyboard_state.rshift_press = is_make ? 1 : 0;
            break;
        case KEY_LSHIFT:
            keyboard_state.lshift_press = is_make ? 1 : 0;
            break;
        case KEY_CAPS:
            if (is_make) {
                keyboard_state.caps_lock = ~keyboard_state.caps_lock;
            }
            break;
        case KEY_ALT:
            keyboard_state.lalt_press = is_make;
            break;
        case KEY_CTRL:
            keyboard_state.lctrl_press = is_make;
            break;
        case KEY_F1:
        case KEY_F2:
        case KEY_F3:
        case KEY_F4:
        case KEY_F5:
        case KEY_F6:
        case KEY_F7:
        case KEY_F8:
            do_fx_key(key);
            break;
        case KEY_F9:
        case KEY_F10:
        case KEY_F11:
        case KEY_F12:
            break;
        default:
            if (is_make) {
                if (keyboard_state.rshift_press || keyboard_state.lshift_press) {
                    key = map_table[key].func;
                    if (keyboard_state.caps_lock) {
                        key = map_table[key].normal;
                    }
                } else {
                    key = map_table[key].normal;
                    if (keyboard_state.caps_lock) {
                        key = map_table[key].func;
                    }
                }
                tty_in(key);
            }
            break;
    }
}

static void do_e0_key(uint8_t raw_code) {
    char key = get_key(raw_code);
    int is_make = make_code(raw_code);

    switch (key) {
        case KEY_CTRL:
            keyboard_state.rctrl_press = is_make ? 1 : 0;
            break;
        case KEY_ALT:
            keyboard_state.ralt_press = is_make ? 1 : 0;
            break;
    }
}

void keyboard_init(void) {
    static int inited = 0;
    if (!inited) {
        kernel_memset(&keyboard_state, 0, sizeof(keyboard_state));
        irq_install(IRQ1_KEYBOARD, (irq_handler_t)exception_handler_keyboard);
        irq_enable(IRQ1_KEYBOARD);
        inited = 1;
    }
}

void do_handler_keyboard(exception_frame_t * frame) {

    static enum {
        NORMAL,
        BEGIN_E0,
        BEGIN_E1
    }recv_state = NORMAL;

    uint32_t status = inb(KEYBOARD_PORT_STAT);
    if (!(status & KEYBOARD_STAT_RECV_READY)) {
        pic_send_eoi(IRQ1_KEYBOARD);
        return;
    }

    uint8_t raw_code = inb(KEYBOARD_PORT_DATA);
    pic_send_eoi(IRQ1_KEYBOARD);

    if (raw_code == KEY_E0) {
        recv_state = BEGIN_E0;
    } else if (raw_code == KEY_E1) {
        recv_state = BEGIN_E1;
    } else {
        switch (recv_state) {
            case NORMAL:
                do_normal_key(raw_code);
                break;
            case BEGIN_E0:
                do_e0_key(raw_code);
                recv_state = NORMAL;
                break;
            case BEGIN_E1:
                recv_state = NORMAL;
                break;
            default:
                break;
        }
    }
}

