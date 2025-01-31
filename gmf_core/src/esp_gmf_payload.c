/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2024 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "stdlib.h"
#include "esp_gmf_oal_mem.h"
#include "esp_gmf_payload.h"
#include "esp_log.h"
#include "string.h"

static const char *TAG = "ESP_GMF_PAYLOAD";

esp_gmf_err_t esp_gmf_payload_new(esp_gmf_payload_t **instance)
{
    if (instance == NULL) {
        ESP_LOGE(TAG, "Invalid parameters on %s, h:%p", __func__, instance);
        return ESP_GMF_ERR_INVALID_ARG;
    }
    *instance = (esp_gmf_payload_t *)esp_gmf_oal_calloc(1, sizeof(esp_gmf_payload_t));
    ESP_LOGD(TAG, "New a payload, h:%p, called:0x%08x", *instance, (intptr_t)__builtin_return_address(0) - 2);
    return ESP_GMF_ERR_OK;
}

esp_gmf_err_t esp_gmf_payload_new_with_len(uint32_t buf_length, esp_gmf_payload_t **instance)
{
    if ((instance == NULL) || (buf_length == 0)) {
        ESP_LOGE(TAG, "Invalid parameters on %s, h:%p, l:%ld", __func__, instance, buf_length);
        return ESP_GMF_ERR_INVALID_ARG;
    }
    *instance = (esp_gmf_payload_t *)esp_gmf_oal_calloc(1, sizeof(esp_gmf_payload_t));
    ESP_GMF_NULL_CHECK(TAG, *instance, return ESP_GMF_ERR_MEMORY_LACK);
    (*instance)->buf = esp_gmf_oal_calloc(1, buf_length);
    ESP_GMF_NULL_CHECK(TAG, (*instance)->buf, {
        esp_gmf_oal_free(*instance);
        return ESP_GMF_ERR_MEMORY_LACK;
    });
    (*instance)->buf_length = buf_length;
    (*instance)->needs_free = 1;
    ESP_LOGD(TAG, "New a payload with buffer, h:%p, buf:%p, l:%ld, called:0x%08x", *instance, (*instance)->buf, buf_length, (intptr_t)__builtin_return_address(0) - 2);
    return ESP_GMF_ERR_OK;
}

esp_gmf_err_t esp_gmf_payload_copy_data(esp_gmf_payload_t *src, esp_gmf_payload_t *dest)
{
    if ((src == NULL) || (dest == 0) || (src->buf == NULL) || (dest->buf == 0)) {
        ESP_LOGE(TAG, "Invalid parameters, src:%p-buf:%p, dest:%p-buf:%p", src, src->buf, dest, dest == NULL ? NULL : dest->buf);
        return ESP_GMF_ERR_INVALID_ARG;
    }
    if (src->valid_size > dest->buf_length) {
        ESP_LOGE(TAG, "The valid size large than target buffer length, src:%p-l:%d, dest:%p-l:%d",
                 src, src->valid_size, dest, dest->buf_length);
        return ESP_GMF_ERR_OUT_OF_RANGE;
    }
    memcpy(dest->buf, src->buf, src->valid_size);
    dest->valid_size = src->valid_size;
    dest->is_done = src->is_done;
    return ESP_GMF_ERR_OK;
}

esp_gmf_err_t esp_gmf_payload_realloc_aligned_buf(esp_gmf_payload_t *instance, uint8_t align, uint32_t new_length)
{
    if ((instance == NULL) || (new_length == 0)) {
        ESP_LOGE(TAG, "Invalid parameters on %s, h:%p, l:%ld", __func__, instance, new_length);
        return ESP_GMF_ERR_INVALID_ARG;
    }
    if (instance->buf_length < new_length) {
        uint8_t *buf = instance->buf;
        uint32_t len = instance->buf_length;
        if (instance->buf) {
            ESP_LOGD(TAG, "Free payload:%p, buf:%p-%d, needs_free:%d", instance, instance->buf, instance->buf_length, instance->needs_free);
            instance->buf_length = 0;
            esp_gmf_oal_free(instance->buf);
            instance->buf = NULL;
        } else {
            instance->needs_free = 1;
        }
        instance->buf = esp_gmf_oal_malloc_align(align, new_length);
        ESP_GMF_NULL_CHECK(TAG, instance->buf, {instance->needs_free = 0; return ESP_GMF_ERR_MEMORY_LACK;});
        instance->buf_length = new_length;
        // Do not set instance->needs_free, `buf` may allocate by others.
        ESP_LOGD(TAG, "Realloc payload, h:%p, buf:%p-%ld, new_buf:%p-%ld, called:0x%08x", instance, buf, len, instance->buf, new_length, (intptr_t)__builtin_return_address(0) - 2);
    }
    return ESP_GMF_ERR_OK;
}

esp_gmf_err_t esp_gmf_payload_realloc_buf(esp_gmf_payload_t *instance, uint32_t new_length)
{
    return esp_gmf_payload_realloc_aligned_buf(instance, 0, new_length);
}

esp_gmf_err_t esp_gmf_payload_set_done(esp_gmf_payload_t *instance)
{
    ESP_GMF_NULL_CHECK(TAG, instance, return ESP_GMF_ERR_INVALID_ARG);
    instance->is_done = true;
    return ESP_GMF_ERR_OK;
}

esp_gmf_err_t esp_gmf_payload_clean_done(esp_gmf_payload_t *instance)
{
    ESP_GMF_NULL_CHECK(TAG, instance, return ESP_GMF_ERR_INVALID_ARG);
    instance->is_done = false;
    return ESP_GMF_ERR_OK;
}

void esp_gmf_payload_delete(esp_gmf_payload_t *instance)
{
    ESP_LOGD(TAG, "Delete a payload, h:%p, needs_free:%d, buf:%p, l:%d", instance, instance != NULL ? instance->needs_free : -1,
             instance != NULL ? instance->buf : NULL, instance != NULL ? instance->buf_length : -1);
    if (instance) {
        if (instance->needs_free) {
            esp_gmf_oal_free(instance->buf);
            instance->needs_free = 0;
        }
        instance->buf = NULL;
        instance->buf_length = 0;
        esp_gmf_oal_free(instance);
    }
}
