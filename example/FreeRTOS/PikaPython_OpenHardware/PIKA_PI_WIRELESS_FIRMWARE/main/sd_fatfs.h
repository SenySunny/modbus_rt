#ifndef SD_FATFS_H
#define SD_FATFS_H

#define MOUNT_POINT "/sd"
#define SDSPI_CS (GPIO_NUM_47)
#define SDSPI_CLK (GPIO_NUM_21)
#define SDSPI_MISO (GPIO_NUM_14)
#define SDSPI_MOSI (GPIO_NUM_13)

int sd_fatfs_init(void);
#endif
