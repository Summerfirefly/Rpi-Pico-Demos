#include "pico/stdlib.h"

#define SCK_PIN 6
#define SDA_PIN 7
#define LED_PIN 25

uint8_t digits[] = {
    0x3fu, 0x06u, 0x5bu, 0x4fu, 0x66u,
    0x6du, 0x7du, 0x07u, 0x7fu, 0x6fu
};

void start()
{
    gpio_put(SDA_PIN, 0);
    gpio_put(SCK_PIN, 0);
    sleep_us(3);
}

void stop()
{
    gpio_put(SDA_PIN, 0);
    gpio_put(SCK_PIN, 1);
    sleep_us(3);
    gpio_put(SDA_PIN, 1);
}

void write_byte(uint8_t data)
{
    for (int i = 0; i < 8; i++)
    {
        gpio_put(SCK_PIN, 0);
        sleep_us(3);
        gpio_put(SDA_PIN, data & 0x1u);
        data >>= 1;
        gpio_put(SCK_PIN, 1);
        sleep_us(3);
    }

    gpio_set_dir(SDA_PIN, false);
    gpio_put(SCK_PIN, 0);
    sleep_us(3);
    gpio_put(SCK_PIN, 1);
    sleep_us(3);
    gpio_put(SCK_PIN, 0);
    sleep_us(3);
    gpio_set_dir(SDA_PIN, true);
}

int main(void)
{
    gpio_init(SCK_PIN);
    gpio_init(SDA_PIN);
    gpio_init(LED_PIN);

    gpio_set_dir_out_masked((1u << SCK_PIN) | (1u << SDA_PIN) | (1u << LED_PIN));

    gpio_put(SCK_PIN, 0);
    gpio_put(SDA_PIN, 1);
    gpio_put(SCK_PIN, 1);

    int num = 0;
    while (1)
    {
        start();
        write_byte(0x40u);
        stop();

        start();
        write_byte(0xc0u);
        write_byte(digits[num / 1000]);
        write_byte(digits[num / 100 % 10]);
        //write_byte(digits[num / 100 % 10] | 0x80);
        write_byte(digits[num / 10 % 10]);
        write_byte(digits[num % 10]);
        stop();

        start();
        write_byte(0x8cu);
        stop();

        num = (num + 1) % 10000;

        gpio_put(LED_PIN, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        sleep_ms(500);
    }

    return 0;
}
