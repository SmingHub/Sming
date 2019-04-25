#include <user_config.h>

static uint32_t interruptState = 0;

void xt_disable_interrupts()
{
    __asm__ __volatile__("rsil %0,15":"=a" (interruptState));
    __asm__("esync");
    __asm__("isync");
    __asm__("dsync");
}
void xt_enable_interrupts()
{
    __asm__ __volatile__("wsr %0,ps"::"a" (interruptState) : "memory");
    __asm__("esync");
}
