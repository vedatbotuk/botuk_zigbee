#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "bsec_interface.h"
#include "bme68x.h"

#include "nvs_flash.h"
#include "nvs.h"

#include "air_quality.h"

#include "update_cluster.h"
#include "macros.h"
#include "random_utils.h"

static const char *TAG_AIR_QUALITY = "AIR_QUALITY";

/* --- Global Handles --- */
i2c_master_dev_handle_t dev_handle;
struct bme68x_dev bme_dev;

/* --- I2C Callbacks --- */
int8_t bus_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    i2c_master_dev_handle_t handle = *(i2c_master_dev_handle_t *)intf_ptr;
    esp_err_t ret = i2c_master_transmit_receive(handle, &reg_addr, 1, reg_data, len, -1);
    return (ret == ESP_OK) ? 0 : -1;
}

int8_t bus_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    i2c_master_dev_handle_t handle = *(i2c_master_dev_handle_t *)intf_ptr;
    uint8_t buffer[64]; // BME68x transactions are small; 64 is plenty
    if (len > 63)
        return -1;

    buffer[0] = reg_addr;
    memcpy(&buffer[1], reg_data, len);
    esp_err_t ret = i2c_master_transmit(handle, buffer, len + 1, -1);
    return (ret == ESP_OK) ? 0 : -1;
}

void load_bsec_state(void)
{
    nvs_handle_t my_handle;
    uint8_t bsec_state[BSEC_MAX_STATE_BLOB_SIZE];
    uint8_t work_buffer[BSEC_MAX_STATE_BLOB_SIZE]; // <--- Added this
    size_t length = BSEC_MAX_STATE_BLOB_SIZE;

    if (nvs_open("storage", NVS_READONLY, &my_handle) == ESP_OK)
    {
        esp_err_t res = nvs_get_blob(my_handle, "bsec_state", bsec_state, &length);
        if (res == ESP_OK && length > 0)
        {
            // Updated call with all 4 required arguments
            bsec_library_return_t status = bsec_set_state(bsec_state, length, work_buffer, BSEC_MAX_STATE_BLOB_SIZE);

            if (status == BSEC_OK)
            {
                ESP_LOGI(TAG_AIR_QUALITY, "BSEC state loaded from NVS");
            }
            else
            {
                ESP_LOGE(TAG_AIR_QUALITY, "BSEC set_state failed: %d", status);
            }
        }
        nvs_close(my_handle);
    }
}

void save_bsec_state(void)
{
    nvs_handle_t my_handle;
    uint8_t bsec_state[BSEC_MAX_STATE_BLOB_SIZE];
    uint8_t work_buffer[BSEC_MAX_STATE_BLOB_SIZE];
    uint32_t length = 0;

    if (bsec_get_state(0, bsec_state, BSEC_MAX_STATE_BLOB_SIZE, work_buffer, BSEC_MAX_STATE_BLOB_SIZE, &length) == BSEC_OK)
    {
        if (nvs_open("storage", NVS_READWRITE, &my_handle) == ESP_OK)
        {
            nvs_set_blob(my_handle, "bsec_state", bsec_state, length);
            nvs_commit(my_handle);
            nvs_close(my_handle);
            ESP_LOGI(TAG_AIR_QUALITY, "BSEC state saved to NVS");
        }
    }
}

void bus_delay_us(uint32_t period, void *intf_ptr)
{
    esp_rom_delay_us(period);
}

const char *accuracy_str(uint8_t acc)
{
    switch (acc)
    {
    case 0:
        return "Stabilizing"; // First ~5 minutes after start
    case 1:
        return "Uncertain"; // Background history not reliable
    case 2:
        return "Calibrating"; // New calibration data detected
    case 3:
        return "Calibrated"; // Fully calibrated
    default:
        return "Unknown";
    }
}

