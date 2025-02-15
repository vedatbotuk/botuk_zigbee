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

#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#ifdef __cplusplus
extern "C"
{
#endif

    extern bool conn;
    extern const char *TAG_SIGNAL_HANDLER;
    extern uint8_t deepsleep_cnt;

    // void sensor_map_status();
    bool connection_status();
    void create_signal_handler(esp_zb_app_signal_t signal_struct);

#ifdef __cplusplus
}
#endif

#endif /* SIGNAL_HANDLER_H */
