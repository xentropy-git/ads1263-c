#include <stdio.h>
#include <lgpio.h>
#include <unistd.h>
#include "main.h"

#define DRDY_PIN  17
#define RST_PIN    18
#define CS_PIN     22


int main() {
    printf("Setting up ADS1263\n");

    printf("Configuring GPIO\n");
    int gpio_h = ads1263_config_gpio(0);
    if (gpio_h < 0) {
        printf("Failed to configure ADS1263\n");
        return -1;
    }

    printf("Configuring SPI\n");
    int spi_h = ads1263_config_spi(0, 0);

    if (spi_h < 0) {
        printf("Failed to configure ADS1263\n");
        return -1;
    }

    printf("ADS1263 setup complete\n");

    ads1263_reset(gpio_h);

    printf("Waiting for DRDY\n");
    ads1263_wait_for_ready(gpio_h);

    printf("Reading registers\n");
    int ok;

    ok = lgGpioWrite(gpio_h, CS_PIN, 0);
    if (ok < 0) {
        printf("Failed to write CS pin: %d\n", ok);
        return -1;
    }
    // Write command: read register 0
    char cmd[] = {0x20 | 0x00, 0x00};
    lgSpiWrite(spi_h, cmd, 2);

    // buffer
    char buffer[32];

    // buffer pointer
    char *response = buffer;

    int count = lgSpiRead(spi_h, buffer, 32);

    printf("Read %d bytes\n", count);

    lgGpioWrite(gpio_h, CS_PIN, 1);
    printf("Register 0: %d\n", buffer[0]);

    for (int i = 0; i < count; i++) {
        printf("0x%02x ", buffer[i]);
    }

    printf("Cleaning up\n");
    lgSpiClose(spi_h);
    lgGpiochipClose(gpio_h);
}

void ads1263_reset(int gpio_h) {
    lgGpioWrite(gpio_h, RST_PIN, 1);
    usleep(200000);
    lgGpioWrite(gpio_h, RST_PIN, 0);
    usleep(200000);
    lgGpioWrite(gpio_h, RST_PIN, 1);
    usleep(200000);
}

void ads1263_wait_for_ready(int gpio_h) {
    int ready = 1;
    int timeout = 1000;
    while (ready) {
        printf(".");
        ready = lgGpioRead(gpio_h, DRDY_PIN);
        usleep(1000);
        timeout--;
        if (timeout == 0) {
            printf("Timeout waiting for DRDY\n");
            return;
        }
    }
}


int ads1263_config_gpio(int device) {
    int h;
    int ok;
    
    h = lgGpiochipOpen(device);
    if (h < 0) {
        printf("Failed to open gpiochip: %d\n", h);
        return -1;
    }

    ok = lgGpioClaimInput(h, LG_SET_PULL_UP, DRDY_PIN);
    if (ok < 0) {
        printf("Failed to claim DRDY pin: %d\n", h);
        return -1;
    }

    ok = lgGpioClaimOutput(h, 0, RST_PIN, 0);
    if (ok < 0) {
        printf("Failed to claim RST pin: %d\n", h);
        return -1;
    }

    ok = lgGpioClaimOutput(h, 0, CS_PIN, 0);
    if (ok < 0) {
        printf("Failed to claim CS pin: %d\n", h);
        return -1;
    }

    printf("Configured ADS1263\n");
    return h;
}

int ads1263_config_spi(int device, int channel) {
    int h;
    int ok;

    int mode = 0b01;

    h = lgSpiOpen(device, channel, 2000000, mode);
    if (h < 0) {
        printf("Failed to open SPI device: %d\n", h);
        return -1;
    }

    printf("Configured SPI\n");
    return h;
}
