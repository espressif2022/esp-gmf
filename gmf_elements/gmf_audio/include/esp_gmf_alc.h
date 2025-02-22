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

#pragma once

#include "esp_gmf_err.h"
#include "esp_ae_alc.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define DEFAULT_ESP_GMF_ALC_CONFIG() {  \
    .sample_rate     = 48000,           \
    .bits_per_sample = 16,              \
    .channel         = 2,               \
}

/**
 * @brief  Initializes the GMF ALC with the provided configuration
 *
 * @param[in]   config  Pointer to the ALC configuration
 * @param[out]  handle  Pointer to the ALC handle to be initialized
 *
 * @return
 *       - ESP_GMF_ERR_OK           Success
 *       - ESP_GMF_ERR_INVALID_ARG  Invalid configuration provided
 *       - ESP_GMF_ERR_MEMORY_LACK  Failed to allocate memory
 */
esp_gmf_err_t esp_gmf_alc_init(esp_ae_alc_cfg_t *config, esp_gmf_obj_handle_t *handle);

/**
 * @brief  Cast to GMF ALC using ALC configuration
 *
 * @param[in]   config  Pointer to the ALC configuration
 * @param[out]  handle  ALC handle to be casted
 *
 * @return
 *       - ESP_GMF_ERR_OK           Success
 *       - ESP_GMF_ERR_INVALID_ARG  Invalid configuration provided
 *       - ESP_GMF_ERR_MEMORY_LACK  Failed to allocate memory
 */
esp_gmf_err_t esp_gmf_alc_cast(esp_ae_alc_cfg_t *config, esp_gmf_obj_handle_t handle);

/**
 * @brief  Set specific channel gain of the ALC handle.
 *         Positive gain indicates an increase in volume,
 *         negative gain indicates a decrease in volume.
 *         0 gain indicates the volume level remains unchanged.
 *
 * @param[in]  handle  The ALC handle
 * @param[in]  idx     The channel index of the gain to retrieve. eg: 0 refers to the first channel
 * @param[in]  gain    The gain value needs to conform to the following conditions:
 *                     - Supported range [-64, 63]
 *                     - Below -64 will set to mute
 *                     - Higher than 63 not supported
 *                     Unit: dB
 *
 * @return
 *       - ESP_GMF_ERR_OK           Operation succeeded
 *       - ESP_GMF_ERR_INVALID_ARG  Invalid input parameter
 */
esp_gmf_err_t esp_gmf_alc_set_gain(esp_gmf_audio_element_handle_t handle, uint8_t idx, int8_t gain);

/**
 * @brief  Get the gain for a specific channel from the ALC handle
 *
 * @param[in]   handle  The ALC handle
 * @param[in]   ch_idx  The channel index of the gain to retrieve. eg: 0 refers to the first channel
 * @param[out]  gain    Pointer to store the retrieved gain. Unit: dB
 *
 * @return
 *       - ESP_GMF_ERR_OK           Operation succeeded
 *       - ESP_GMF_ERR_INVALID_ARG  Invalid input parameter
 */
esp_gmf_err_t esp_gmf_alc_get_gain(esp_gmf_audio_element_handle_t handle, uint8_t idx, int8_t *gain);

#ifdef __cplusplus
}
#endif /* __cplusplus */