void sim_bsec_task(void *pvParameters)
{
    ESP_LOGI(TAG_AIR_QUALITY, "Simulated BSEC Task Started");

    static float IAQ = 0.0f;
    static int accuracy = 6;
    static const char *acc_str = "Calibrated";
    static float eCO2 = 400.0f;
    static float bVOC = 0.5f;
    static float temp = 25.0f;
    static float hum = 50.0f;
    static float gas_res = 10000.0f;

    while (1)
    {
        IAQ = random_utils_generate(501);                            // Random IAQ between 0 and 500
        eCO2 = 400.0f + random_utils_generate(1601);                 // Random eCO2 between 400 and 2000
        bVOC = 0.5f + ((float)random_utils_generate(1000)) / 100.0f; // Random bVOC between 0.5 and 10.5
        temp = 15.0f + ((float)random_utils_generate(201));          // Random temp between 15.0 and 35.0
        hum = 20.0f + ((float)random_utils_generate(601));           // Random hum between 20.0 and 80.0
        gas_res = 5000.0f + ((float)random_utils_generate(15001));   // Random gas_res between 5000 and 20000

        ESP_LOGI(TAG_AIR_QUALITY, "Simulated IAQ: %.2f, Accuracy: %s", IAQ, acc_str);
        ESP_LOGI(TAG_AIR_QUALITY, "Simulated eCO2: %.2f ppm", eCO2);
        ESP_LOGI(TAG_AIR_QUALITY, "Simulated bVOC: %.2f mg/m3", bVOC);
        ESP_LOGI(TAG_AIR_QUALITY, "Simulated Temperature: %.2f °C", temp);
        ESP_LOGI(TAG_AIR_QUALITY, "Simulated Humidity: %.2f %%", hum);
        ESP_LOGI(TAG_AIR_QUALITY, "Simulated Gas Resistance: %.2f Ohms", gas_res);

        zb_update_iaq(IAQ, accuracy);
        zb_update_co2(eCO2);
        zb_update_bvoc(bVOC);
        zb_update_temp(temp);
        zb_update_hum(hum);
        zb_update_gas_resistance(gas_res);

        vTaskDelay(pdMS_TO_TICKS(10000)); // Simulate work every 5 seconds
    }
}

