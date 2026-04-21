#include <string.h>
#include "fs/flash/flash_config.h"
#include "fs/flash/ram_disk.h"
#include "fs/flash/lfs_disk.h"


void lfs_init() {
	lfs_t lfs;
	
	/* Initialize LittleFs */
	int lfs_status = lfs_mount(&lfs, &LFS_CFG);	// mount the filesystem
	if (lfs_status) {
		lfs_format(&lfs, &LFS_CFG);	// reformat file system if error occurred (normally on first boot)
		lfs_mount(&lfs, &LFS_CFG);
	}
}

uint32_t flash_mount(uint8_t num) {
	return RAM_disk_import_lfs_memcard();
}

void* flash_get_by_num(uint8_t lun) { // Nothing to return. Just for compatibilities aspects
    return (void *)1;
}

uint32_t flash_get_sectors(uint8_t lun) {
    return (uint32_t) DISK_BLOCK_NUM;
}

uint16_t flash_get_block_size(uint8_t lun) {
    return (uint16_t) BLOCK_SIZE;
}

void flash_init(uint8_t num) {
	lfs_init();
}


uint32_t flash_read(uint8_t* data, uint32_t *size, uint8_t* file_name, uint32_t max_size) {
	uint32_t status = 0;
	if(data) {
		lfs_t lfs;
		lfs_file_t memcard;
		if(LFS_ERR_OK == lfs_mount(&lfs, &LFS_CFG)) {
			if(LFS_ERR_OK == lfs_file_open(&lfs, &memcard, file_name, LFS_O_RDONLY)) {
				lfs_ssize_t lfs_size = lfs_file_read(&lfs, &memcard, data, max_size);
				*size = lfs_size;
				lfs_file_close(&lfs, &memcard);
				status = FR_OK;
			} else  {
				status = !FR_OK;
			}
			lfs_unmount(&lfs);
		} else {
			status = !FR_OK;
		}
	} else {
		status = !FR_OK;
	}
	return status;
}


uint32_t flash_write(uint8_t* data, uint32_t size, uint8_t* file_name, uint32_t *written) {
	return flash_write_at(data, size, 0, file_name, written);
}


uint32_t flash_write_at(uint8_t* data, uint32_t size, uint32_t offset, uint8_t* file_name, uint32_t *written) {
	uint32_t status = !FR_OK;

	if(data) {
		lfs_t lfs;
		lfs_file_t memcard;
		if(LFS_ERR_OK == lfs_mount(&lfs, &LFS_CFG)) {
			if(LFS_ERR_OK == lfs_file_open(&lfs, &memcard, file_name, LFS_O_RDWR)) {
				lfs_file_seek(&lfs, &memcard,  offset, LFS_SEEK_SET);
				*written = lfs_file_write(&lfs, &memcard, data, size);
				lfs_file_close(&lfs, &memcard);
				status = FR_OK;
			}
			lfs_unmount(&lfs);
		} 
	}

	return status;
}


void flash_dir_read(void (*callback)(unsigned char *filename, uint32_t size)) {
	lfs_dir_t dir;
	struct lfs_info info;
	lfs_t lfs;
	lfs_file_t memcard;

	if(LFS_ERR_OK == lfs_mount(&lfs, &LFS_CFG)) {
		lfs_dir_open(&lfs, &dir, "/");

		while (lfs_dir_read(&lfs, &dir, &info) > 0) {
			callback(info.name, info.size);			
		}

		lfs_dir_close(&lfs, &dir);
		lfs_unmount(&lfs);
	}
}