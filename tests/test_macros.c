#include "macros.h"

#if EXPECTED_CASE == 0
#define EXPECT_IRRIGATION 0
#define EXPECT_SWITCH 0
#define EXPECT_BUILTIN_LIGHT 0
#define EXPECT_WATERLEAK 0
#define EXPECT_DEEP_SLEEP 0
#define EXPECT_LIGHT_SLEEP 0
#define EXPECT_BATTERY 0
#define EXPECT_BME680 0
#define EXPECT_DHT22 0
#define EXPECT_OTA 0
#define EXPECT_END_DEVICE 0
#define EXPECT_ROUTER 1
#elif EXPECTED_CASE == 1
#define EXPECT_IRRIGATION 1
#define EXPECT_SWITCH 1
#define EXPECT_BUILTIN_LIGHT 0
#define EXPECT_WATERLEAK 1
#define EXPECT_DEEP_SLEEP 0
#define EXPECT_LIGHT_SLEEP 0
#define EXPECT_BATTERY 0
#define EXPECT_BME680 0
#define EXPECT_DHT22 0
#define EXPECT_OTA 0
#define EXPECT_END_DEVICE 1
#define EXPECT_ROUTER 0
#elif EXPECTED_CASE == 2
#define EXPECT_IRRIGATION 0
#define EXPECT_SWITCH 0
#define EXPECT_BUILTIN_LIGHT 0
#define EXPECT_WATERLEAK 0
#define EXPECT_DEEP_SLEEP 0
#define EXPECT_LIGHT_SLEEP 0
#define EXPECT_BATTERY 0
#define EXPECT_BME680 0
#define EXPECT_DHT22 0
#define EXPECT_OTA 1
#define EXPECT_END_DEVICE 1
#define EXPECT_ROUTER 0
#else
#error "Unknown EXPECTED_CASE"
#endif

#ifdef IRRIGATION_FEATURES
#define ACTUAL_IRRIGATION 1
#else
#define ACTUAL_IRRIGATION 0
#endif
#ifdef SWITCH_FEATURES
#define ACTUAL_SWITCH 1
#else
#define ACTUAL_SWITCH 0
#endif
#ifdef BUILTIN_LIGHT
#define ACTUAL_BUILTIN_LIGHT 1
#else
#define ACTUAL_BUILTIN_LIGHT 0
#endif
#ifdef WATERLEAK_FEATURES
#define ACTUAL_WATERLEAK 1
#else
#define ACTUAL_WATERLEAK 0
#endif
#ifdef DEEP_SLEEP
#define ACTUAL_DEEP_SLEEP 1
#else
#define ACTUAL_DEEP_SLEEP 0
#endif
#ifdef LIGHT_SLEEP
#define ACTUAL_LIGHT_SLEEP 1
#else
#define ACTUAL_LIGHT_SLEEP 0
#endif
#ifdef BATTERY_FEATURES
#define ACTUAL_BATTERY 1
#else
#define ACTUAL_BATTERY 0
#endif
#ifdef BME680
#define ACTUAL_BME680 1
#else
#define ACTUAL_BME680 0
#endif
#ifdef DHT22
#define ACTUAL_DHT22 1
#else
#define ACTUAL_DHT22 0
#endif
#ifdef OTA_UPDATE
#define ACTUAL_OTA 1
#else
#define ACTUAL_OTA 0
#endif
#ifdef END_DEVICE
#define ACTUAL_END_DEVICE 1
#else
#define ACTUAL_END_DEVICE 0
#endif
#ifdef ROUTER_DEVICE
#define ACTUAL_ROUTER 1
#else
#define ACTUAL_ROUTER 0
#endif

_Static_assert(ACTUAL_IRRIGATION == EXPECT_IRRIGATION, "irrigation feature mismatch");
_Static_assert(ACTUAL_SWITCH == EXPECT_SWITCH, "switch feature mismatch");
_Static_assert(ACTUAL_BUILTIN_LIGHT == EXPECT_BUILTIN_LIGHT, "built-in light feature mismatch");
_Static_assert(ACTUAL_WATERLEAK == EXPECT_WATERLEAK, "waterleak feature mismatch");
_Static_assert(ACTUAL_DEEP_SLEEP == EXPECT_DEEP_SLEEP, "deep sleep feature mismatch");
_Static_assert(ACTUAL_LIGHT_SLEEP == EXPECT_LIGHT_SLEEP, "light sleep feature mismatch");
_Static_assert(ACTUAL_BATTERY == EXPECT_BATTERY, "battery feature mismatch");
_Static_assert(ACTUAL_BME680 == EXPECT_BME680, "BME680 feature mismatch");
_Static_assert(ACTUAL_DHT22 == EXPECT_DHT22, "DHT22 feature mismatch");
_Static_assert(ACTUAL_OTA == EXPECT_OTA, "OTA feature mismatch");
_Static_assert(ACTUAL_END_DEVICE == EXPECT_END_DEVICE, "end-device role mismatch");
_Static_assert(ACTUAL_ROUTER == EXPECT_ROUTER, "router role mismatch");

int main(void)
{
    return 0;
}