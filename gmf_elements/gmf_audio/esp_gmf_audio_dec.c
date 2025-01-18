/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2025 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
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
#include "esp_gmf_err.h"
#include "esp_gmf_audio_dec.h"
#include "esp_audio_types.h"
#include "esp_audio_simple_dec_default.h"
#include "gmf_audio_common.h"

#define DEFAULT_DEC_OUTPUT_BUFFER_SIZE                     1024
#define AUDIO_DEC_CALC_PTS(out_len, sample_rate, ch, bits) (out_len) * 8000 / ((sample_rate) * (ch) * (bits))

/**
 * @brief Audio simple decoder context in GMF
 */
typedef struct {
    esp_gmf_audio_element_t        parent;   /*!< The GMF audio decoder handle */
    esp_audio_simple_dec_handle_t  dec_hd;   /*!< The audio simple decoder handle */
    esp_audio_simple_dec_raw_t     in_data;  /*!< The audio simple decoder input data handle */
    esp_audio_simple_dec_out_t     out_data; /*!< The audio simple decoder output data handle */
    int32_t                        buf_size; /*!< The size of decoder out buffer */
    esp_gmf_payload_t             *in_load;  /*!< The input payload */
    uint64_t                       pts;      /*!< Audio pts */
} esp_gmf_audio_dec_t;

static const char *TAG = "ESP_GMF_ASMP_DEC";

static inline esp_gmf_err_t dupl_esp_audio_simple_cfg(esp_audio_simple_dec_cfg_t *config, esp_audio_simple_dec_cfg_t **new_config)
{
    void *sub_cfg = NULL;
    *new_config = esp_gmf_oal_calloc(1, sizeof(*config));
    ESP_GMF_MEM_VERIFY(TAG, *new_config, {return ESP_GMF_ERR_MEMORY_LACK;}, "audio simple decoder configuration", sizeof(*config));
    memcpy(*new_config, config, sizeof(*config));
    if (config->dec_cfg && (config->cfg_size > 0)) {
        sub_cfg = esp_gmf_oal_calloc(1, config->cfg_size);
        ESP_GMF_MEM_VERIFY(TAG, sub_cfg, {esp_gmf_oal_free(*new_config); return ESP_GMF_ERR_MEMORY_LACK;},
                           "decoder configuration", config->cfg_size);
        memcpy(sub_cfg, config->dec_cfg, config->cfg_size);
        (*new_config)->dec_cfg = sub_cfg;
    }
    return ESP_GMF_JOB_ERR_OK;
}

static inline void free_esp_audio_simple_cfg(esp_audio_simple_dec_cfg_t *config)
{
    if (config) {
        if (config->dec_cfg) {
            esp_gmf_oal_free(config->dec_cfg);
            config->cfg_size = 0;
        }
        esp_gmf_oal_free(config);
    }
}

static esp_gmf_err_t esp_gmf_audio_dec_new(void *cfg, esp_gmf_obj_handle_t *handle)
{
    ESP_GMF_NULL_CHECK(TAG, cfg, {return ESP_GMF_ERR_INVALID_ARG;});
    ESP_GMF_NULL_CHECK(TAG, handle, {return ESP_GMF_ERR_INVALID_ARG;});
    *handle = NULL;
    esp_audio_simple_dec_cfg_t *dec_cfg = (esp_audio_simple_dec_cfg_t *)cfg;
    esp_gmf_obj_handle_t new_obj = NULL;
    esp_gmf_err_t ret = esp_gmf_audio_dec_init(dec_cfg, &new_obj);
    if (ret != ESP_GMF_ERR_OK) {
        return ret;
    }
    ret = esp_gmf_audio_dec_cast(dec_cfg, new_obj);
    if (ret != ESP_GMF_ERR_OK) {
        esp_gmf_obj_delete(new_obj);
        return ret;
    }
    *handle = (void *)new_obj;
    return ESP_GMF_ERR_OK;
}

