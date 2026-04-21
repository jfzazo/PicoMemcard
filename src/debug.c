
#include <stdio.h>
#include <stdarg.h>
#include "tusb.h"
#include "debug.h"

static void usb_debug_vprintf(const char *fmt, va_list args)
{
    char buffer[128];
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    if (len <= 0) return;

    if (tud_cdc_connected() && tud_cdc_write_available() >= len)
    {
        tud_cdc_write(buffer, len);
        tud_cdc_write_flush();
    }
}

void usb_debug_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    usb_debug_vprintf(fmt, args);
    va_end(args);
}
