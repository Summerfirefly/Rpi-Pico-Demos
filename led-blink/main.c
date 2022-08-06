#include "pico/stdlib.h"

int main(void)
{
    uart_init(uart0, 115200);
    gpio_set_function(16, GPIO_FUNC_UART);
    gpio_set_function(17, GPIO_FUNC_UART);

    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    while (1)
    {
        uart_puts(uart0, "Hello LED!\r\n");
        gpio_put(25, 1);
	sleep_ms(1000);
	gpio_put(25, 0);
	sleep_ms(1000);
    }

    return 0;
}