static esp_gmf_job_err_t esp_gmf_audio_dec_open(esp_gmf_audio_element_handle_t self, void *para)
{
    ESP_GMF_NULL_CHECK(TAG, self, {return ESP_GMF_JOB_ERR_FAIL;});
    esp_gmf_audio_dec_t *audio_dec = (esp_gmf_audio_dec_t *)self;
    esp_audio_simple_dec_cfg_t *dec_cfg = (esp_audio_simple_dec_cfg_t *)OBJ_GET_CFG(self);
    if (dec_cfg == NULL) {
        ESP_LOGE(TAG, "There is no simple decoder configuration!");
        return ESP_GMF_JOB_ERR_FAIL;
    }
    esp_audio_simple_dec_open(dec_cfg, &audio_dec->dec_hd);
    ESP_GMF_CHECK(TAG, audio_dec->dec_hd, {return ESP_GMF_JOB_ERR_FAIL;}, "Failed to create simple decoder handle");
    esp_gmf_port_enable_payload_share(ESP_GMF_ELEMENT_GET(self)->in, false);
    audio_dec->buf_size = DEFAULT_DEC_OUTPUT_BUFFER_SIZE;
    ESP_LOGD(TAG, "Open, el: %p, cfg: %p, type: %d", self, dec_cfg, dec_cfg->dec_type);
    return ESP_GMF_JOB_ERR_OK;
}

