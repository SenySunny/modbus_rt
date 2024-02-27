#include "pika_hal.h"
#include "aht20_driver.h"

// AHT20 device address
#define AHT20_SLAVE_ADDRESS 0x38
// Command to initiate AHT20 measurement

#define AHT20_INIT_CMD 0xBA

#define AHT20_SDA "P41"
#define AHT20_SCL "P42"

pika_dev* aht20_iic;

int aht20_reset(void){
    // Send initialization command to AHT20
    uint8_t init_cmd = AHT20_INIT_CMD;
    if (pika_hal_write(aht20_iic, &init_cmd, 1) != 1) {
        return -1; // Error
    }
    return 0;
}

int aht20_init(void) {
    // Open the I2C device
    if (aht20_iic != NULL){
        return 0;
    }
    aht20_iic = pika_hal_open(PIKA_HAL_SOFT_IIC, "soft_iic");

    pika_hal_SOFT_IIC_config iic_cfg = {0};
    // Configure the IIC device
    iic_cfg.slave_addr = AHT20_SLAVE_ADDRESS;
    iic_cfg.SCL = pika_hal_open(PIKA_HAL_GPIO, AHT20_SCL);
    iic_cfg.SDA = pika_hal_open(PIKA_HAL_GPIO, AHT20_SDA);

    pika_hal_ioctl(aht20_iic, PIKA_HAL_IOCTL_CONFIG, &iic_cfg);
    pika_hal_ioctl(aht20_iic, PIKA_HAL_IOCTL_ENABLE);
    // aht20_reset();
    pika_sleep_ms(100);
    return 0; // Success
}

// Function to read data from AHT20
int aht20_read(AHT20_Data* data) {

    uint8_t state = 0;
    int retry = 0;
    do{
        pika_sleep_ms(10);
        if (pika_hal_read(aht20_iic, &state, 1) != 1){
            return -1;
        };
        retry++;
    }while((state & 0x18) != 0x18 && retry < 10);

    if ((state & 0x18) != 0x18){
        return -1;
    }

    // Send measurement command to AHT20
    uint8_t measure_cmd[3] = {0xAC, 0x33, 0x00};
    if (pika_hal_write(aht20_iic, &measure_cmd, 3) != 3) {
        return -1; // Error
    }
    // Read the measurement results
    uint8_t rawData[6] = {0};
    pika_sleep_ms(100);
    if (pika_hal_read(aht20_iic, &rawData, 6) != 6) {
        return -1; // Error
    }

    pika_debug("aht20 raw:[0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X]", rawData[0], rawData[1], rawData[2], rawData[3], rawData[4], rawData[5]);

    // Parse the raw data into the structure
    // This depends on the data format specified in the AHT20 datasheet
    // You may need to adjust this parsing according to the actual data format
    data->status = rawData[0];
    data->humidity = ((rawData[1] << 12) | (rawData[2] << 4) | (rawData[3] >> 4)) * 100.0 / (1 << 20);
    data->temperature = ((rawData[3] & 0x0F) << 16 | (rawData[4] << 8) | rawData[5]) * 200.0 / (1 << 20) - 50.0;

    pika_debug("aht20 data: status: 0x%02X, temperature: %f, humidity: %f", data->status, data->temperature, data->humidity);
    return 0; // Success
}
