#include "pico/stdio.h"
#include "pico/stdlib.h"
/* File system */
#include "fs/fs.h"
/* Time and Timestamps */
#include "pico/time.h"
/* TinyUSB */
#include "bsp/board.h"
#include "tusb.h"
/* Memcard Simulation */
#include "memcard_simulator.h"
/* LED Control */
#include "led.h"
/* Global Configuration */
#include "config.h"
#include "pico/bootrom.h"

bool tud_mount_status = false;

void cdc_task(void);

/*------------- MAIN -------------*/
int main(void) {
	stdio_init_all();
	led_init();
	

	if(fs_manager.init) fs_manager.init(0);

	/* Pico connected to PC, initialize USB transfer mode */
	board_init();
	tusb_init();

	while(true) {
		tud_task(); // tinyusb device task
		cdc_task();

		if(to_ms_since_boot(get_absolute_time()) > TUD_MOUNT_TIMEOUT && !tud_mount_status)
			break;
	}
	
	/* Pico powered by PSX, initialize memory card simulation */
	simulate_memory_card();	

	return 0;
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) {
	/* Initialize SD card/QSPI */
	if(fs_manager.mount) {
		tud_mount_status = true;
		fs_manager.mount(0);
	}
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
	if(fs_manager.umount) {
		fs_manager.umount(0);
	}
	tud_mount_status = false;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
	(void) remote_wakeup_en;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+

#define CMD_BUF_SIZE 64

static char cmd_buf[CMD_BUF_SIZE];
static uint8_t cmd_len = 0;
static bool prompt_shown = false;

void show_prompt(void) {
    if (tud_cdc_connected()) {
        tud_cdc_write_str("> ");
        tud_cdc_write_flush();
        prompt_shown = true;
    }
}

void handle_command(const char *cmd) {
    if (strcmp(cmd, "bootsel") == 0) {
        DBG_VERBOSE("\nRebooting to BOOTSEL...\n");
        sleep_ms(100); // allow flush
        reset_usb_boot(0, 0);
    }
    else if (strcmp(cmd, "help") == 0) {
        DBG_VERBOSE("\nCommands:\n");
        DBG_VERBOSE("  help     - show this message\n");
        DBG_VERBOSE("  bootsel  - reboot to BOOTSEL\n");
    }
    else if (strlen(cmd) == 0) {
        // ignore empty
    }
    else {
        DBG_VERBOSE("\nUnknown: %s\n", cmd);
    }
}

void cdc_task(void) {
    while (tud_cdc_available()) {
        char c;
        tud_cdc_read(&c, 1);

        // ENTER
        if (c == '\r' || c == '\n') {
            tud_cdc_write_str("\r\n");

            cmd_buf[cmd_len] = '\0';
            handle_command(cmd_buf);

            cmd_len = 0;
            prompt_shown = false;
        }
        // BACKSPACE
        else if (c == 0x7F || c == '\b') {
            if (cmd_len > 0) {
                cmd_len--;
                tud_cdc_write_str("\b \b"); // erase char on terminal
            }
        }
        // NORMAL CHAR
        else if (cmd_len < CMD_BUF_SIZE - 1) {
            cmd_buf[cmd_len++] = c;
            tud_cdc_write(&c, 1); // echo
        }
    }

    if (!prompt_shown && tud_cdc_connected()) {
        show_prompt();
    }
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
	(void) itf;
	(void) rts;
}


void tud_cdc_rx_cb(uint8_t itf) {
    (void) itf;
}