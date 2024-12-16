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

#include "ota.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_check.h"
#include <string.h>
#include "zlib.h"
#include "esp_ota_ops.h"

static const char *TAG_OTA = "OTA_UPDATE";

static const esp_partition_t *s_ota_partition = NULL;
static esp_ota_handle_t s_ota_handle = 0;
static size_t ota_data_len_ = 0;
static uint8_t *ota_header_ = NULL;
static size_t ota_header_size_ = 0;
static bool ota_upgrade_subelement_ = false;
static bool ota_initialized = false;

typedef struct
{
    const esp_partition_t *part; // Pointer to the OTA partition
    esp_ota_handle_t handle;     // Handle to manage OTA writing
    z_stream zlib_stream;        // Zlib stream for decompression
    bool zlib_init;              // Flag for zlib initialization
} CompressedOTA;

static size_t min_size_t(size_t a, size_t b)
{
    return (a < b) ? a : b;
}

static void clear_ota_header()
{
    free(ota_header_);
    ota_header_size_ = 0;
    ota_header_ = NULL;
}

static bool CompressedOTA_start(CompressedOTA *ctx)
{
    if (ctx->zlib_init)
    {
        ESP_LOGW(TAG_OTA, "zlib already initialized, ending it");
        inflateEnd(&ctx->zlib_stream);
        ctx->zlib_init = false;
    }

    memset(&ctx->zlib_stream, 0, sizeof(ctx->zlib_stream));
    ctx->zlib_stream.zalloc = Z_NULL;
    ctx->zlib_stream.zfree = Z_NULL;
    ctx->zlib_stream.opaque = Z_NULL;
    ctx->zlib_stream.next_in = NULL;
    ctx->zlib_stream.avail_in = 0;

    int ret = inflateInit(&ctx->zlib_stream);
    if (ret == Z_OK)
    {
        ESP_LOGI(TAG_OTA, "zlib initialized");
        ctx->zlib_init = true;
        return true;
    }
    else
    {
        ESP_LOGE(TAG_OTA, "zlib init failed: %d", ret);
        return false;
    }
}

static bool CompressedOTA_write(CompressedOTA *ctx, const uint8_t *data, size_t size, bool flush)
{
    uint8_t buf[256];

    if (ctx->part == NULL)
    {
        ESP_LOGE(TAG_OTA, "OTA not started, partition not initialized");
        return false;
    }

    ctx->zlib_stream.avail_in = size;
    ctx->zlib_stream.next_in = (Bytef *)data;

    do
    {
        ctx->zlib_stream.avail_out = sizeof(buf);
        ctx->zlib_stream.next_out = buf;

        int ret = inflate(&ctx->zlib_stream, flush ? Z_FINISH : Z_NO_FLUSH);
        if (ret == Z_STREAM_ERROR || ret == Z_NEED_DICT || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR)
        {
            ESP_LOGE(TAG_OTA, "zlib error: %d", ret);
            esp_ota_abort(ctx->handle);
            ctx->part = NULL;
            return false;
        }

        size_t available = sizeof(buf) - ctx->zlib_stream.avail_out;

        if (available > 0)
        {
            esp_err_t err = esp_ota_write(ctx->handle, buf, available);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG_OTA, "Error writing OTA: %d", err);
                esp_ota_abort(ctx->handle);
                ctx->part = NULL;
                return false;
            }
        }
    } while (ctx->zlib_stream.avail_in > 0 || ctx->zlib_stream.avail_out == 0);

    return true;
}

