#include "PikaVM.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/usb_serial_jtag.h"
#include "esp_chip_info.h"
#include "esp_core_dump.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs_dev.h"
#include "esp_vfs_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sdmmc_cmd.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "pikaScript.h"
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <time.h>

/* drivers */
#include "aht20_driver.h"
#include "beep_driver.h"
#include "fs800e_driver.h"
#include "lcd_driver.h"
#include "pika_hal.h"
#include "rs485_driver.h"
#include "sd_fatfs.h"

static volatile char gRXC = EOF;
static volatile uint8_t gNeedPut = 0;

#define USING_SD_CARD 1
#define USING_LCD 1
#define USING_AHT20 1
#define USING_BEEP 1
// #define USING_RS485 1
// #define USING_FS800E 1
// #define FORCE_ERAISE 1

void io_init(char *name, int dir, int val) {
  pika_dev *pin = pika_hal_open(PIKA_HAL_GPIO, name);
  pika_hal_GPIO_config cfg = {0};
  cfg.dir = dir;
  pika_hal_ioctl(pin, PIKA_HAL_IOCTL_CONFIG, &cfg);
  pika_hal_ioctl(pin, PIKA_HAL_IOCTL_ENABLE);
  pika_hal_write(pin, &val, sizeof(val));
  pika_hal_close(pin);
}

void eraise_task(void *pvParameter) {
  pika_dev *eraise_key = pika_hal_open(PIKA_HAL_GPIO, "P0");
  pika_hal_GPIO_config cfg = {0};
  cfg.dir = PIKA_HAL_GPIO_DIR_IN;
  cfg.pull = PIKA_HAL_GPIO_PULL_UP;
  pika_hal_ioctl(eraise_key, PIKA_HAL_IOCTL_CONFIG, &cfg);
  pika_hal_ioctl(eraise_key, PIKA_HAL_IOCTL_ENABLE);

  int pressed = 0;
  int max = 10;
  while (1) {
    /* sleep 10 ms */
    vTaskDelay(10 / portTICK_PERIOD_MS);
    uint32_t value = 0;
    pika_hal_read(eraise_key, &value, sizeof(value));
    if (0 == value) {
      pressed++;
      printf("[Info] erase key pressed(%d/%d)\n", pressed, max);
    } else {
      pressed = 0;
    }
    if (pressed >= max) {
      printf("[Info] erase key pressed(%d/%d)\n", pressed, max);
      printf("[Info] erase flash\n");
      remove(PIKA_SHELL_SAVE_APP_PATH);
      pika_platform_reboot();
    }
  }
}

void app_main(void) {
  usb_serial_jtag_driver_config_t usb_serial_jtag_config = {
      .rx_buffer_size = 1024 * 8, .tx_buffer_size = 1024};
  usb_serial_jtag_driver_install(&usb_serial_jtag_config);
  esp_vfs_usb_serial_jtag_use_driver();

  printf("Minimum free heap size: %" PRIu32 " bytes\n",
         esp_get_minimum_free_heap_size());

#if USING_SD_CARD
  int retry = 0;
  printf("Initializing SD card...\r\n");
  while (0 != sd_fatfs_init()) {
    /*加入指示SD卡初始化失败代码*/
    vTaskDelay(1000);
    retry++;
    if (retry > 10) {
      printf("Failed to init SD card, rebooting...\r\n");
      pika_platform_reboot();
    }
  }
#endif

#if USING_LCD
  printf("Initializing LCD...\r\n");
  LCD_Init();
  LCD_Fill(0, 0, Y_MAX_PIXEL, X_MAX_PIXEL, BLUE);
#endif

#if USING_AHT20
  printf("Initializing AHT20...\r\n");
  aht20_init();
  AHT20_Data data;
  aht20_read(&data);
  printf("Temperature: %f, Humidity: %f\r\n", data.temperature, data.humidity);
#endif

#if USING_BEEP
  printf("Initializing beep...\r\n");
  beep_init();
  beep_on();
  vTaskDelay(100);
  beep_off();
  printf("Beep initialization completed!\r\n");
#endif

#if USING_RS485
  printf("Initializing rs485...\r\n");
  char rs485_buf[100] = {0};
  int rs485_rcv_len = 0;
#define RS485_TEST_TIMEOUT 10
  int rs485_test_cnt = 0;
  rs485_init(PIKA_HAL_UART_BAUDRATE_9600);
  while (1) {
    vTaskDelay(1000);
    rs485_rcv_len = rs485_read(rs485_buf, sizeof(rs485_buf));
    if (rs485_rcv_len == 0) {
      rs485_test_cnt++;
      printf("RS485 did not receive data!\r\n");
    } else {
      rs485_test_cnt = 0;
    }

    if (rs485_test_cnt >= RS485_TEST_TIMEOUT) {
      printf("RS485 test failed!\r\n");
      break;
    }

    if (strstr(rs485_buf, "exit") != NULL) {
      printf("RS485 test completed!\r\n");
      break;
    }
    rs485_write(rs485_buf, rs485_rcv_len);
    memset(rs485_buf, 0, sizeof(rs485_buf));
  }
#endif

#if USING_FS800E
  printf("Initializing fs800e...\r\n");
  fs800e_init();
  vTaskDelay(1000);
  fs800e_exit_transparent_mode();
  vTaskDelay(1000);
  fs800e_echo_off();
  while (1) {
    vTaskDelay(1000);
    if (fs800e_at_test() == 0) {
      break;
    }
  }
  printf("FS800e initialization completed!\r\n");
#endif

  /* Print chip information */
  esp_chip_info_t chip_info;
  uint32_t flash_size;
  esp_chip_info(&chip_info);
  printf("This is %s chip with %d CPU core(s), WiFi%s%s%s, ", CONFIG_IDF_TARGET,
         chip_info.cores, (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
         (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
         (chip_info.features & CHIP_FEATURE_IEEE802154)
             ? ", 802.15.4 (Zigbee/Thread)"
             : "");

  unsigned major_rev = chip_info.revision / 100;
  unsigned minor_rev = chip_info.revision % 100;
  printf("silicon revision v%d.%d, ", major_rev, minor_rev);
  if (esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
    printf("Get flash size failed");
    return;
  }

  printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
         (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded"
                                                       : "external");

  printf("Minimum free heap size: %" PRIu32 " bytes\n",
         esp_get_minimum_free_heap_size());

  // for (int i = 10; i >= 0; i--) {
  //     printf("Restarting in %d seconds...\n", i);
  //     vTaskDelay(1000 / portTICK_PERIOD_MS);
  // }
  // printf("Restarting now.\n");
  // fflush(stdout);

#ifdef FORCE_ERAISE
  remove(PIKA_SHELL_SAVE_APP_PATH);
#endif

  // eraise_task
  xTaskCreate(eraise_task, "task_eriase", 1024 * 8, NULL, 10,
              NULL); /* try to boot from sd card */

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  PikaObj *root = newRootObj("pikaMain", New_PikaMain);
  FILE *fp = fopen(PIKA_SHELL_SAVE_APP_PATH, "rb");
  if (fp != NULL) {
    fclose(fp);
    printf("[Info] load app from sd card\n");
    obj_linkLibraryFile(root, PIKA_SHELL_SAVE_APP_PATH);
    obj_runModule(root, "main");
  } else {
    printf("[Info] load app from firmware\n");
    extern unsigned char pikaModules_py_a[];
    obj_linkLibrary(root, pikaModules_py_a);
  }

  pikaPythonShell(root);

  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
