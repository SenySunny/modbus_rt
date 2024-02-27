#include "fs800e_driver.h"

#define FS800E_RX "P16"
#define FS800E_TX "P15"

#define FS800E_RCV_BUF_LEN 1000

typedef struct _fs800e_ {
    bool busy;
    pika_dev* uart;
    struct {
        char buf[FS800E_RCV_BUF_LEN];
        uint16_t pos;
    }rcv;
}fs800e_t;

fs800e_t s_fs800e = {
    .busy = false,
    .rcv = {
        .pos = 0,
    },
};

static int _wait_answer(const char *answer, int timeout);
static int _write(void *buf, size_t len);
static int _send_cmd(void *buf, size_t len, const char *answer, int timeout);
static void _read_from_uart(void);
static void _rcv_buf_clear(void);

int fs800e_init(void) {
    s_fs800e.uart = pika_hal_open(PIKA_HAL_UART, "UART1");

    pika_hal_UART_config uart_cfg = {
        .baudrate           = PIKA_HAL_UART_BAUDRATE_115200,
        .data_bits          = PIKA_HAL_UART_DATA_BITS_8,
        .stop_bits          = PIKA_HAL_UART_STOP_BITS_1,
        .parity             = PIKA_HAL_UART_PARITY_NONE,
        .flow_control       = PIKA_HAL_UART_FLOW_CONTROL_NONE,
    };

    uart_cfg.RX = pika_hal_open(PIKA_HAL_GPIO, FS800E_RX);
    uart_cfg.TX = pika_hal_open(PIKA_HAL_GPIO, FS800E_TX);

    pika_hal_ioctl(s_fs800e.uart, PIKA_HAL_IOCTL_CONFIG, &uart_cfg);

    pika_hal_ioctl(s_fs800e.uart, PIKA_HAL_IOCTL_ENABLE);

    return 0;
}

static int _wait_answer(const char *answer, int timeout) {
    int timecnt = 0;

    if (answer == NULL) {
        return 0;
    }

    for (timecnt = 0; timecnt < timeout; timecnt++) {
        _read_from_uart();
        if (strstr(s_fs800e.rcv.buf, answer) != NULL) {
            return 0;
        }
        vTaskDelay(100);
    }

    return -1;
}

static int _write(void *buf, size_t len) {
    _rcv_buf_clear();
    return pika_hal_write(s_fs800e.uart, buf, len);
}

static int _send_cmd(void *buf, size_t len, const char *answer, int timeout) {
    _write(buf, len);
    return _wait_answer(answer, timeout);
}

static void _read_from_uart(void) {
    int len = 0;
    len = pika_hal_read(s_fs800e.uart, &s_fs800e.rcv.buf[s_fs800e.rcv.pos], FS800E_RCV_BUF_LEN - s_fs800e.rcv.pos);
    s_fs800e.rcv.pos += len;
}

static void _rcv_buf_clear(void) {
    _read_from_uart();
    memset(s_fs800e.rcv.buf, 0x00, FS800E_RCV_BUF_LEN);
    s_fs800e.rcv.pos = 0;
}

int fs800e_exit_transparent_mode(void) {
    return _send_cmd("+++", strlen("+++"), "OK", 10);
}

int fs800e_echo_off(void) {
    return _send_cmd("AT+E=OFF\r\n", strlen("AT+E=OFF\r\n"), "OK", 10);
}

int fs800e_at_test(void) {
    return _send_cmd("AT\r\n", strlen("AT\r\n"), "OK", 10);
}

