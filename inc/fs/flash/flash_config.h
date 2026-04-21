#pragma once

#include "config.h"
#include "ff.h"
    
#ifdef __cplusplus
extern "C" {
#endif
    void flash_init(uint8_t num);
    uint32_t flash_mount(uint8_t num);

    void* flash_get_by_num(uint8_t lun);
    uint32_t flash_get_sectors(uint8_t lun);
    uint16_t flash_get_block_size(uint8_t lun);

    uint32_t flash_read(uint8_t* data, uint32_t *size, uint8_t* file_name, uint32_t max_size);
    uint32_t flash_write(uint8_t* data, uint32_t size, uint8_t* file_name, uint32_t *written);
    uint32_t flash_write_at(uint8_t* data, uint32_t size, uint32_t offset, uint8_t* file_name, uint32_t *written);

    void flash_dir_read(void (*callback)(unsigned char *filename, uint32_t size));
#ifdef __cplusplus
}
#endif
