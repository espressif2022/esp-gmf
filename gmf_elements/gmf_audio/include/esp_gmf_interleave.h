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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define DEFAULT_ESP_GMF_INTERLEAVE_CONFIG() {  \
    .sample_rate     = 48000,                  \
    .bits_per_sample = 16,                     \
    .src_num         = 2,                      \
}

/**
 * @brief  Configuration structure for interleave
 */
typedef struct {
    uint32_t sample_rate;     /*!< The audio sample rate */
    uint8_t  bits_per_sample; /*!< The audio bits per sample, supports 16, 24, 32 bits */
    uint8_t  src_num;         /*!< The number of input source num */
} esp_gmf_interleave_cfg;

/**
 * @brief  Initializes the GMF interleave with the provided configuration
 *
 * @param[in]   config  Pointer to the interleave configuration
 * @param[out]  handle  Pointer to the interleave handle to be initialized
 *
 * @return
 *       - ESP_GMF_ERR_OK           Success
 *       - ESP_GMF_ERR_INVALID_ARG  Invalid configuration provided
 *       - ESP_GMF_ERR_MEMORY_LACK  Failed to allocate memory
 */
esp_gmf_err_t esp_gmf_interleave_init(esp_gmf_interleave_cfg *config, esp_gmf_obj_handle_t *handle);

/**
 * @brief  Initializes the GMF interleave with the provided configuration
 *
 * @param[in]   config  Pointer to the interleave configuration
 * @param[out]  handle  Pointer to the interleave handle to be initialized
 *
 * @return
 *       - ESP_GMF_ERR_OK           Success
 *       - ESP_GMF_ERR_INVALID_ARG  Invalid configuration provided
 *       - ESP_GMF_ERR_MEMORY_LACK  Failed to allocate memory
 */
esp_gmf_err_t esp_gmf_interleave_cast(esp_gmf_interleave_cfg *config, esp_gmf_obj_handle_t handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */
