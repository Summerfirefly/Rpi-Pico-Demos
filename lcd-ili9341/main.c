#include "pico/stdlib.h"

uint BLK_PIN = 10;
uint RST_PIN = 11;
uint DC_PIN = 12;
uint CS_PIN = 13;
uint SDA_PIN = 14;
uint SCK_PIN = 15;

void write_byte(uint8_t byte)
{
    gpio_put(SCK_PIN, 0);
    for (int i = 0; i < 8; i++)
    {
        gpio_put(SDA_PIN, (byte & 0x80) != 0);
        gpio_put(SCK_PIN, 1);
        byte = byte << 1;
        gpio_put(SCK_PIN, 0);
    }
}

void write_cmd(uint8_t cmd)
{
    gpio_put(DC_PIN, 0);
    gpio_put(CS_PIN, 0);
    write_byte(cmd);
    gpio_put(CS_PIN, 1);
}

void write_data(uint8_t data)
{
    gpio_put(DC_PIN, 1);
    gpio_put(CS_PIN, 0);
    write_byte(data);
    gpio_put(CS_PIN, 1);
}

void fill_lcd(uint16_t color)
{
    write_cmd(0x2c);
    for (int i = 0; i < 320; i++)
    {
        for (int j = 0; j < 240; j++)
        {
            write_data(color >> 8);
            write_data(color & 0x00ff);
        }
    }
    write_cmd(0x00);
}

int main(void)
{
    // SCK - GP15
    // SDA - GP14
    // CS - GP13
    // DC - GP12
    // RST - GP11
    // BLK - GP10

    gpio_init(BLK_PIN);
    gpio_init(RST_PIN);
    gpio_init(DC_PIN);
    gpio_init(CS_PIN);
    gpio_init(SDA_PIN);
    gpio_init(SCK_PIN);
    gpio_set_dir(BLK_PIN, GPIO_OUT);
    gpio_set_dir(RST_PIN, GPIO_OUT);
    gpio_set_dir(DC_PIN, GPIO_OUT);
    gpio_set_dir(CS_PIN, GPIO_OUT);
    gpio_set_dir(SDA_PIN, GPIO_OUT);
    gpio_set_dir(SCK_PIN, GPIO_OUT);

    gpio_put(CS_PIN, 1);
    gpio_put(SCK_PIN, 0);
    gpio_put(BLK_PIN, 1);

    gpio_put(RST_PIN, 1);
    sleep_ms(5);
    gpio_put(RST_PIN, 0);
    sleep_ms(5);
    gpio_put(RST_PIN, 1);
    sleep_ms(5);

    write_cmd(0x36);    // Memory Access Control
    write_data(0x48);

    write_cmd(0x3A);
    write_data(0x55);

    write_cmd(0xB1);
    write_data(0x00);
    write_data(0x18);

    write_cmd(0xB6);    // Display Function Control
    write_data(0x08);
    write_data(0x82);
    write_data(0x27);

    write_cmd(0x11);    //Exit Sleep
    sleep_ms(120);

    write_cmd(0x29);    //Display on

    while (1)
    {
        //fill_lcd(0xf800);
        //sleep_ms(1000);
        //fill_lcd(0x07e0);
        //sleep_ms(1000);
        //fill_lcd(0x001f);
        //sleep_ms(1000);
        fill_lcd(0xffe0);
        sleep_ms(1000);
    }

    return 0;
}
