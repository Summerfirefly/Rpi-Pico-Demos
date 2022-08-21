#include "pico/stdlib.h"
#include "pico/printf.h"
#include "bme280.h"

#define UART_TX 0
#define UART_RX 1

int main(void)
{
    stdio_uart_init_full(uart0, 115200, UART_TX, UART_RX);
    bme280_init();

    while (1)
    {
        int32_t temp;
        uint32_t press;
        uint32_t humi;
        bme280_get_data(&temp, &press, &humi);
        printf("temperature = %5.2f, pressure = %9.2f, humidity = %6.2f\r",
            temp / 100.0f, press / 256.0f, humi / 1024.0f);
        sleep_ms(1000);
    }

    return 0;
}
