#include <assert.h>
#include <stdint.h>

#include "esp_random.h"
#include "esp_timer.h"
#include "random_utils.h"

static uint32_t random_value = 0x12345678;
static int64_t timer_value = 987654321;

uint32_t esp_random(void)
{
    return random_value;
}

int64_t esp_timer_get_time(void)
{
    return timer_value;
}

static void test_returns_value_within_requested_range(void)
{
    const int bounds[] = {1, 2, 4, 101, 1601, 0x7fffffff};

    for (unsigned int index = 0; index < sizeof(bounds) / sizeof(bounds[0]); index++)
    {
        for (int attempt = 0; attempt < 100; attempt++)
        {
            random_value += 0x9e3779b9;
            timer_value += 7919;
            int result = random_utils_generate(bounds[index]);
            assert(result >= 0);
            assert(result < bounds[index]);
        }
    }
}

static void test_rejects_non_positive_bounds(void)
{
    assert(random_utils_generate(0) == -1);
    assert(random_utils_generate(-1) == -1);
}

static void test_single_item_range_returns_zero(void)
{
    random_value = 0x12345678;
    timer_value = 987654321;

    for (int attempt = 0; attempt < 25; attempt++)
    {
        assert(random_utils_generate(1) == 0);
    }
}

static void test_supports_large_positive_bounds(void)
{
    const int bounds[] = {2, 3, 5, 16, 128, 1024, 32768, 2147483647};

    for (unsigned int index = 0; index < sizeof(bounds) / sizeof(bounds[0]); index++)
    {
        for (int attempt = 0; attempt < 25; attempt++)
        {
            random_value += 0x9e3779b9;
            timer_value += 7919;

            int result = random_utils_generate(bounds[index]);
            assert(result >= 0);
            assert(result < bounds[index]);
        }
    }
}

int main(void)
{
    test_returns_value_within_requested_range();
    test_rejects_non_positive_bounds();
    test_single_item_range_returns_zero();
    test_supports_large_positive_bounds();
    return 0;
}