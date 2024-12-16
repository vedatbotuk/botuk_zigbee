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

#include <stdlib.h>
#include "random_utils.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_random.h"

int random_utils_generate(int max_value)
{
  // Gather more entropy from multiple sources
  uint32_t seed = esp_random() ^ esp_timer_get_time();
  seed ^= (esp_random() << 16); // Mix additional hardware RNG bits
  srand(seed);                  // Initialize the random number generator

  // Generate a random number within the range [0, max_value)
  return rand() % max_value;
}
