/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include <unistd.h>
#include <pthread.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "driver/usb_serial_jtag.h"
#include "esp_vfs_dev.h"

#include "esp_pthread.h"

#include "modbus_rtu.h"

int modbus_rtu_slave_open_test( void ) ;
void *wifi_thread_entry(void * arg);
void app_main(void)
{
    usb_serial_jtag_driver_config_t usb_serial_jtag_config = {
        .rx_buffer_size = 1024 * 8, .tx_buffer_size = 1024};
    usb_serial_jtag_driver_install(&usb_serial_jtag_config);

    esp_vfs_usb_serial_jtag_use_driver();

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), WiFi%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    // for (int i = 10; i >= 0; i--) {
    //     printf("Restarting in %d seconds...\n", i);
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }
    // printf("Restarting now.\n");
    // fflush(stdout);

    int ret = MODBUS_RT_EOK;
    if(MODBUS_RT_EOK != (ret = modbus_rtu_slave_open_test())) {
        printf("modbus_rtu_slave_open faild.\n");
    }
    printf("modbus_rtu_slave_open success.\n");

    //wifi pthread
    pthread_t thread_wifi;
    ret = pthread_create(&thread_wifi, NULL, wifi_thread_entry, NULL);
    
    // int len = 0;
    // uint8_t buf_t[1024] = {0};
    // uart_type_dev *dev = os_uart_open("uart2", 115200, 8, 'n', 1, 0);
    // os_uart_send(dev,(uint8_t *)"12345\r\n", 7);
    while(true){
        // len = os_uart_receive(dev, buf_t, 1024, 100,10);
        // if(len > 0) {
        //     os_uart_send(dev, buf_t, len);
        // }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
    esp_restart();
}
