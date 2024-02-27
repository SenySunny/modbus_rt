#include "rs485_driver.h"

#define RS485_RX "P17"
#define RS485_TX "P18"

pika_dev* rs485;

int rs485_init(PIKA_HAL_UART_BAUDRATE baudrate) {
    rs485 = pika_hal_open(PIKA_HAL_UART, "UART2");

    pika_hal_UART_config uart_cfg = {
        .baudrate  = baudrate,
        .data_bits = PIKA_HAL_UART_DATA_BITS_8,
        .stop_bits = PIKA_HAL_UART_STOP_BITS_1,
        .parity    = PIKA_HAL_UART_PARITY_NONE,
        .flow_control = PIKA_HAL_UART_FLOW_CONTROL_NONE,
    };

    uart_cfg.RX = pika_hal_open(PIKA_HAL_GPIO, RS485_RX);
    uart_cfg.TX = pika_hal_open(PIKA_HAL_GPIO, RS485_TX);

    pika_hal_ioctl(rs485, PIKA_HAL_IOCTL_CONFIG, &uart_cfg);

    pika_hal_ioctl(rs485, PIKA_HAL_IOCTL_ENABLE);

    return 0;
}

int rs485_write(void *buf, size_t len) {
    return pika_hal_write(rs485, buf, len);
}

int rs485_read(void *buf, size_t len) {
    return pika_hal_read(rs485, buf, len);
}
