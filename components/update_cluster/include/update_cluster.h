/*
 * selforganized_802.15.4_network_with_esp32
 * Copyright (c) 2024 Vedat Botuk.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef ZB_TEMP_SENSOR_H
#define ZB_TEMP_SENSOR_H

#include <stdbool.h>

extern bool connection_status(void);
bool is_connected(void);


void zb_update_temp(int16_t temperature);
void zb_report_temp();
void zb_update_hum(int humidity);
void zb_report_hum();
void zb_update_pressure(int16_t pressure);
void zb_report_pressure();
void zb_update_battery_level(int level, int voltage);
void zb_report_battery_level();
void zb_update_waterleak(uint16_t leak);
void zb_report_waterleak(uint16_t leak);
void zb_update_current_time(uint32_t current_time);
void zb_update_local_time(uint32_t local_time);
void zb_update_iaq(float iaq, uint8_t accuracy);
void zb_update_iaq_accuracy(uint8_t accuracy);
void zb_report_iaq();
void zb_report_iaq_accuracy();
void zb_update_co2(float eco2);
void zb_report_co2();
void zb_update_bvoc(float bvoc);
void zb_report_bvoc();
void zb_update_gas_resistance(float gas_resistance);
void zb_report_gas_resistance();
void zb_update_builtin_light_flash_red(uint8_t flash_status);
void zb_update_builtin_light_flash_yellow(uint8_t flash_status);
void zb_update_builtin_light_flash_green(uint8_t flash_status);
void zb_update_builtin_light_flash_white(uint8_t flash_status);
void zb_report_builtin_light_flash_red();
void zb_report_builtin_light_flash_yellow();
void zb_report_builtin_light_flash_green();
void zb_report_builtin_light_flash_white();

#endif // ZB_TEMP_SENSOR_H