static esp_gmf_job_err_t esp_gmf_audio_dec_process(esp_gmf_audio_element_handle_t self, void *para)
{
    esp_gmf_port_t *in_port = ESP_GMF_ELEMENT_GET(self)->in;
    esp_gmf_port_t *out = ESP_GMF_ELEMENT_GET(self)->out;
    esp_audio_err_t ret = ESP_AUDIO_ERR_OK;
    int out_len = -1;
    esp_gmf_err_io_t load_ret = ESP_GMF_IO_OK;
    esp_gmf_audio_dec_t *audio_dec = (esp_gmf_audio_dec_t *)self;
    esp_gmf_payload_t *out_load = NULL;
    esp_audio_simple_dec_info_t dec_info = {0};
    esp_gmf_info_sound_t snd_info = {0};
    esp_gmf_info_file_t info = {0};
    int in_read_num = GMF_AUDIO_INPUT_SAMPLE_NUM * 4;
    if (audio_dec->in_data.len == 0) {
        load_ret = esp_gmf_port_acquire_in(in_port, &audio_dec->in_load, in_read_num, in_port->wait_ticks);
        ESP_GMF_PORT_ACQUIRE_IN_CHECK(TAG, load_ret, out_len, {goto __aud_proc_release;});
        audio_dec->in_data.buffer = audio_dec->in_load->buf;
        audio_dec->in_data.len = audio_dec->in_load->valid_size;
        audio_dec->in_data.consumed = 0;
        audio_dec->in_data.eos = audio_dec->in_load->is_done;
    }
    ESP_LOGV(TAG, "Read, in_len: %ld, done: %d\r\n", audio_dec->in_data.len, audio_dec->in_load ? audio_dec->in_load->is_done : -1);
    load_ret = esp_gmf_port_acquire_out(out, &out_load, audio_dec->buf_size, ESP_GMF_MAX_DELAY);
    ESP_GMF_PORT_ACQUIRE_OUT_CHECK(TAG, load_ret, out_len, {goto __aud_proc_release;});
    out_load->valid_size = 0;
    audio_dec->out_data.buffer = out_load->buf;
    audio_dec->out_data.len = out_load->buf_length;
    while (1) {
        ret = esp_audio_simple_dec_process(audio_dec->dec_hd, &audio_dec->in_data, &audio_dec->out_data);
        if (ret != ESP_AUDIO_ERR_OK && ret != ESP_AUDIO_ERR_BUFF_NOT_ENOUGH) {
            ESP_LOGE(TAG, "Failed to decode data, ret: %d", ret);
            out_len = ESP_GMF_JOB_ERR_FAIL;
            goto __aud_proc_release;
        }
        if (audio_dec->in_data.consumed <= audio_dec->in_data.len) {
            audio_dec->in_data.buffer += audio_dec->in_data.consumed;
            audio_dec->in_data.len -= audio_dec->in_data.consumed;
        }
        if (ret == ESP_AUDIO_ERR_BUFF_NOT_ENOUGH) {
            load_ret = esp_gmf_port_release_out(out, out_load, out->wait_ticks);
            ESP_GMF_PORT_RELEASE_OUT_CHECK(TAG, load_ret, out_len, {goto __aud_proc_release;});
            load_ret = esp_gmf_port_acquire_out(out, &out_load, audio_dec->out_data.needed_size, ESP_GMF_MAX_DELAY);
            ESP_GMF_PORT_ACQUIRE_OUT_CHECK(TAG, load_ret, out_len, {goto __aud_proc_release;});
            ESP_LOGW(TAG, "Not enough memory for out, need:%d, old: %ld, new: %d", (int)audio_dec->out_data.needed_size,
                     audio_dec->out_data.len, out_load->buf_length);
            audio_dec->out_data.buffer = out_load->buf;
            audio_dec->out_data.len = out_load->buf_length;
            audio_dec->buf_size = audio_dec->out_data.needed_size;
            continue;
        }
        ESP_LOGV(TAG, "Dec, out len: %ld, need: %ld, in len: %ld, consumed: %ld, dec: %ld",
                 audio_dec->out_data.len, audio_dec->out_data.needed_size,
                 audio_dec->in_data.len, audio_dec->in_data.consumed, audio_dec->out_data.decoded_size);
        ESP_LOGV(TAG, "buf: %p, sz: %d, dec: %ld", out_load->buf, out_load->valid_size, audio_dec->out_data.decoded_size);
        if (audio_dec->out_data.decoded_size > 0) {
            esp_audio_simple_dec_get_info(audio_dec->dec_hd, &dec_info);
            esp_gmf_audio_el_get_snd_info(self, &snd_info);
            if (snd_info.sample_rates != dec_info.sample_rate
                || snd_info.channels != dec_info.channel
                || snd_info.bits != dec_info.bits_per_sample) {
                ESP_LOGI(TAG, "NOTIFY Info, rate: %d, bits: %d, ch: %d --> rate: %ld, bits: %d, ch: %d",
                         snd_info.sample_rates, snd_info.bits, snd_info.channels, dec_info.sample_rate, dec_info.bits_per_sample, dec_info.channel);
                GMF_AUDIO_UPDATE_SND_INFO(self, dec_info.sample_rate, dec_info.bits_per_sample, dec_info.channel);
            }
            out_load->valid_size = audio_dec->out_data.decoded_size;
            out_load->is_done = audio_dec->in_load->is_done;
            out_len = out_load->valid_size;
            audio_dec->pts += AUDIO_DEC_CALC_PTS(out_len, dec_info.sample_rate, dec_info.channel, dec_info.bits_per_sample);
            out_load->pts = audio_dec->pts;
            if (out_load->valid_size > 0) {
                esp_gmf_audio_el_update_file_pos(self, out_load->valid_size);
            }
            load_ret = esp_gmf_port_release_out(out, out_load, out->wait_ticks);
            ESP_GMF_PORT_RELEASE_OUT_CHECK(TAG, load_ret, out_len, {goto __aud_proc_release;});
            out_load = NULL;
        } else {
            // Need to store all the inbuf data to simple decoder
            if (audio_dec->in_data.len > 0) {
                continue;
            }
            // Reacquire the input data, until have decoded frame
            if (audio_dec->in_load->is_done) {
                // This case audio decoder is done, need to return
                out_load->is_done = audio_dec->in_load->is_done;
                esp_gmf_audio_el_get_file_info(self, &info);
                ESP_LOGD(TAG, "Total: %lld bytes(%d)", info.pos, __LINE__);
                out_len = ESP_GMF_JOB_ERR_DONE;
                goto __aud_proc_release;
            }
            load_ret = esp_gmf_port_release_in(in_port, audio_dec->in_load, ESP_GMF_MAX_DELAY);
            ESP_GMF_PORT_RELEASE_IN_CHECK(TAG, load_ret, out_len, {goto __aud_proc_release;});
            audio_dec->in_load = NULL;
            load_ret = esp_gmf_port_acquire_in(in_port, &audio_dec->in_load, in_read_num, in_port->wait_ticks);
            ESP_GMF_PORT_ACQUIRE_IN_CHECK(TAG, load_ret, out_len, {goto __aud_proc_release;});
            audio_dec->in_data.buffer = audio_dec->in_load->buf;
            audio_dec->in_data.len = audio_dec->in_load->valid_size;
            audio_dec->in_data.consumed = 0;
            audio_dec->in_data.eos = audio_dec->in_load->is_done;
            ESP_LOGD(TAG, "Read More, ret: %d, decoded_size: %ld, in_len: %ld", ret, audio_dec->out_data.decoded_size, audio_dec->in_data.len);
            continue;
        }
        ESP_LOGV(TAG, "Release IN, in_len: %ld, done: %d, decoded_size: %ld",
                 audio_dec->in_data.len, audio_dec->in_load ? audio_dec->in_load->is_done : -1, audio_dec->out_data.decoded_size);
        if (audio_dec->in_data.len == 0) {
            ESP_LOGD(TAG, "Release In port %p, ret: %d", in_port, ret);
            if (audio_dec->in_load->is_done) {
                esp_gmf_audio_el_get_file_info(self, &info);
                ESP_LOGD(TAG, "Total: %lld bytes(%d)", info.pos, __LINE__);
                out_len = ESP_GMF_JOB_ERR_DONE;
            }
            goto __aud_proc_release;
        }
        break;
    }
    return out_len;
__aud_proc_release:
    if (audio_dec->in_load != NULL) {
        load_ret = esp_gmf_port_release_in(in_port, audio_dec->in_load, ESP_GMF_MAX_DELAY);
        ESP_GMF_PORT_RELEASE_IN_CHECK(TAG, load_ret, out_len, NULL);
    }
    if (out_load != NULL) {
        load_ret = esp_gmf_port_release_out(out, out_load, out->wait_ticks);
        ESP_GMF_PORT_RELEASE_OUT_CHECK(TAG, load_ret, out_len, NULL);
    }
    return out_len;
}

