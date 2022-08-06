#include "pico/printf.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"
#include "pio_write.pio.h"

#define BLK_PIN 10
#define RST_PIN 11
#define DC_PIN 12
#define CS_PIN 13
#define SCK_PIN 14
#define SDA_PIN 15
#define LED_PIN 25

PIO pio = pio0;
uint sm = 0;

int byteCountToWrite = 0;

void write_byte(uint8_t byte)
{
    byteCountToWrite++;
    pio_sm_put_blocking(pio, sm, ((uint32_t)byte) << 24);
}

void set_dc_pin(uint pin, bool isData)
{
    while (byteCountToWrite > 0)
    {
        // Wait for IRQ0 from PIO0
    }
    gpio_put(pin, isData);
}

void fill_lcd(uint16_t color)
{
    uint64_t startT = to_us_since_boot(get_absolute_time());
    set_dc_pin(DC_PIN, false);
    write_byte(0x2c);

    set_dc_pin(DC_PIN, true);
    for (int i = 0; i < 320; i++)
    {
        for (int j = 0; j < 240; j++)
        {
            write_byte((uint8_t)((color & 0xff00) >> 8));
            write_byte((uint8_t)(color & 0x00ff));
        }
    }

    uint64_t endT = to_us_since_boot(get_absolute_time());
    printf("Filled LCD takes %lld us\r\n", endT - startT);

    set_dc_pin(DC_PIN, false);
    write_byte(0x00);
}

static inline void pio_write_program_init(PIO pio, uint sm, uint offset)
{
    pio_sm_config c = pio_write_program_get_default_config(offset);
    sm_config_set_out_pins(&c, SDA_PIN, 1);
    sm_config_set_sideset_pins(&c, CS_PIN); // 2 sideset pins: CS = 13, SCK = 14
    sm_config_set_out_shift(&c, false, false, 8); // Shift left, autopull disabled, pull_threshold = 8

    // Default sys clock = 125MHz
    //sm_config_set_clkdiv(&c, 25.0f); // 5.00MHz
    //sm_config_set_clkdiv(&c, 12.5f); // 10.00MHz (Max clk in ILI9341 datasheet)
    sm_config_set_clkdiv(&c, 4.0f); // 31.25MHz (Unstable, need reset some times)
    //sm_config_set_clkdiv(&c, 2.0f); // 62.50MHz (Unstable, need reset many times)
    //sm_config_set_clkdiv(&c, 1.5f); // 83.33MHz (Very Unstable, will miss data)

    pio_gpio_init(pio, CS_PIN);
    pio_gpio_init(pio, SCK_PIN);
    pio_gpio_init(pio, SDA_PIN);

    pio_sm_set_consecutive_pindirs(pio, sm, CS_PIN, 3, true); // Pin Number: CS = 13, SCK = 14, SDA = 15

    // Initialize pins' value: CS = high, SCK = low, SDA = low
    pio_sm_set_pins_with_mask(pio, sm, 1u << CS_PIN, (1u << CS_PIN) | (1u << SCK_PIN) | (1u << SDA_PIN));

    // Enable IRQs from PIO0-SM0 to system PIO0_IRQ_0
    pio_set_irq0_source_enabled(pio, pis_interrupt0, true);

    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

void pio_irq_handler(void)
{
    if (pio_interrupt_get(pio, 0))
    {
        byteCountToWrite--;
        pio_interrupt_clear(pio, 0);
    }
}

int main(void)
{
    stdio_uart_init();

    irq_set_enabled(PIO0_IRQ_0, 1);
    irq_add_shared_handler(PIO0_IRQ_0, pio_irq_handler, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);

    uint offset = pio_add_program(pio, &pio_write_program);
    pio_write_program_init(pio, sm, offset);

    gpio_init_mask((1u << BLK_PIN) | (1u << RST_PIN) | (1u << DC_PIN));
    gpio_set_dir_out_masked((1u << BLK_PIN) | (1u << RST_PIN) | (1u << DC_PIN));

    gpio_put(RST_PIN, 1);
    sleep_ms(5);
    gpio_put(RST_PIN, 0);
    sleep_ms(5);
    gpio_put(RST_PIN, 1);
    sleep_ms(5);

    // Memory Access Control
    set_dc_pin(DC_PIN, false);
    write_byte(0x36);
    set_dc_pin(DC_PIN, true);
    write_byte(0x48);

    // Colour Mode: 16-bits
    set_dc_pin(DC_PIN, false);
    write_byte(0x3A);
    set_dc_pin(DC_PIN, true);
    write_byte(0x55);

    // Frame Rate Control: fosc / 1, 61Hz
    set_dc_pin(DC_PIN, false);
    write_byte(0xB1);
    set_dc_pin(DC_PIN, true);
    write_byte(0x00);
    write_byte(0x1F);

    sleep_ms(120);

    // Exit Sleep
    set_dc_pin(DC_PIN, false);
    write_byte(0x11);

    sleep_ms(120);

    // Display on
    set_dc_pin(DC_PIN, false);
    write_byte(0x29);

    // Enable LCD Backlight
    gpio_put(BLK_PIN, 1);

    while (1)
    {
        fill_lcd(0x0000);
        sleep_ms(1000);
        fill_lcd(0xf800);
        sleep_ms(1000);
        fill_lcd(0x07e0);
        sleep_ms(1000);
        fill_lcd(0x001f);
        sleep_ms(1000);
        fill_lcd(0xffe0);
        sleep_ms(1000);
        fill_lcd(0x07ff);
        sleep_ms(1000);
        fill_lcd(0xf81f);
        sleep_ms(1000);
        fill_lcd(0xffff);
        sleep_ms(1000);
    }

    return 0;
}
