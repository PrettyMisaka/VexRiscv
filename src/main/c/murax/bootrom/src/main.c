#include "uart.h"
extern void crtStart(void);

void main()
{
    print("enter bootrom success!\n");
    crtStart();
}