static esp_gmf_job_err_t esp_gmf_audio_dec_close(esp_gmf_audio_element_handle_t self, void *para)
{
    ESP_GMF_NULL_CHECK(TAG, self, {return ESP_GMF_ERR_OK;});
    ESP_LOGD(TAG, "Closed, %p", self);
    esp_gmf_audio_dec_t *audio_dec = (esp_gmf_audio_dec_t *)self;
    if (audio_dec->dec_hd != NULL) {
        esp_audio_simple_dec_close(audio_dec->dec_hd);
        audio_dec->dec_hd = NULL;
    }
    audio_dec->pts = 0;
    esp_gmf_info_sound_t snd_info = {0};
    audio_dec->in_load = NULL;
    audio_dec->in_data.len = 0;
    esp_gmf_audio_el_set_snd_info(self, &snd_info);
    return ESP_GMF_ERR_OK;
}

static esp_gmf_err_t esp_gmf_audio_dec_destroy(esp_gmf_audio_element_handle_t self)
{
    ESP_GMF_NULL_CHECK(TAG, self, {return ESP_GMF_ERR_INVALID_ARG;});
    ESP_LOGD(TAG, "Destroyed, %p", self);
    free_esp_audio_simple_cfg(OBJ_GET_CFG(self));
    esp_gmf_audio_el_deinit(self);
    esp_gmf_oal_free(self);
    return ESP_GMF_ERR_OK;
}

