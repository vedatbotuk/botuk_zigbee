#ifndef AIR_QUALITY_H
#define AIR_QUALITY_H

#include <stdint.h>
#include "bme68x.h"
#include "bsec_interface.h"
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SDA_GPIO            GPIO_NUM_3
#define SCL_GPIO            GPIO_NUM_4
#define BME68X_ADDR         0x77

extern i2c_master_dev_handle_t dev_handle;
extern struct bme68x_dev bme_dev;
extern bool connection_status(void);

bool is_connected(void);

/**
 * @brief I2C read callback for BME68x
 */
int8_t bus_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);

/**
 * @brief I2C write callback for BME68x
 */
int8_t bus_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);

/**
 * @brief Delay callback for BME68x (microseconds)
 */
void bus_delay_us(uint32_t period, void *intf_ptr);

/**
 * @brief Load BSEC state from NVS
 */
void load_bsec_state(void);

/**
 * @brief Save BSEC state to NVS
 */
void save_bsec_state(void);

/**
 * @brief Convert accuracy code to string
 */
const char* accuracy_str(uint8_t acc);

/**
 * @brief Simulate BSEC task for testing without hardware
 */
void sim_bsec_task(void *pvParameters);

/**
 * @brief BSEC main FreeRTOS task
 */
void bsec_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif // AIR_QUALITY_H