void bsec_task(void *pvParameters)
{
    ESP_LOGI(TAG_AIR_QUALITY, "BSEC Task Started");

    // 1. Initialize I2C Master Bus
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = SDA_GPIO,
        .scl_io_num = SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus_handle));

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BME68X_ADDR,
        .scl_speed_hz = 100000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));
    ESP_LOGI(TAG_AIR_QUALITY, "I2C Initialized");

    // 2. Setup BME68x Device
    bme_dev.read = bus_i2c_read;
    bme_dev.write = bus_i2c_write;
    bme_dev.delay_us = bus_delay_us;
    bme_dev.intf = BME68X_I2C_INTF;
    bme_dev.intf_ptr = &dev_handle;
    bme_dev.amb_temp = 25;

    int8_t rslt = bme68x_init(&bme_dev);
    if (rslt != BME68X_OK)
    {
        ESP_LOGE(TAG_AIR_QUALITY, "BME68x Init Failed: %d. Check Address or Wiring!", rslt);
        vTaskDelete(NULL);
    }
    ESP_LOGI(TAG_AIR_QUALITY, "BME68x Found");

    // 3. Initialize BSEC
    if (bsec_init() != BSEC_OK)
    {
        ESP_LOGE(TAG_AIR_QUALITY, "BSEC Init Failed");
        vTaskDelete(NULL);
    }
    ESP_LOGI(TAG_AIR_QUALITY, "BSEC Library Initialized");

    // 4. Subscribe to IAQ
    bsec_sensor_configuration_t requested_virtual_sensors[6];
    requested_virtual_sensors[0].sensor_id = BSEC_OUTPUT_IAQ;
    requested_virtual_sensors[0].sample_rate = BSEC_SAMPLE_RATE_ULP;

    requested_virtual_sensors[1].sensor_id = BSEC_OUTPUT_RAW_TEMPERATURE;
    requested_virtual_sensors[1].sample_rate = BSEC_SAMPLE_RATE_ULP;

    requested_virtual_sensors[2].sensor_id = BSEC_OUTPUT_RAW_HUMIDITY;
    requested_virtual_sensors[2].sample_rate = BSEC_SAMPLE_RATE_ULP;

    requested_virtual_sensors[3].sensor_id = BSEC_OUTPUT_RAW_GAS;
    requested_virtual_sensors[3].sample_rate = BSEC_SAMPLE_RATE_ULP;

    requested_virtual_sensors[4].sensor_id = BSEC_OUTPUT_CO2_EQUIVALENT;
    requested_virtual_sensors[4].sample_rate = BSEC_SAMPLE_RATE_ULP;

    requested_virtual_sensors[5].sensor_id = BSEC_OUTPUT_BREATH_VOC_EQUIVALENT;
    requested_virtual_sensors[5].sample_rate = BSEC_SAMPLE_RATE_ULP;

    bsec_sensor_configuration_t required_sensor_settings[BSEC_MAX_PHYSICAL_SENSOR];
    uint8_t n_required_sensor_settings = BSEC_MAX_PHYSICAL_SENSOR;

    bsec_update_subscription(requested_virtual_sensors, 6,
                             required_sensor_settings,
                             &n_required_sensor_settings);

    // 5. Main Loop
    bsec_bme_settings_t settings;
    struct bme68x_data data[3];
    uint8_t n_fields;

    ESP_ERROR_CHECK(nvs_flash_init()); // Ensure NVS is ready
    load_bsec_state();

    uint32_t state_save_counter = 0;

    while (1)
    {
        int64_t curr_time_ns = esp_timer_get_time() * 1000;
        bsec_sensor_control(curr_time_ns, &settings);

        if (settings.trigger_measurement)
        {
            // Configure Sensor
            struct bme68x_conf conf = {.os_hum = settings.humidity_oversampling,
                                       .os_pres = settings.pressure_oversampling,
                                       .os_temp = settings.temperature_oversampling,
                                       .filter = BME68X_FILTER_OFF,
                                       .odr = BME68X_ODR_NONE};
            bme68x_set_conf(&conf, &bme_dev);

            // Heater
            struct bme68x_heatr_conf h_conf = {.enable = BME68X_ENABLE,
                                               .heatr_temp = settings.heater_temperature,
                                               .heatr_dur = settings.heater_duration};
            bme68x_set_heatr_conf(BME68X_FORCED_MODE, &h_conf, &bme_dev);
            bme68x_set_op_mode(BME68X_FORCED_MODE, &bme_dev);

            // Wait for measurement
            uint32_t meas_dur = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &bme_dev);
            vTaskDelay(pdMS_TO_TICKS((meas_dur / 1000) + 1));

            if (bme68x_get_data(BME68X_FORCED_MODE, data, &n_fields, &bme_dev) == BME68X_OK && n_fields > 0)
            {
                bme_dev.amb_temp = data[0].temperature;

                int64_t curr_time_ns = esp_timer_get_time() * 1000;

                bsec_input_t inputs[4];

                inputs[0].time_stamp = curr_time_ns;
                inputs[0].signal = data[0].temperature;
                inputs[0].sensor_id = BSEC_INPUT_TEMPERATURE;

                inputs[1].time_stamp = curr_time_ns;
                inputs[1].signal = data[0].pressure;
                inputs[1].sensor_id = BSEC_INPUT_PRESSURE;

                inputs[2].time_stamp = curr_time_ns;
                inputs[2].signal = data[0].humidity;
                inputs[2].sensor_id = BSEC_INPUT_HUMIDITY;

                inputs[3].time_stamp = curr_time_ns;
                inputs[3].signal = data[0].gas_resistance;
                inputs[3].sensor_id = BSEC_INPUT_GASRESISTOR;

                bsec_output_t outputs[BSEC_NUMBER_OUTPUTS];
                uint8_t n_outputs = BSEC_NUMBER_OUTPUTS;
                bsec_do_steps(inputs, 4, outputs, &n_outputs);

                for (int i = 0; i < n_outputs; i++)
                {
                    if (outputs[i].sensor_id == BSEC_OUTPUT_IAQ && outputs[i].accuracy >= 3)
                    {
                        // Save every 10,000 samples (or whatever fits your sample rate)
                        if (state_save_counter++ > 10000)
                        {
                            save_bsec_state();
                            state_save_counter = 0;
                        }
                    }

                    switch (outputs[i].sensor_id)
                    {
                    case BSEC_OUTPUT_IAQ:
                        ESP_LOGI(TAG_AIR_QUALITY, "IAQ: %.2f", outputs[i].signal);
                        ESP_LOGI(TAG_AIR_QUALITY, "Accuracy: %s (%d)", accuracy_str(outputs[i].accuracy), outputs[i].accuracy);
                        zb_update_iaq(outputs[i].signal, outputs[i].accuracy);

                        break;
                    case BSEC_OUTPUT_CO2_EQUIVALENT:
                        ESP_LOGI(TAG_AIR_QUALITY, "eCO2: %.0f ppm", outputs[i].signal);
                        zb_update_co2(outputs[i].signal);

                        break;
                    case BSEC_OUTPUT_BREATH_VOC_EQUIVALENT:
                        ESP_LOGI(TAG_AIR_QUALITY, "bVOC: %.2f ppm", outputs[i].signal);
                        zb_update_bvoc(outputs[i].signal);

                        break;
                    case BSEC_OUTPUT_RAW_TEMPERATURE:
                        ESP_LOGI(TAG_AIR_QUALITY, "Temp: %.2f°C", outputs[i].signal);
                        zb_update_temp((int)(outputs[i].signal * 100));

                        break;
                    case BSEC_OUTPUT_RAW_HUMIDITY:
                        ESP_LOGI(TAG_AIR_QUALITY, "Hum: %.2f%%", outputs[i].signal);
                        zb_update_hum((uint16_t)(outputs[i].signal * 100));

                        break;
                    case BSEC_OUTPUT_RAW_GAS:
                        ESP_LOGI(TAG_AIR_QUALITY, "Gas Res: %.0f Ohm", outputs[i].signal);
                        zb_update_gas_resistance(outputs[i].signal);

                        break;
                    default:
                        break;
                    }
                }
            }
        }
        // Calculate sleep time based on BSEC's request
        int64_t sleep_time_ns = settings.next_call - (esp_timer_get_time() * 1000);
        if (sleep_time_ns > 0)
        {
            // Convert ns to ticks, ensure at least 1 tick
            vTaskDelay(pdMS_TO_TICKS(sleep_time_ns / 1000000));
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(10)); // Fallback short sleep
        }
    }
}