esp_err_t zb_ota_upgrade_status_handler(esp_zb_zcl_ota_upgrade_value_message_t message)
{
    static uint32_t total_size = 0;
    static uint32_t offset = 0;
    static int64_t start_time = 0;
    const uint8_t *payload = (const uint8_t *)message.payload;
    size_t payload_size = message.payload_size;
    static CompressedOTA ota_ctx;

    esp_err_t ret = ESP_OK;
    if (message.info.status == ESP_ZB_ZCL_STATUS_SUCCESS)
    {
        switch (message.upgrade_status)
        {
        case ESP_ZB_ZCL_OTA_UPGRADE_STATUS_START:
            ESP_LOGI(TAG_OTA, "-- OTA upgrade start");
            start_time = esp_timer_get_time();
            s_ota_partition = esp_ota_get_next_update_partition(NULL);
            assert(s_ota_partition);
            ret = esp_ota_begin(s_ota_partition, 0, &s_ota_handle);
            clear_ota_header();
            ota_upgrade_subelement_ = false;
            ota_data_len_ = 0;
            ESP_RETURN_ON_ERROR(ret, TAG_OTA, "Failed to begin OTA partition, status: %s", esp_err_to_name(ret));

            if (!ota_initialized)
            {
                memset(&ota_ctx, 0, sizeof(CompressedOTA));
                ota_ctx.part = s_ota_partition;
                ota_ctx.handle = s_ota_handle;

                if (CompressedOTA_start(&ota_ctx))
                {
                    ESP_LOGI(TAG_OTA, "OTA process started successfully.");
                    ota_initialized = true;
                }
                else
                {
                    ESP_LOGE(TAG_OTA, "Failed to start OTA process.");
                    return ESP_FAIL;
                }
            }
            break;
        case ESP_ZB_ZCL_OTA_UPGRADE_STATUS_RECEIVE:
            total_size = message.ota_header.image_size;
            offset += message.payload_size;

            ESP_LOGI(TAG_OTA, "OTA [%ld/%ld]", offset, total_size);

            while (ota_header_size_ < 6 && payload_size > 0)
            {
                ota_header_ = realloc(ota_header_, (ota_header_size_ + 1) * sizeof(uint8_t));
                ota_header_[ota_header_size_] = payload[0];
                payload++;
                payload_size--;
                ota_header_size_++;
            }
            if (!ota_upgrade_subelement_ && ota_header_size_ == 6)
            {
                if (ota_header_[0] == 0 && ota_header_[1] == 0)
                {
                    ota_upgrade_subelement_ = true;
                    ota_data_len_ = (((int)ota_header_[5] & 0xFF) << 24) | (((int)ota_header_[4] & 0xFF) << 16) | (((int)ota_header_[3] & 0xFF) << 8) | ((int)ota_header_[2] & 0xFF);
                    ESP_LOGI(TAG_OTA, "in if OTA sub-element size %zu", ota_data_len_);
                }
                else
                {
                    ESP_LOGE(TAG_OTA, "OTA sub-element not success.");
                }
            }

            if (ota_data_len_)
            {
                payload_size = min_size_t(ota_data_len_, payload_size);
                ota_data_len_ -= payload_size;

                if (CompressedOTA_write(&ota_ctx, payload, payload_size, false))
                {
                    ESP_LOGI(TAG_OTA, "Data chunk written successfully.");
                }
                else
                {
                    ESP_LOGE(TAG_OTA, "Failed to write data chunk.");
                }
            }
            break;
        case ESP_ZB_ZCL_OTA_UPGRADE_STATUS_APPLY:
            ESP_LOGI(TAG_OTA, "-- OTA upgrade apply");
            break;
        case ESP_ZB_ZCL_OTA_UPGRADE_STATUS_CHECK:
            ret = offset == total_size ? ESP_OK : ESP_FAIL;
            ESP_LOGI(TAG_OTA, "-- OTA upgrade check status: %s", esp_err_to_name(ret));
            break;
        case ESP_ZB_ZCL_OTA_UPGRADE_STATUS_FINISH:
            ESP_LOGI(TAG_OTA, "-- OTA Finish");
            ESP_LOGI(TAG_OTA,
                     "-- OTA Information: version: 0x%lx, manufactor code: 0x%x, image type: 0x%x, total size: %ld bytes, cost time: %lld ms,",
                     message.ota_header.file_version, message.ota_header.manufacturer_code, message.ota_header.image_type,
                     message.ota_header.image_size, (esp_timer_get_time() - start_time) / 1000);
            ret = esp_ota_end(s_ota_handle);
            ESP_RETURN_ON_ERROR(ret, TAG_OTA, "Failed to end OTA partition, status: %s", esp_err_to_name(ret));
            ret = esp_ota_set_boot_partition(s_ota_partition);
            ESP_RETURN_ON_ERROR(ret, TAG_OTA, "Failed to set OTA boot partition, status: %s", esp_err_to_name(ret));
            ESP_LOGW(TAG_OTA, "Prepare to restart system");
            esp_restart();
            break;
        default:
            ESP_LOGI(TAG_OTA, "OTA status: %d", message.upgrade_status);
            break;
        }
    }
    return ret;
}
