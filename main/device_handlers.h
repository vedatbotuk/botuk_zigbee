#ifndef DEVICE_HANDLERS_H
#define DEVICE_HANDLERS_H

#include "esp_zigbee_core.h"

#ifdef __cplusplus
extern "C"
{
#endif

esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message);

#ifdef __cplusplus
}
#endif

#endif
