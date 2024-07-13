#include "init/main.h"

int main(int argc, char ** argv) {
    //int a = 3 / 0;
    *(char *)0 = 0x123;
    return 0;
}
