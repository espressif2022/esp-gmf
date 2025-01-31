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

#pragma once

#include "esp_gmf_data_bus.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/**
 * @brief  GMF Pointer Buffer is an interface for passing buffer addresses without generating any copies.
 *         esp_gmf_pbuf_acquire_read is used to retrieve a filled buffer. esp_gmf_pbuf_acquire_write indicates the
 *         address of an available buffer. If no available buffer exists, a request will also be made. The
 *         maximum number of requests is specified during initialization. The esp_gmf_pbuf_acquire_read/write and
 *         esp_gmf_pbuf_release_read/writer must be used in pairs and cannot be invoked recursively.
 *
 *         Its interfaces do not possess blocking conditions, but they can be called safely in a multithreaded environment.
 */

/**
 * @brief  Represents a handle to a pointer buffer
 */
typedef void *esp_gmf_pbuf_handle_t;

/**
 * @brief  Creates a new pointer buffer with the specified capacity
 *         It simply allocates an instance without any buffer
 *
 * @param[in]   capacity  The capacity of the maximum pointer buffer
 * @param[out]  handle    Pointer to store the handle to the created pointer buffer
 *
 * @return
 *       - ESP_GMF_ERR_OK           Operation successful
 *       - ESP_GMF_ERR_INVALID_ARG  Invalid argument provided
 *       - ESP_GMF_ERR_MEMORY_LACK  Memory allocation failed
 */
esp_gmf_err_t esp_gmf_pbuf_create(int capacity, esp_gmf_pbuf_handle_t *handle);

/**
 * @brief  Destroy a pointer buffer and release all associated resources
 *
 * @param[in]  handle  Handle to the pointer buffer to be destroyed
 *
 * @return
 *       - ESP_GMF_ERR_OK           Operation successful
 *       - ESP_GMF_ERR_INVALID_ARG  Invalid argument provided
 */
esp_gmf_err_t esp_gmf_pbuf_destroy(esp_gmf_pbuf_handle_t handle);

/**
 * @brief  Acquire valid data from the pointer buffer for reading, it consistently retrieves the filled header data
 *
 * @note  1. If called before any write operation, it will return ESP_GMF_IO_FAIL
 *        2. `esp_gmf_pbuf_acquire_read` and `esp_gmf_pbuf_release_read` must be called in pairs, and consecutive acquisition
 *            followed by consecutive release is not allowed
 *
 * @param[in]   handle       Handle to the pointer buffer
 * @param[out]  blk          Pointer to the data bus block structure to store the acquired data
 * @param[in]   wanted_size  Desired size of data to be acquired
 * @param[in]   block_ticks  Ticks to wait for the operation
 *
 * @return
 *       - > 0              The specific length of data being read
 *       - ESP_GMF_IO_OK    Operation succeeded
 *       - ESP_GMF_IO_FAIL  Invalid arguments, or no filled data
 */
esp_gmf_err_io_t esp_gmf_pbuf_acquire_read(esp_gmf_pbuf_handle_t handle, esp_gmf_data_bus_block_t *blk, uint32_t wanted_size, int block_ticks);

/**
 * @brief  Returned acquired buffer to the specific pointer buffer
 *
 * @note  1. The returned buffer MUST be acquired from `esp_gmf_pbuf_acquire_read` to ensure proper synchronization
 *        2. `esp_gmf_pbuf_acquire_read` and `esp_gmf_pbuf_release_read` must be called in pairs, and consecutive acquisition
 *           followed by consecutive release is not allowed
 *
 * @param[in]  handle       Handle to the pointer buffer
 * @param[in]  blk          Pointer to the data bus block structure containing the data to be released
 * @param[in]  block_ticks  Ticks to wait for the operation
 *
 * @return
 *       - ESP_GMF_IO_OK    Operation succeeded
 *       - ESP_GMF_IO_FAIL  Invalid arguments, or the buffer does not belong to the provided handle
 */
esp_gmf_err_io_t esp_gmf_pbuf_release_read(esp_gmf_pbuf_handle_t handle, esp_gmf_data_bus_block_t *blk, int block_ticks);

/**
 * @brief  Acquire space corresponding to the desired size within a specific handle
 *         The first call requests the desired buffer, or it does so when no buffer is available.
 *
 * @note  1. The allocated number of buffer is greater than given capacity, it will return ESP_GMF_IO_FAIL
 *        2. `esp_gmf_pbuf_acquire_write` and `esp_gmf_pbuf_release_write` must be called in pairs, and consecutive acquisition
 *            followed by consecutive release is not allowed
 *
 * @param[in]   handle       Handle to the pointer buffer
 * @param[out]  blk          Pointer to the data bus block structure to store the acquired space
 * @param[in]   wanted_size  Desired size of space to be acquired
 * @param[in]   block_ticks  Ticks to wait for the operation
 *
 * @return
 *       - > 0              The specific length of space can be write
 *       - ESP_GMF_IO_OK    Operation succeeded, or it's done to write
 *       - ESP_GMF_IO_FAIL  Invalid arguments, or no filled data
 */
