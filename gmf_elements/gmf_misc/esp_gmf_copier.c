/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2025 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD.>
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

#include <string.h>
#include "esp_log.h"
#include "esp_gmf_oal_mem.h"
#include "esp_gmf_element.h"
#include "esp_gmf_err.h"
#include "esp_gmf_copier.h"

/**
 * @brief Copier context in GMF
 */
typedef struct esp_gmf_copier {
    struct esp_gmf_element parent;   /*!< The GMF copier handle */
    uint8_t                copy_num; /*!< The output stream number */
} esp_gmf_copier_t;

static const char *TAG = "ESP_GMF_COPIER";

static esp_gmf_err_t esp_gmf_copier_new(void *cfg, esp_gmf_obj_handle_t *handle)
{
    ESP_GMF_NULL_CHECK(TAG, cfg, {return ESP_GMF_ERR_INVALID_ARG;});
    ESP_GMF_NULL_CHECK(TAG, handle, {return ESP_GMF_ERR_INVALID_ARG;});
    *handle = NULL;
    esp_gmf_copier_cfg_t *copier_cfg = (esp_gmf_copier_cfg_t *)cfg;
    esp_gmf_obj_handle_t new_obj = NULL;
    esp_gmf_err_t ret = esp_gmf_copier_init(copier_cfg, &new_obj);
    if (ret != ESP_GMF_ERR_OK) {
        return ret;
    }
    ret = esp_gmf_copier_cast(copier_cfg, new_obj);
    if (ret != ESP_GMF_ERR_OK) {
        esp_gmf_obj_delete(new_obj);
        return ret;
    }
    *handle = (void *)new_obj;
    return ESP_GMF_ERR_OK;
}

static esp_gmf_job_err_t esp_gmf_copier_open(esp_gmf_element_handle_t self, void *para)
{
    ESP_LOGD(TAG, "%s", __func__);
    return ESP_GMF_ERR_OK;
}

static esp_gmf_job_err_t esp_gmf_copier_process(esp_gmf_element_handle_t self, void *para)
{
    esp_gmf_element_handle_t hd = (esp_gmf_element_handle_t)self;
    esp_gmf_port_t *in = ESP_GMF_ELEMENT_GET(hd)->in;
    esp_gmf_port_t *out = ESP_GMF_ELEMENT_GET(hd)->out;
    esp_gmf_payload_t *in_load = NULL;
    esp_gmf_payload_t *out_load = NULL;
    int out_len = -1;
    int idx = 0;
    esp_gmf_err_io_t ret = esp_gmf_port_acquire_in(in, &in_load, in->user_buf_len, ESP_GMF_MAX_DELAY);
    ESP_GMF_PORT_ACQUIRE_IN_CHECK(TAG, ret, out_len, {goto __copy_release;});
    while (out) {
        out_load = NULL;
        if (out != ESP_GMF_ELEMENT_GET(hd)->out) {
            ret = esp_gmf_port_acquire_out(out, &out_load, in_load->buf_length, 0);
            ESP_GMF_PORT_ACQUIRE_OUT_CHECK(TAG, ret, out_len, {goto __copy_release;});
            esp_gmf_payload_copy_data(in_load, out_load);
        } else {
            out_load = in_load;
            ret = esp_gmf_port_acquire_out(out, &out_load, in_load->buf_length, 0);
            ESP_GMF_PORT_ACQUIRE_OUT_CHECK(TAG, ret, out_len, {goto __copy_release;});
        }
        ret = esp_gmf_port_release_out(out, out_load, out->wait_ticks);
        if (ret < ESP_GMF_IO_OK) {
            if (ret == ESP_GMF_IO_FAIL) {
                ESP_LOGE(TAG, "Failed to release out, idx: %d, ret: %d.", idx, ret);
                out_len = ret;
                goto __copy_release;
            } else {
                ESP_LOGW(TAG, "Something error on release out, idx: %d, ret: %d.", idx, ret);
            }
        }
        ESP_LOGV(TAG, "OUT: %p, rd: %s, I: %p, b: %p, sz: %d, O: %p, b: %p, sz: %d, done: %d, t: %d", out, OBJ_GET_TAG(out->reader),
                 in_load, in_load->buf, in_load->valid_size, out_load,
                 out_load != NULL ? out_load->buf : NULL,
                 out_load != NULL ? out_load->valid_size : -1,
                 out_load != NULL ? out_load->is_done : -1,
                 out->wait_ticks);
        out = out->next;
        idx++;
    }
    out_len = in_load->valid_size;
    if (in_load->is_done) {
        out_len = ESP_GMF_JOB_ERR_DONE;
    }
__copy_release:
    if (in_load != NULL) {
        ret = esp_gmf_port_release_in(in, in_load, 0);
        ESP_GMF_PORT_RELEASE_IN_CHECK(TAG, ret, out_len, NULL);
    }
    return out_len;
}

