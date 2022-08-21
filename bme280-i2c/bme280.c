#include "bme280.h"
#include "pico/stdlib.h"

i2c_inst_t *i2c = i2c0;
uint bme280_i2c_addr = 0x77;
uint bme280_sda = 20;
uint bme280_scl = 21;

uint8_t cal_press_temp[24];
uint8_t cal_humi[8];
uint8_t messure_data[8];
int32_t t_fine;


void bme280_read(uint8_t reg_addr, uint8_t *dst, uint size);
void bme280_write(uint8_t reg_addr, uint8_t data);
int32_t get_temp(void);
uint32_t get_press(void);
uint32_t get_humi(void);


void bme280_init(void)
{
    gpio_set_function(bme280_sda, GPIO_FUNC_I2C);
    gpio_set_function(bme280_scl, GPIO_FUNC_I2C);
    i2c_init(i2c, 100000);

    bme280_reset();

    // Init cal data
    bool isready = false;
    while (!isready)
    {
        uint8_t status;
        bme280_read(0xf3, &status, 1);
        isready = (status & 0x1) == 0;
    }

    bme280_read(0x88, cal_press_temp, 24);
    bme280_read(0xa1, cal_humi, 1);
    bme280_read(0xe1, cal_humi + 1, 7);

    // Set config
    bme280_write(0xf2, 0x02); // reg 0xf2, humidity oversampling x 2
    bme280_write(0xf4, 0x6f); // reg 0xf4, pressure & temperature oversampling x 4, normal mode
    bme280_write(0xf5, 0x48); // reg 0xf5, t_standby 125ms, filter x 4, disable 3-wire SPI
}

void bme280_reset(void)
{
    bme280_write(0xe0, 0xb6);
}

void bme280_get_data(int32_t *temp, uint32_t *press, uint32_t *humi)
{
    bme280_read(0xf7, messure_data, 8);
    int32_t t = get_temp();
    if (temp != NULL)
    {
        *temp = t;
    }

    if (press != NULL)
    {
        *press = get_press();
    }

    if (humi != NULL)
    {
        *humi = get_humi();
    }
}


void bme280_read(uint8_t reg_addr, uint8_t *dst, uint size)
{
    i2c_write_blocking(i2c, bme280_i2c_addr, &reg_addr, 1, false);
    i2c_read_blocking(i2c, bme280_i2c_addr, dst, size, false);
}

void bme280_write(uint8_t reg_addr, uint8_t data)
{
    uint8_t arr[] = { reg_addr, data };
    i2c_write_blocking(i2c, bme280_i2c_addr, arr, 2, false);
}

int32_t get_temp(void)
{
    uint16_t digT1 = *((uint16_t *)(cal_press_temp));
    int16_t digT2 = *((int16_t *)(cal_press_temp + 2));
    int16_t digT3 = *((int16_t *)(cal_press_temp + 4));

    int32_t data =
        ((uint32_t)messure_data[3] << 12) |
        ((uint32_t)messure_data[4] << 4) |
        ((uint32_t)messure_data[5] >> 4);

    int32_t var1, var2;
    var1 = (data >> 3) - ((int32_t)digT1 << 1);
    var1 *= (int32_t)digT2;
    var1 >>= 11;

    var2 = (data >> 4) - (int32_t)digT1;
    var2 = (var2 * var2) >> 12;
    var2 *= (int32_t)digT3;
    var2 >>= 14;

    t_fine = var1 + var2;
    return (t_fine * 5 + 128) >> 8;
}

uint32_t get_press(void)
{
    uint16_t digP1 = *((uint16_t *)(cal_press_temp + 6));
    int16_t digP2 = *((int16_t *)(cal_press_temp + 8));
    int16_t digP3 = *((int16_t *)(cal_press_temp + 10));
    int16_t digP4 = *((int16_t *)(cal_press_temp + 12));
    int16_t digP5 = *((int16_t *)(cal_press_temp + 14));
    int16_t digP6 = *((int16_t *)(cal_press_temp + 16));
    int16_t digP7 = *((int16_t *)(cal_press_temp + 18));
    int16_t digP8 = *((int16_t *)(cal_press_temp + 20));
    int16_t digP9 = *((int16_t *)(cal_press_temp + 22));

    int32_t data =
        ((uint32_t)messure_data[0] << 12) |
        ((uint32_t)messure_data[1] << 4) |
        ((uint32_t)messure_data[2] >> 4);

    int64_t var1, var2, p;
    var1 = (int64_t)t_fine - 128000;
    var2 = var1 * var1 * (int64_t)digP6;
    var2 += (var1 * (int64_t)digP5) << 17;
    var2 += (int64_t)digP4 << 35;
    var1 = ((var1 * var1 * (int64_t)digP3) >> 8) + ((var1 * (int64_t)digP2) << 12);
    var1 = (((int64_t)1 << 47) + var1) * ((int64_t)digP1) >> 33;
    if (var1 == 0)
    {
        return 0;
    }

    p = 1048576 - data;
    p = ((p << 31) - var2) * 3125 / var1;
    var1 = ((int64_t)digP9 * (p >> 13) * (p >> 13)) >> 25;
    var2 = ((int64_t)digP8 * p) >> 19;
    p = ((p + var1 + var2) >> 8) + ((int64_t)digP7 << 4);
    return (uint32_t)p;
}

uint32_t get_humi(void)
{
    uint8_t digH1 = cal_humi[0];
    int16_t digH2 = *((int16_t *)(cal_humi + 1));
    uint8_t digH3 = cal_humi[3];

    int16_t digH4 = (int16_t)((int8_t)cal_humi[4]) << 4;
    digH4 |= cal_humi[5] & 0x0f;
    int16_t digH5 = (int16_t)((int8_t)cal_humi[6]) << 4;
    digH5 |= (cal_humi[5] >> 4) & 0x0f;

    int8_t digH6 = (int8_t)cal_humi[7];

    int32_t data =
        ((uint32_t)messure_data[6] << 8) |
        ((uint32_t)messure_data[7]);

    int32_t tmp = t_fine - 76800;
    int32_t var1 = (data << 14) - ((int32_t)digH4 << 20) - (int32_t)digH5 * tmp;
    var1 += 16384;
    var1 >>= 15;
    int32_t var2 = (tmp * (int32_t)digH6) >> 10;
    var2 *= ((tmp * (int32_t)digH3) >> 11) + 32768;
    var2 >>= 10;
    var2 += 2097152;
    var2 = var2 * (int32_t)digH2 + 8192;
    var2 >>= 14;
    tmp = var1 * var2;
    tmp -= ((((tmp >> 15) * (tmp >> 15)) >> 7) * (int32_t)digH1) >> 4;
    tmp = tmp < 0 ? 0 : tmp;
    tmp = tmp > 419430400 ? 419430400 : tmp;
    return (uint32_t)(tmp >> 12);
}