esp_gmf_err_io_t esp_gmf_pbuf_acquire_write(esp_gmf_pbuf_handle_t handle, esp_gmf_data_bus_block_t *blk, uint32_t wanted_size, int block_ticks);

/**
 * @brief  Store the given buffer to filled list by the specific pointer buffer
 *
 * @note  1. The stored buffer MUST be acquired from `esp_gmf_pbuf_acquire_write` to ensure proper synchronization
 *        2. `esp_gmf_pbuf_acquire_write` and `esp_gmf_pbuf_release_write` must be called in pairs, and consecutive acquisition
 *            followed by consecutive release is not allowed
 *
 * @param[in]  handle       Handle to the pointer buffer
 * @param[in]  blk          Pointer to the data bus block structure containing the space to be released
 * @param[in]  block_ticks  Ticks to wait for the operation
 *
 * @return
 *       - ESP_GMF_IO_OK    Operation succeeded
 *       - ESP_GMF_IO_FAIL  Invalid arguments, or the buffer does not belong to the provided handle
 */
esp_gmf_err_io_t esp_gmf_pbuf_release_write(esp_gmf_pbuf_handle_t handle, esp_gmf_data_bus_block_t *blk, int block_ticks);

/**
 * @brief  Set the status of writing to the pointer buffer as done
 *
 * @param[in]  handle  Handle to the pointer buffer
 *
 * @return
 *       - ESP_GMF_ERR_OK           Operation successful
 *       - ESP_GMF_ERR_INVALID_ARG  Invalid argument provided
 */
esp_gmf_err_t esp_gmf_pbuf_done_write(esp_gmf_pbuf_handle_t handle);

/**
 * @brief  Abort waiting until there is space for reading or writing of the pointer buffer
 *
 * @param[in]  handle  Handle to the pointer buffer
 *
 * @return
 *       - ESP_GMF_ERR_OK           Operation successful
 *       - ESP_GMF_ERR_INVALID_ARG  Invalid argument provided
 */
esp_gmf_err_t esp_gmf_pbuf_abort(esp_gmf_pbuf_handle_t handle);

/**
 * @brief  Reset the pointer buffer, clearing all values to initial state
 *
 * @param[in]  handle  Handle to the pointer buffer
 *
 * @return
 *       - ESP_GMF_ERR_OK           Operation successful
 *       - ESP_GMF_ERR_INVALID_ARG  Invalid argument provided
 */
esp_gmf_err_t esp_gmf_pbuf_reset(esp_gmf_pbuf_handle_t handle);

/**
 * @brief  Get the total free size of the pointer buffer
 *
 * @param[in]   handle     Handle to the pointer buffer
 * @param[out]  free_size  Pointer to store the total free size
 *
 * @return
 *       - ESP_GMF_ERR_OK           Operation successful
 *       - ESP_GMF_ERR_INVALID_ARG  Invalid argument provided
 */
esp_gmf_err_t esp_gmf_pbuf_get_free_size(esp_gmf_pbuf_handle_t handle, uint32_t *free_size);

/**
 * @brief  Get the number of bytes that have filled the pointer buffer
 *
 * @param[in]   handle       Handle to the pointer buffer
 * @param[out]  filled_size  Pointer to store the filled size
 *
 * @return
 *       - ESP_GMF_ERR_OK           Operation successful
 *       - ESP_GMF_ERR_INVALID_ARG  Invalid argument provided
 */
esp_gmf_err_t esp_gmf_pbuf_get_filled_size(esp_gmf_pbuf_handle_t handle, uint32_t *filled_size);

/**
 * @brief  Get the total size of the pointer buffer (in bytes)
 *
 * @param[in]   handle      Handle to the pointer buffer
 * @param[out]  total_size  Pointer to store the total size
 *
 * @return
 *       - ESP_GMF_ERR_OK           Operation successful
 *       - ESP_GMF_ERR_INVALID_ARG  Invalid argument provided
 */
esp_gmf_err_t esp_gmf_pbuf_get_total_size(esp_gmf_pbuf_handle_t handle, uint32_t *total_size);

#ifdef __cplusplus
}
#endif  /* __cplusplus */
