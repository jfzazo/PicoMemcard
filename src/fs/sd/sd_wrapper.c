#include <string.h>
#include "fs/sd/sd_config.h"

void sd_init(uint8_t num) {
	/* Initialize SD card */
	sd_card_t *p_sd = sd_get_by_num(num);
	if (!p_sd) return;
	p_sd->init(p_sd);
}



uint32_t sd_mount(uint8_t num) {
	/* Mount and test SD card filesystem */
	sd_card_t *p_sd = sd_get_by_num(num);
	return f_mount(&p_sd->fatfs, "", 1);
}


void* sd_ext_get_by_num(uint8_t lun) {
    return (void *)sd_get_by_num(lun);
}

uint32_t sd_get_sectors(uint8_t lun) {
    sd_card_t* p_sd = sd_get_by_num(lun);
	if (!p_sd) return 0;

    return (uint32_t) p_sd->sectors;
}
uint16_t sd_get_block_size(uint8_t lun) {
    return (uint16_t) BLOCK_SIZE;
}



uint32_t sd_read(uint8_t* data, uint32_t *size, uint8_t* file_name, uint32_t max_size) {
	uint32_t status = FR_OK;
	FIL memcard;

	if(data) {
		status = f_open(&memcard, file_name, FA_READ);
		if(status == FR_OK) {
			UINT sd_size;
			status = f_read(&memcard, data, max_size, &sd_size);
			*size = sd_size;
			f_close(&memcard);
		} else {
			status = !FR_OK;
		}
	} else {
		status = !FR_OK;
	}

	return status;
}


uint32_t sd_write(uint8_t* data, uint32_t size, uint8_t* file_name, uint32_t *written) {
	return sd_write_at(data, size, 0, file_name, written);
}
uint32_t sd_write_at(uint8_t* data, uint32_t size, uint32_t offset, uint8_t* file_name, uint32_t *written) {
	uint32_t status = !FR_OK;
	FIL memcard;
	if(data) {
		if(FR_OK == f_open(&memcard, file_name, FA_READ | FA_WRITE)) {
			UINT bytes_written;
			f_lseek(&memcard, offset);
			status = f_write(&memcard, data, size, &bytes_written);
			*written = bytes_written;

			f_close(&memcard);
		} else {
			status = !FR_OK;
		}
	}

	return status;
}


void sd_dir_read(void (*callback)(unsigned char *filename, uint32_t size)) {
	FRESULT res;
	DIR root;
	FILINFO f_info;
	res = f_opendir(&root, "");	// open root directory
	if(res == FR_OK) {
		while(true) {
			res = f_readdir(&root, &f_info);
			if(res != FR_OK || f_info.fname[0] == 0) break;
			if(!(f_info.fattrib & AM_DIR)) {	// not a directory
				callback(f_info.fname, f_info.fsize);
			}
		}
	}
}