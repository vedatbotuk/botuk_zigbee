#ifndef TEST_BME68X_H
#define TEST_BME68X_H

#include <stdint.h>

#define BME68X_OK 0
#define BME68X_I2C_INTF 1
#define BME68X_ENABLE 1
#define BME68X_FORCED_MODE 1
#define BME68X_FILTER_OFF 0
#define BME68X_ODR_NONE 0

struct bme68x_data {
    float temperature;
    float pressure;
    float humidity;
    float gas_resistance;
};

struct bme68x_conf {
    uint8_t os_hum;
    uint8_t os_pres;
    uint8_t os_temp;
    uint8_t filter;
    uint8_t odr;
};

struct bme68x_heatr_conf {
    uint8_t enable;
    uint16_t heatr_temp;
    uint16_t heatr_dur;
};

struct bme68x_dev {
    int8_t (*read)(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
    int8_t (*write)(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
    void (*delay_us)(uint32_t period, void *intf_ptr);
    uint8_t intf;
    void *intf_ptr;
    int32_t amb_temp;
};

int8_t bme68x_init(struct bme68x_dev *dev);
int8_t bme68x_set_conf(const struct bme68x_conf *conf, struct bme68x_dev *dev);
int8_t bme68x_set_heatr_conf(uint8_t op_mode, const struct bme68x_heatr_conf *conf, struct bme68x_dev *dev);
int8_t bme68x_set_op_mode(uint8_t op_mode, struct bme68x_dev *dev);
uint32_t bme68x_get_meas_dur(uint8_t op_mode, const struct bme68x_conf *conf, struct bme68x_dev *dev);
int8_t bme68x_get_data(uint8_t op_mode, struct bme68x_data *data, uint8_t *n_fields, struct bme68x_dev *dev);

#endif
