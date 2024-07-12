#ifndef MAIN_H
#define MAIN_H

#define ESC_CMD2(Pn, cmd)               "\x1b["#Pn#cmd
#define ESC_COLOR_ERROR                 ESC_CMD2(31, m)
#define ESC_COLOR_DEFAULT               ESC_CMD2(39, m)


#endif