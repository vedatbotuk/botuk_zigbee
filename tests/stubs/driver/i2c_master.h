#ifndef TEST_I2C_MASTER_H
#define TEST_I2C_MASTER_H

#include <stdint.h>

#define GPIO_NUM_3 3
#define GPIO_NUM_4 4
#define I2C_NUM_0 0
#define I2C_ADDR_BIT_LEN_7 0
#define I2C_CLK_SRC_DEFAULT 0

#define ESP_OK 0
#define ESP_ERR_INVALID_STATE 1
#define ESP_ERR_INVALID_ARG 2
#define ESP_ERROR_CHECK(x) do { esp_err_t __err = (x); if (__err != ESP_OK) return; } while (0)

typedef int esp_err_t;
typedef void *i2c_master_bus_handle_t;
typedef void *i2c_master_dev_handle_t;

typedef struct {
    int i2c_port;
    int sda_io_num;
    int scl_io_num;
    int clk_source;
    uint32_t glitch_ignore_cnt;
} i2c_master_bus_config_t;

typedef struct {
    uint8_t dev_addr_length;
    uint8_t device_address;
    uint32_t scl_speed_hz;
} i2c_device_config_t;

esp_err_t i2c_new_master_bus(const i2c_master_bus_config_t *bus_cfg, i2c_master_bus_handle_t *bus_handle);
esp_err_t i2c_master_bus_add_device(i2c_master_bus_handle_t bus_handle, const i2c_device_config_t *dev_cfg,
                                    i2c_master_dev_handle_t *dev_handle);
esp_err_t i2c_master_transmit_receive(i2c_master_dev_handle_t handle, const uint8_t *tx_buffer, size_t tx_len,
                                       uint8_t *rx_buffer, size_t rx_len, int32_t timeout);
esp_err_t i2c_master_transmit(i2c_master_dev_handle_t handle, const uint8_t *data, size_t len, int32_t timeout);

#endif
