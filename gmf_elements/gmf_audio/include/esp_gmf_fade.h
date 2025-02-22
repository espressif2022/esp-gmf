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
#include "esp_ae_fade.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define DEFAULT_ESP_GMF_FADE_CONFIG() {           \
    .sample_rate     = 48000,                     \
    .bits_per_sample = 16,                        \
    .channel         = 2,                         \
    .transit_time    = 500,                       \
    .mode            = ESP_AE_FADE_MODE_FADE_IN,  \
    .curve           = ESP_AE_FADE_CURVE_LINE,    \
}

/**
 * @brief  Initializes the GMF fade with the provided configuration
 *
 * @param[in]   config  Pointer to the fade configuration
 * @param[out]  handle  Pointer to the fade handle to be initialized
 *
 * @return
 *       - ESP_GMF_ERR_OK           Success
 *       - ESP_GMF_ERR_INVALID_ARG  Invalid configuration provided
 *       - ESP_GMF_ERR_MEMORY_LACK  Failed to allocate memory
 */
esp_gmf_err_t esp_gmf_fade_init(esp_ae_fade_cfg_t *config, esp_gmf_obj_handle_t *handle);

/**
 * @brief  Initializes the GMF fade with the provided configuration
 *
 * @param[in]   config  Pointer to the fade configuration
 * @param[out]  handle  Pointer to the fade handle to be initialized
 *
 * @return
 *       - ESP_GMF_ERR_OK           Success
 *       - ESP_GMF_ERR_INVALID_ARG  Invalid configuration provided
 *       - ESP_GMF_ERR_MEMORY_LACK  Failed to allocate memory
 */
esp_gmf_err_t esp_gmf_fade_cast(esp_ae_fade_cfg_t *config, esp_gmf_obj_handle_t handle);

/**
 * @brief  Set the fade process mode
 *
 * @param[in]  handle  The fade handle
 * @param[in]  mode    The mode of fade
 *
 * @return
 *       - ESP_GMF_ERR_OK           Operation succeeded
 *       - ESP_GMF_ERR_INVALID_ARG  Invalid input parameter
 */
esp_gmf_err_t esp_gmf_fade_set_mode(esp_gmf_audio_element_handle_t handle, esp_ae_fade_mode_t mode);

/**
 * @brief  Get the fade process mode
 *
 * @param[in]   handle  The fade handle
 * @param[out]  mode    The mode of fade
 *
 * @return
 *       - ESP_GMF_ERR_OK           Operation succeeded
 *       - ESP_GMF_ERR_INVALID_ARG  Invalid input parameter
 */
esp_gmf_err_t esp_gmf_fade_get_mode(esp_gmf_audio_element_handle_t handle, esp_ae_fade_mode_t *mode);

/**
 * @brief  Reset the fade process to the initial configuration state.
 *         If the initial configuration mode is fade in, the current weight is set to 0.
 *         If the initial configuration mode is fade out, the current weight is set to 1.
 *         This function helps the user to restart fade in or fade out without
 *         reopening and closing.
 *
 * @param[in]  handle  The fade handle
 *
 * @return
 *       - ESP_GMF_ERR_OK           Operation succeeded
 *       - ESP_GMF_ERR_INVALID_ARG  Invalid input parameter
 */
esp_gmf_err_t esp_gmf_fade_reset(esp_gmf_audio_element_handle_t handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */
