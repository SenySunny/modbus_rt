#include "pika_hal.h"
#include "beep_driver.h"

#define BEEP_GPIO "P6"

pika_dev* beep;

static inline void _pika_hal_GPIO_write(pika_dev *dev, uint32_t val) {
  pika_hal_write(dev, &val, sizeof(val));
}

int beep_init(void) {
    beep = pika_hal_open(PIKA_HAL_GPIO, BEEP_GPIO);

    pika_hal_GPIO_config cfg_io = {0};

    cfg_io.dir = PIKA_HAL_GPIO_DIR_OUT;
    cfg_io.pull = PIKA_HAL_GPIO_PULL_DOWN;
    pika_hal_ioctl(beep, PIKA_HAL_IOCTL_CONFIG, &cfg_io);

    pika_hal_ioctl(beep, PIKA_HAL_IOCTL_ENABLE);

    return 0;
}

int beep_on(void) {
    _pika_hal_GPIO_write(beep, 1);

    return 0;
}

int beep_off(void) {
    _pika_hal_GPIO_write(beep, 0);

    return 0;
}

