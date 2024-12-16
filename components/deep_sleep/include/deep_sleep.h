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

#ifndef DEEP_SLEEP_H
#define DEEP_SLEEP_H

#ifdef __cplusplus
extern "C"
{
#endif

  static const int before_deep_sleep_time_sec = 5;
  void start_deep_sleep(void);
  void zb_deep_sleep_init(void);

#ifdef __cplusplus
}
#endif

#endif /* DEEP_SLEEP_H */
