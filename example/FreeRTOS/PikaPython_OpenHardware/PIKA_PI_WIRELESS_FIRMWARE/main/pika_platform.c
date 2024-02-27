#include "PikaObj.h"
#include "driver/uart.h"
#include "driver/usb_serial_jtag.h"
#include <time.h>

char pika_platform_getchar() {
  while (1) {
    char buff[1] = {0};
    if (usb_serial_jtag_read_bytes(buff, 1, 100) > 0) {
      return buff[0];
    }
    vTaskDelay(1);
  }
}

int pika_platform_putchar(char ch) {
  usb_serial_jtag_write_bytes(&ch, 1, 0);
  return 0;
}

int64_t pika_platform_get_tick(void) {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

void pika_platform_reboot(void) {
  /* reboot */
  abort();
}

FILE *pika_platform_fopen(const char *filename, const char *modes) {
  return fopen(filename, modes);
}

int pika_platform_fclose(FILE *fp) { return fclose(fp); }

int pika_platform_fseek(FILE *fp, long offset, int whence) {
  return fseek(fp, offset, whence);
}

long pika_platform_ftell(FILE *fp) { return ftell(fp); }

size_t pika_platform_fread(void *ptr, size_t size, size_t count, FILE *fp) {
  return fread(ptr, size, count, fp);
}

size_t pika_platform_fwrite(const void *ptr, size_t size, size_t count,
                            FILE *fp) {
  return fwrite(ptr, size, count, fp);
}