static esp_gmf_job_err_t esp_gmf_copier_close(esp_gmf_element_handle_t self, void *para)
{
    ESP_LOGD(TAG, "%s", __func__);
    return ESP_GMF_ERR_OK;
}

static esp_gmf_err_t esp_gmf_copier_destroy(esp_gmf_element_handle_t self)
{
    if (self != NULL) {
        ESP_LOGD(TAG, "Destroyed");
        esp_gmf_element_deinit(self);
        esp_gmf_oal_free(self);
    }
    return ESP_GMF_ERR_OK;
}

esp_gmf_err_t esp_gmf_copier_init(esp_gmf_copier_cfg_t *config, esp_gmf_obj_handle_t *handle)
{
    ESP_GMF_NULL_CHECK(TAG, config, {return ESP_GMF_ERR_INVALID_ARG;});
    ESP_GMF_NULL_CHECK(TAG, handle, {return ESP_GMF_ERR_INVALID_ARG;});
    *handle = NULL;
    esp_gmf_copier_t *copier = esp_gmf_oal_calloc(1, sizeof(esp_gmf_copier_t));
    ESP_GMF_MEM_VERIFY(TAG, copier, {return ESP_GMF_ERR_MEMORY_LACK;},
                       "copier", sizeof(esp_gmf_copier_t));
    copier->copy_num = config->copy_num;
    esp_gmf_obj_t *obj = (esp_gmf_obj_t *)copier;
    obj->new_obj = esp_gmf_copier_new;
    obj->del_obj = esp_gmf_copier_destroy;
    esp_gmf_err_t ret = esp_gmf_obj_set_config(obj, config, sizeof(*config));
    ESP_GMF_RET_ON_NOT_OK(TAG, ret, goto COPIER_FAIL, "Failed set OBJ configuration");
    ret = esp_gmf_obj_set_tag(obj, "copier");
    ESP_GMF_RET_ON_NOT_OK(TAG, ret, goto COPIER_FAIL, "Failed set OBJ tag");
    *handle = obj;
    return ESP_GMF_ERR_OK;
COPIER_FAIL:
    esp_gmf_obj_delete(obj);
    return ret;
}

esp_gmf_err_t esp_gmf_copier_cast(esp_gmf_copier_cfg_t *config, esp_gmf_obj_handle_t handle)
{
    ESP_GMF_NULL_CHECK(TAG, config, {return ESP_GMF_ERR_INVALID_ARG;});
    ESP_GMF_NULL_CHECK(TAG, handle, {return ESP_GMF_ERR_INVALID_ARG;});
    esp_gmf_element_handle_t copier = (esp_gmf_element_handle_t)handle;
    ESP_GMF_ELEMENT_GET(copier)->ops.open = esp_gmf_copier_open;
    ESP_GMF_ELEMENT_GET(copier)->ops.process = esp_gmf_copier_process;
    ESP_GMF_ELEMENT_GET(copier)->ops.close = esp_gmf_copier_close;
    esp_gmf_element_cfg_t el_cfg = {0};
    ESP_GMF_ELEMENT_CFG(el_cfg, false, ESP_GMF_EL_PORT_CAP_SINGLE, ESP_GMF_EL_PORT_CAP_MULTI,
                        ESP_GMF_PORT_TYPE_BLOCK | ESP_GMF_PORT_TYPE_BYTE, ESP_GMF_PORT_TYPE_BYTE | ESP_GMF_PORT_TYPE_BLOCK);
    return esp_gmf_element_init(copier, &el_cfg);
}
