#include "hardware/i2c.h"

void bme280_init(void);
void bme280_reset(void);
void bme280_get_data(int32_t *temp, uint32_t *press, uint32_t *humi);