esp_gmf_err_t esp_gmf_audio_dec_init(esp_audio_simple_dec_cfg_t *config, esp_gmf_obj_handle_t *handle)
{
    ESP_GMF_NULL_CHECK(TAG, config, {return ESP_GMF_ERR_INVALID_ARG;});
    ESP_GMF_NULL_CHECK(TAG, handle, {return ESP_GMF_ERR_INVALID_ARG;});
    *handle = NULL;
    esp_gmf_err_t ret = ESP_GMF_ERR_OK;
    esp_gmf_audio_dec_t *dec_hd = esp_gmf_oal_calloc(1, sizeof(esp_gmf_audio_dec_t));
    ESP_GMF_MEM_VERIFY(TAG, dec_hd, {return ESP_GMF_ERR_MEMORY_LACK;}, "audio decoder", sizeof(esp_gmf_audio_dec_t));
    esp_gmf_obj_t *obj = (esp_gmf_obj_t *)dec_hd;
    obj->new = esp_gmf_audio_dec_new;
    obj->delete = esp_gmf_audio_dec_destroy;
    esp_audio_simple_dec_cfg_t *cfg = NULL;
    dupl_esp_audio_simple_cfg(config, &cfg);
    ESP_GMF_CHECK(TAG, cfg, {ret = ESP_GMF_ERR_MEMORY_LACK; goto ES_DEC_FAIL;}, "Failed to allocate audio decoder configuration");
    esp_gmf_obj_set_config(obj, cfg, sizeof(*config));
    ret = esp_gmf_obj_set_tag(obj, "aud_simp_dec");
    ESP_GMF_RET_ON_NOT_OK(TAG, ret, goto ES_DEC_FAIL, "Failed to set obj tag");
    *handle = obj;
    esp_gmf_element_cfg_t el_cfg = {0};
    ESP_GMF_ELEMENT_CFG(el_cfg, false, ESP_GMF_EL_PORT_CAP_SINGLE, ESP_GMF_EL_PORT_CAP_SINGLE,
                        ESP_GMF_PORT_TYPE_BLOCK | ESP_GMF_PORT_TYPE_BYTE, ESP_GMF_PORT_TYPE_BYTE | ESP_GMF_PORT_TYPE_BLOCK);
    ret = esp_gmf_audio_el_init(dec_hd, &el_cfg);
    ESP_GMF_RET_ON_NOT_OK(TAG, ret, goto ES_DEC_FAIL, "Failed to initialize audio decoder element");
    ESP_LOGD(TAG, "Initialization, %s-%p", OBJ_GET_TAG(obj), obj);
    return ESP_GMF_ERR_OK;
ES_DEC_FAIL:
    esp_gmf_obj_delete(obj);
    return ret;
}

esp_gmf_err_t esp_gmf_audio_dec_cast(esp_audio_simple_dec_cfg_t *config, esp_gmf_obj_handle_t handle)
{
    ESP_GMF_NULL_CHECK(TAG, config, {return ESP_GMF_ERR_INVALID_ARG;});
    ESP_GMF_NULL_CHECK(TAG, handle, {return ESP_GMF_ERR_INVALID_ARG;});
    esp_audio_simple_dec_cfg_t *cfg = NULL;
    dupl_esp_audio_simple_cfg(config, &cfg);
    ESP_GMF_CHECK(TAG, cfg, {return ESP_GMF_ERR_MEMORY_LACK;}, "Failed to duplicate audio config");
    // Free memory before overwriting
    free_esp_audio_simple_cfg(OBJ_GET_CFG(handle));
    esp_gmf_obj_set_config(handle, cfg, sizeof(*config));
    esp_gmf_audio_element_t *audio_dec_el = (esp_gmf_audio_element_t *)handle;
    audio_dec_el->base.ops.open = esp_gmf_audio_dec_open;
    audio_dec_el->base.ops.process = esp_gmf_audio_dec_process;
    audio_dec_el->base.ops.close = esp_gmf_audio_dec_close;
    return ESP_GMF_ERR_OK;
}
