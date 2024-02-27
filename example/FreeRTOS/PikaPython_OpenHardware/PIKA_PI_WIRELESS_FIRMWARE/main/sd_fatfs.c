#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include "driver/gpio.h"
#include "driver/sdspi_host.h"
#include "driver/uart.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "hal/spi_types.h"
#include "sdmmc_cmd.h"

#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "pikaScript.h"
#include "protocol_examples_common.h"
#include "sd_fatfs.h"

static sdmmc_card_t* card;
static bool is_sd_mounted = false;

int sd_fatfs_init(void) {
    esp_err_t ret;
    const char mount_point[] = MOUNT_POINT;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024,
    };

    // Use settings defined above to initialize SD card and mount FAT
    // filesystem. Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience
    // functions. Please check its source code and implement error recovery when
    // developing production applications.

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI3_HOST;
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = SDSPI_MOSI,
        .miso_io_num = SDSPI_MISO,
        .sclk_io_num = SDSPI_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        printf("Failed to initialize SPI bus.\r\n");
        return -1;
    }

    // This initializes the slot without card detect (CD) and write protect (WP)
    // signals. Modify slot_config.gpio_cd and slot_config.gpio_wp if your board
    // has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SDSPI_CS;
    slot_config.host_id = host.slot;

    printf("Mounting filesystem...\r\n");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config,
                                  &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            printf(
                "Failed to mount filesystem. "
                "If you want the card to be formatted, set the "
                "CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.\r\n");
        } else {
            printf(
                "Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.\r\n",
                esp_err_to_name(ret));
        }
        return -1;
    } else {
        printf("Filesystem mounted\r\n");
        is_sd_mounted = true;
        sdmmc_card_print_info(stdout, card);
    }
    return 0;
}
