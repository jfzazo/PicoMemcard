#ifndef __DEBUG_H__
#define __DEBUG_H__

#define DBG_LEVEL_NONE 0
#define DBG_LEVEL_INFO 1
#define DBG_LEVEL_VERBOSE 2

#if DBG_LEVEL>0
    void usb_debug_printf(const char *fmt, ...);

    #define DBG_INFO(...)   do { if (DBG_LEVEL >= 1) usb_debug_printf(__VA_ARGS__); else ((void)0); } while (0);
    #define DBG_VERBOSE(...) do { if (DBG_LEVEL >= 2) usb_debug_printf(__VA_ARGS__); else ((void)0); } while (0);

#else

    #define usb_debug_printf(...) ((void)0);

    #define DBG_INFO(...)   do { usb_debug_printf(__VA_ARGS__); } while (0);
    #define DBG_VERBOSE(...) do { usb_debug_printf(__VA_ARGS__); } while (0);

#endif



#endif