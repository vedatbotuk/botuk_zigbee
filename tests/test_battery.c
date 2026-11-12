#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "battery_read.h"

static const int generated_values[] = {42, 3, 0, 0, 100, 3};
static const int expected_bounds[] = {101, 4, 101, 4, 101, 4};
static size_t random_call_count;
static int update_call_count;
static int report_call_count;
static int updated_level;
static int updated_voltage;

void vTaskDelay(TickType_t ticks)
{
    (void)ticks;
}

int random_utils_generate(int max_value)
{
    assert(random_call_count < sizeof(generated_values) / sizeof(generated_values[0]));
    assert(max_value == expected_bounds[random_call_count]);
    return generated_values[random_call_count++];
}

void zb_update_battery_level(int level, int voltage)
{
    update_call_count++;
    updated_level = level;
    updated_voltage = voltage;
}

void zb_report_battery_level(void)
{
    report_call_count++;
}

static void test_simulated_values_are_published_and_cached(void)
{
    const uint8_t expected_levels[] = {42, 0, 100};
    const uint8_t expected_voltages[] = {3, 0, 3};

    for (size_t index = 0; index < sizeof(expected_levels) / sizeof(expected_levels[0]); index++)
    {
        assert(read_battery_level() == ESP_OK);
        assert(get_battery_level() == expected_levels[index]);
        assert(update_call_count == (int)index + 1);
        assert(updated_level == 2 * expected_levels[index]);
        assert(updated_voltage == expected_voltages[index]);
        assert(report_call_count == (int)index + 1);
    }

    assert(random_call_count == sizeof(generated_values) / sizeof(generated_values[0]));
}

int main(void)
{
    test_simulated_values_are_published_and_cached();
    return 0;
}