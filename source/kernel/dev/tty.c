#include "cpu/irq.h"
#include "dev/console.h"
#include "dev/dev.h"
#include "dev/keyboard.h"
#include "dev/tty.h"
#include "ipc/sem.h"
#include "sys/_intsup.h"
#include "tools/log.h"


static tty_t tty_devs[TTY_NR];
static int curr_tty = 0;

static void tty_fifo_init(tty_fifo_t * fifo, char * buf, int buf_size) {
    fifo->buf = buf;
    fifo->size = buf_size;
    fifo->read = fifo->write = 0;
    fifo->count = 0;
}

int tty_fifo_put(tty_fifo_t * fifo, char c) {
    irq_state_t state = irq_enter_protection();
    if (fifo->count >= fifo->size) {
        irq_leave_protection(state);
        return -1;
    }
    fifo->buf[fifo->write++] = c;
    if (fifo->write >= fifo->size) {
        fifo->write = 0;
    }
    fifo->count++;
    irq_leave_protection(state);
    return 0;
}

int tty_fifo_get(tty_fifo_t * fifo, char *c) {
    irq_state_t state = irq_enter_protection();
    if (fifo->count <= 0) {
        irq_leave_protection(state);
        return -1;
    }
    *c = fifo->buf[fifo->read++];
    if (fifo->read >= fifo->size) {
        fifo->read = 0;
    }
    fifo->count--;
    irq_leave_protection(state);
    return 0;
}

static tty_t * get_tty(device_t * dev) {
    int idx = dev->minor;
    if (idx < 0 || idx > TTY_NR || !dev->open_cnt) {
        log_printf("tty is not opened. tty=%d", idx);
        return (tty_t *)0;
    }
    return &tty_devs[idx];
}

int tty_open(device_t * dev) {
    int idx = dev->minor;
    if (idx < 0 || idx >= TTY_NR) {
        log_printf("open tty failed. incorrect tty minor=%d", idx);
        return -1;
    }
    tty_t * tty = tty_devs + idx;
    tty_fifo_init(&tty->ififo, tty->ibuf, TTY_IBUF_SIZE);
    sem_init(&tty->osem, TTY_OBUF_SIZE);

    tty_fifo_init(&tty->ofifo, tty->obuf, TTY_OBUF_SIZE);
    sem_init(&tty->isem, 0);

    tty->iflags = TTY_INCLR | TTY_IECHO;
    tty->console_idx = idx;
    tty->oflags = TTY_OCRLF;

    console_init(tty->console_idx);
    keyboard_init();

    return 0;
}

int tty_write(device_t * dev, int addr, char * buf, int size) {
    if (size < 0) {
        return -1;
    }
    tty_t * tty = get_tty(dev);
    if (!tty) {
        return -1;
    }
    int len = 0;
    while (size) {
        char c = *buf++;

        if ((c == '\n') && (tty->oflags & TTY_OCRLF)) {
            sem_wait(&tty->osem);
            int err = tty_fifo_put(&tty->ofifo, '\r');
            if (err < 0) {
                break;
            }
        }

        sem_wait(&tty->osem);
        int err = tty_fifo_put(&tty->ofifo, c);
        if (err < 0) {
            break;
        }
        len++;
        size--;
        console_write(tty);
    }
    return len;
}

int tty_read(device_t * dev, int addr, char * buf, int size) {
    if (size < 0) {
        return -1;
    }
    tty_t * tty = get_tty(dev);
    char * p_buf = buf;
    int len = 0;
    while (len < size) {
        sem_wait(&tty->isem);
        
        char c;
        tty_fifo_get(&tty->ififo, &c);

        switch (c) {
            case 0x7F:
                if (len == 0) {
                    continue;
                }
                len--;
                p_buf--;
                break;
            case '\n':
                if ((tty->iflags & TTY_INCLR) && (len < size - 1)) {
                    *p_buf++ = '\r';
                    len++;
                }
                *p_buf++ = '\n';
                len++;
                break;
            default:
                *p_buf++ = c;
                len++;
                break;
        }
        if (tty->iflags & TTY_IECHO) {
            tty_write(dev, 0, &c, 1);
        }
        if (c == '\r' || c == '\n') {
            break;
        }
    }
    return len;
}



int tty_control(device_t * dev, int cmd, int arg0, int arg1) {
    tty_t * tty = get_tty(dev);
    switch (cmd) {
        case TTY_CMD_ECHO:
            if (arg0) {
                tty->iflags |= TTY_IECHO;
            } else {
                tty->iflags &= ~TTY_IECHO;
            }
            break;
    }
    return 0;
}

int tty_close(device_t * dev) {
    return 0;
}


void tty_in(char ch) {
    tty_t * tty = tty_devs + curr_tty;
    if (sem_count(&tty->isem) >= TTY_IBUF_SIZE) {
        return;
    }
    tty_fifo_put(&tty->ififo, ch);
    sem_notify(&tty->isem);
}

void tty_select(int tty) {
    if (curr_tty != tty) {
        console_select(tty);
        curr_tty = tty;
    }
}

dev_desc_t dev_tty_desc = {
    .name = "tty",
    .major = DEV_TTY,
    .open = tty_open,
    .read = tty_read,
    .write = tty_write,
    .control = tty_control,
    .close = tty_close,
};