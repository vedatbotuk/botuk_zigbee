#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "air_quality.h"
#include "bme68x.h"
#include "bsec_interface.h"
#include "driver/i2c_master.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "freertos/task.h"
#include "update_cluster.h"
#include "random_utils.h"

void vTaskDelay(TickType_t ticks)
{
    (void)ticks;
}

void vTaskDelete(void *task)
{
    (void)task;
}

int8_t bsec_init(void)
{
    return BSEC_OK;
}

int8_t bsec_update_subscription(const bsec_sensor_configuration_t *requested_virtual_sensors,
                               uint8_t n_requested_virtual_sensors,
                               bsec_sensor_configuration_t *required_sensor_settings,
                               uint8_t *n_required_sensor_settings)
{
    (void)requested_virtual_sensors;
    (void)n_requested_virtual_sensors;
    (void)required_sensor_settings;
    (void)n_required_sensor_settings;
    return BSEC_OK;
}

void bsec_sensor_control(int64_t curr_time_ns, bsec_bme_settings_t *settings)
{
    (void)curr_time_ns;
    memset(settings, 0, sizeof(*settings));
    settings->trigger_measurement = 0;
    settings->next_call = 0;
}

void bsec_do_steps(const bsec_input_t *inputs, uint8_t n_inputs, bsec_output_t *outputs, uint8_t *n_outputs)
{
    (void)inputs;
    (void)n_inputs;
    (void)outputs;
    if (n_outputs != NULL)
    {
        *n_outputs = 0;
    }
}

int8_t bme68x_init(struct bme68x_dev *dev)
{
    (void)dev;
    return BME68X_OK;
}

int8_t bme68x_set_conf(const struct bme68x_conf *conf, struct bme68x_dev *dev)
{
    (void)conf;
    (void)dev;
    return BME68X_OK;
}

int8_t bme68x_set_heatr_conf(uint8_t op_mode, const struct bme68x_heatr_conf *conf, struct bme68x_dev *dev)
{
    (void)op_mode;
    (void)conf;
    (void)dev;
    return BME68X_OK;
}

int8_t bme68x_set_op_mode(uint8_t op_mode, struct bme68x_dev *dev)
{
    (void)op_mode;
    (void)dev;
    return BME68X_OK;
}

uint32_t bme68x_get_meas_dur(uint8_t op_mode, const struct bme68x_conf *conf, struct bme68x_dev *dev)
{
    (void)op_mode;
    (void)conf;
    (void)dev;
    return 0;
}

int8_t bme68x_get_data(uint8_t op_mode, struct bme68x_data *data, uint8_t *n_fields, struct bme68x_dev *dev)
{
    (void)op_mode;
    (void)data;
    (void)dev;
    if (n_fields != NULL)
    {
        *n_fields = 0;
    }
    return BME68X_OK;
}

esp_err_t i2c_new_master_bus(const i2c_master_bus_config_t *bus_cfg, i2c_master_bus_handle_t *bus_handle)
{
    (void)bus_cfg;
    (void)bus_handle;
    return ESP_OK;
}

esp_err_t i2c_master_bus_add_device(i2c_master_bus_handle_t bus_handle, const i2c_device_config_t *dev_cfg,
                                    i2c_master_dev_handle_t *dev_handle)
{
    (void)bus_handle;
    (void)dev_cfg;
    (void)dev_handle;
    return ESP_OK;
}

esp_err_t i2c_master_transmit_receive(i2c_master_dev_handle_t handle, const uint8_t *tx_buffer, size_t tx_len,
                                       uint8_t *rx_buffer, size_t rx_len, int32_t timeout)
{
    (void)handle;
    (void)tx_buffer;
    (void)tx_len;
    (void)rx_buffer;
    (void)rx_len;
    (void)timeout;
    return ESP_OK;
}

esp_err_t i2c_master_transmit(i2c_master_dev_handle_t handle, const uint8_t *data, size_t len, int32_t timeout)
{
    (void)handle;
    (void)data;
    (void)len;
    (void)timeout;
    return ESP_OK;
}

esp_err_t nvs_flash_init(void)
{
    return ESP_OK;
}

bool connection_status(void)
{
    return true;
}

int random_utils_generate(int max_value)
{
    return (max_value > 0) ? 0 : -1;
}

int64_t esp_timer_get_time(void)
{
    return 0;
}

void zb_update_iaq(uint16_t iaq) { (void)iaq; }
void zb_update_iaq_accuracy(uint8_t accuracy) { (void)accuracy; }
void zb_update_temp(int16_t temperature) { (void)temperature; }
void zb_update_hum(int humidity) { (void)humidity; }
void zb_update_pressure(int16_t pressure) { (void)pressure; }
void zb_update_co2(float eco2) { (void)eco2; }
void zb_update_bvoc(float bvoc) { (void)bvoc; }
void zb_update_gas_resistance(float gas_resistance) { (void)gas_resistance; }
void esp_rom_delay_us(uint32_t period) { (void)period; }

static void test_accuracy_strings(void)
{
    assert(strcmp(accuracy_str(0), "Stabilizing") == 0);
    assert(strcmp(accuracy_str(1), "Uncertain") == 0);
    assert(strcmp(accuracy_str(2), "Calibrating") == 0);
    assert(strcmp(accuracy_str(3), "Calibrated") == 0);
    assert(strcmp(accuracy_str(99), "Unknown") == 0);
}

int main(void)
{
    test_accuracy_strings();
    return 0;
}
