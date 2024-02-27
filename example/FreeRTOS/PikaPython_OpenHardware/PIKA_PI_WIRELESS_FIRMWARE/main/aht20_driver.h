#ifndef AHT20_DRIVER_H
#define AHT20_DRIVER_H

#include <stdint.h>

// Define the structure to hold the temperature, humidity and status
typedef struct {
    float temperature;
    float humidity;
    uint8_t status;
} AHT20_Data;

int aht20_init(void);
int aht20_read(AHT20_Data* data);
#endif
