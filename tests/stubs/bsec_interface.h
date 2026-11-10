#ifndef TEST_BSEC_INTERFACE_H
#define TEST_BSEC_INTERFACE_H

#include <stdint.h>

#define BSEC_OK 0
#define BSEC_NUMBER_OUTPUTS 10
#define BSEC_MAX_PHYSICAL_SENSOR 8

#define BSEC_OUTPUT_IAQ 1
#define BSEC_OUTPUT_RAW_TEMPERATURE 2
#define BSEC_OUTPUT_RAW_PRESSURE 3
#define BSEC_OUTPUT_RAW_HUMIDITY 4
#define BSEC_OUTPUT_RAW_GAS 5
#define BSEC_OUTPUT_CO2_EQUIVALENT 6
#define BSEC_OUTPUT_BREATH_VOC_EQUIVALENT 7

#define BSEC_SAMPLE_RATE_LP 3
#define BSEC_SAMPLE_RATE_ULP 4

#define BSEC_INPUT_TEMPERATURE 10
#define BSEC_INPUT_PRESSURE 11
#define BSEC_INPUT_HUMIDITY 12
#define BSEC_INPUT_GASRESISTOR 13

typedef struct {
    uint8_t sensor_id;
    float sample_rate;
} bsec_sensor_configuration_t;

typedef struct {
    uint8_t trigger_measurement;
    uint8_t pressure_oversampling;
    uint8_t humidity_oversampling;
    uint8_t temperature_oversampling;
    int64_t next_call;
    uint8_t heater_temperature;
    uint8_t heater_duration;
} bsec_bme_settings_t;

typedef struct {
    int64_t time_stamp;
    float signal;
    uint8_t sensor_id;
} bsec_input_t;

typedef struct {
    int64_t time_stamp;
    float signal;
    uint8_t sensor_id;
    uint8_t accuracy;
} bsec_output_t;

int8_t bsec_init(void);
int8_t bsec_update_subscription(const bsec_sensor_configuration_t *requested_virtual_sensors,
                               uint8_t n_requested_virtual_sensors,
                               bsec_sensor_configuration_t *required_sensor_settings,
                               uint8_t *n_required_sensor_settings);
void bsec_sensor_control(int64_t curr_time_ns, bsec_bme_settings_t *settings);
void bsec_do_steps(const bsec_input_t *inputs, uint8_t n_inputs, bsec_output_t *outputs, uint8_t *n_outputs);

#endif
