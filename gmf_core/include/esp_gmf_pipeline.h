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

#include "esp_gmf_element.h"
#include "esp_gmf_io.h"
#include "esp_gmf_task.h"
#include "esp_gmf_event.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define ESP_GMF_PIPELINE_GET_IN_INSTANCE(p)   (p->in)
#define ESP_GMF_PIPELINE_GET_OUT_INSTANCE(p)  (p->out)
#define ESP_GMF_PIPELINE_GET_FIRST_ELEMENT(p) (p->head_el)
#define ESP_GMF_PIPELINE_GET_LAST_ELEMENT(p)  (p->last_el)

/**
 * @brief  Pointer to a GMF pipeline
 */
typedef struct esp_gmf_pipeline *esp_gmf_pipeline_handle_t;

/**
 * @brief  Function pointer type for pipeline previous stop callback
 */
typedef esp_gmf_err_t (*esp_gmf_pipeline_prev_stop)(void *handle);  /*!<  */

/**
 * @brief  Structure representing a pipeline in GMF
 */
typedef struct esp_gmf_pipeline {
    esp_gmf_element_handle_t    head_el;        /*!< Handle of the first element in the pipeline */
    esp_gmf_element_handle_t    last_el;        /*!< Handle of the last element in the pipeline */
    esp_gmf_io_handle_t         in;             /*!< Handle of the input I/O port */
    esp_gmf_io_handle_t         out;            /*!< Handle of the output I/O port */

    esp_gmf_event_item_t       *evt_conveyor;   /*!< Event conveyor list */
    esp_gmf_event_cb            evt_acceptor;   /*!< Event acceptor callback function */
    esp_gmf_event_cb            user_cb;        /*!< User callback function */
    void                       *user_ctx;       /*!< User context */
    esp_gmf_event_state_t       state;          /*!< Current state of the pipeline */
    esp_gmf_task_handle_t       thread;         /*!< Handle of the task associated with the pipeline */
    esp_gmf_pipeline_prev_stop  prev_stop;      /*!< A pointer to the previous stop callback */
    void                       *prev_stop_ctx;  /*!< The previous stop callback context */
    void                       *lock;           /*!< Lock for thread synchronization */
} esp_gmf_pipeline_t;

/**
 * @brief  The GMF pipeline configuration structure
 */
typedef struct esp_gmf_pipeline_cfg {
    esp_gmf_event_cb  event;  /*!< Event callback function */
} esp_gmf_pipeline_cfg_t;

/**
 * @brief  Create a new GMF pipeline
 *
 * @param[out]  pipeline  Pointer to store the newly created GMF pipeline handle
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  If the pipeline handle is invalid
 *       - ESP_GMF_ERR_MEMORY_LACK  Memory allocation failed
 */
esp_gmf_err_t esp_gmf_pipeline_create(esp_gmf_pipeline_handle_t *pipeline);

/**
 * @brief  Destroy a GMF pipeline, freeing associated resources
 *
 * @param[in]  pipeline  GMF pipeline handle to destroy
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  If the pipeline handle is invalid
 */
esp_gmf_err_t esp_gmf_pipeline_destroy(esp_gmf_pipeline_handle_t pipeline);

/**
 * @brief  Set the I/O handle for a GMF pipeline
 *
 * @param[in]  pipeline  GMF pipeline handle
 * @param[in]  io        GMF I/O handle
 * @param[in]  dir       I/O direction (input or output)
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  If the pipeline handle or I/O port is invalid
 */
esp_gmf_err_t esp_gmf_pipeline_set_io(esp_gmf_pipeline_handle_t pipeline, esp_gmf_io_handle_t io, int dir);

/**
 * @brief  Register a GMF element with a GMF pipeline
 *
 * @param[in]  pipeline  GMF pipeline handle
 * @param[in]  el        GMF element handle to register
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  if the pipeline handle or element is invalid
 */
esp_gmf_err_t esp_gmf_pipeline_register_el(esp_gmf_pipeline_handle_t pipeline, esp_gmf_element_handle_t el);

/**
 * @brief  List all GMF elements in the pipeline
 *
 * @param[in]  pipeline  GMF pipeline handle
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  If the pipeline handle is invalid
 */
esp_gmf_err_t esp_gmf_pipeline_list_el(esp_gmf_pipeline_handle_t pipeline);

/**
 * @brief  Set the event callback function for a GMF pipeline
 *
 * @param[in]  pipeline  GMF pipeline handle
 * @param[in]  cb        Event callback function
 * @param[in]  ctx       Context to be passed to the callback function
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  If the pipeline handle is invalid
 */
esp_gmf_err_t esp_gmf_pipeline_set_event(esp_gmf_pipeline_handle_t pipeline, esp_gmf_event_cb cb, void *ctx);

/**
 * @brief  Bind a given task to the pipeline
 *
 * @param[in]  pipeline  GMF pipeline handle
 * @param[in]  task      GMF task handle
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  If the pipeline or task handle is invalid
 */
esp_gmf_err_t esp_gmf_pipeline_bind_task(esp_gmf_pipeline_handle_t pipeline, esp_gmf_task_handle_t task);

/**
 * @brief  Load linked element jobs to the bind task on the specific pipeline
 *
 * @param[in]  pipeline  GMF pipeline handle
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  If the pipeline or task handle is invalid
 */
esp_gmf_err_t esp_gmf_pipeline_loading_jobs(esp_gmf_pipeline_handle_t pipeline);

/**
 * @brief  Set the input URI for the specific pipeline
 *
 * @param[in]  pipeline  GMF pipeline handle
 * @param[in]  uri       Input URI
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  If the pipeline handle is invalid
 *       - ESP_GMF_ERR_MEMORY_LACK  Memory allocation for URI failed
 */
esp_gmf_err_t esp_gmf_pipeline_set_in_uri(esp_gmf_pipeline_handle_t pipeline, const char *uri);

/**
 * @brief  Set the output URI for specific pipeline
 *
 * @param[in]  pipeline  GMF pipeline handle
 * @param[in]  uri       Output URI
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  If the pipeline handle is invalid
 *       - ESP_GMF_ERR_MEMORY_LACK  Memory allocation for URI failed
 */
esp_gmf_err_t esp_gmf_pipeline_set_out_uri(esp_gmf_pipeline_handle_t pipeline, const char *uri);

/**
 * @brief  Register an event recipient for specific pipeline
 *
 * @param[in]  connector  GMF pipeline handle of the connector
 * @param[in]  connectee  GMF pipeline handle of the connectee
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  If the pipeline handles are invalid
 *       - ESP_GMF_ERR_MEMORY_LACK  Memory allocation failed
 */
esp_gmf_err_t esp_gmf_pipeline_reg_event_recipient(esp_gmf_pipeline_handle_t connector, esp_gmf_pipeline_handle_t connectee);

/**
 * @brief  Connect two GMF pipelines
 *
 * @param[in]  connector       GMF pipeline handle of the connector
 * @param[in]  connector_name  Name of the connector element
 * @param[in]  connector_port  Port handle of the connector element
 * @param[in]  connectee       GMF pipeline handle of the connectee
 * @param[in]  connectee_name  Name of the connectee element
 * @param[in]  connectee_port  Port handle of the connectee element
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  If the handles are invalid
 *       - ESP_GMF_ERR_NOT_FOUND    The connector_name is not in the connector pipeline, or connectee_name is not in the connectee pipeline
 *       - ESP_GMF_ERR_NOT_SUPPORT  The given port is not support
 */
esp_gmf_err_t esp_gmf_pipeline_connect_pipe(esp_gmf_pipeline_handle_t connector, const char *connector_name, esp_gmf_port_handle_t connector_port,
                                            esp_gmf_pipeline_handle_t connectee, const char *connectee_name, esp_gmf_port_handle_t connectee_port);

/**
 * @brief  Set the callback function to be called at `esp_gmf_pipeline_stop` is executed.
 *         If `prev_stop_cb` is NULL, no previous stop function will be invoked.
 *
 * @param[in]  pipeline      Handle to the GMF pipeline
 * @param[in]  prev_stop_cb  Callback function to execute before stopping the pipeline
 * @param[in]  ctx           Context to be passed to the previous stop function
 *
 * @return
 *       - ESP_GMF_ERR_OK           Success
 *       - ESP_GMF_ERR_INVALID_ARG  Invalid pipeline or callback argument
 */
esp_gmf_err_t esp_gmf_pipeline_set_prev_stop_cb(esp_gmf_pipeline_handle_t pipeline, esp_gmf_pipeline_prev_stop prev_stop_cb, void *ctx);

/**
 * @brief  Run the specific pipeline using `esp_gmf_task_run`, which blocks by default for `DEFAULT_TASK_OPT_MAX_TIME_MS`
 *         To change the waiting time, use `esp_gmf_task_set_timeout`
 *
 * @param[in]  pipeline  GMF pipeline handle
 *
 * @return
 *       - ESP_GMF_ERR_OK             On success
 *       - ESP_GMF_ERR_INVALID_ARG    If the pipeline or thread handle is invalid
 *       - ESP_GMF_ERR_NOT_SUPPORT    Indicating the state of task is ESP_GMF_EVENT_STATE_PAUSED or ESP_GMF_EVENT_STATE_RUNNING
 *       - ESP_GMF_ERR_INVALID_STATE  The task is not running
 *       - ESP_GMF_ERR_TIMEOUT        Indicating that the synchronization operation has timed out
 */
esp_gmf_err_t esp_gmf_pipeline_run(esp_gmf_pipeline_handle_t pipeline);

/**
 * @brief  Stop a running pipeline using `esp_gmf_task_stop`, which blocks by default for `DEFAULT_TASK_OPT_MAX_TIME_MS`
 *         To change the waiting time, use `esp_gmf_task_set_timeout`
 *         The previous stop called before the task stop if the previous stop callback is valid
 *
 * @param[in]  pipeline  GMF pipeline handle
 *
 * @return
 *       - ESP_GMF_ERR_OK             On success
 *       - ESP_GMF_ERR_INVALID_ARG    If the pipeline or thread handle is invalid
 *       - ESP_GMF_ERR_NOT_SUPPORT    The state of task is ESP_GMF_EVENT_STATE_NONE
 *       - ESP_GMF_ERR_INVALID_STATE  The task is not running
 *       - ESP_GMF_ERR_TIMEOUT        Indicating that the synchronization operation has timed out
 */
esp_gmf_err_t esp_gmf_pipeline_stop(esp_gmf_pipeline_handle_t pipeline);

/**
 * @brief  Pause a running GMF pipeline using `esp_gmf_task_pause`, which blocks by default for `DEFAULT_TASK_OPT_MAX_TIME_MS`
 *         To change the waiting time, use `esp_gmf_task_set_timeout`
 *
 * @param[in]  pipeline  GMF pipeline handle
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  If the pipeline or thread handle is invalid
 *       - ESP_GMF_ERR_NOT_SUPPORT  The state of task is not ESP_GMF_EVENT_STATE_RUNNING
 *       - ESP_GMF_ERR_TIMEOUT      Indicating that the synchronization operation has timed out
 */
esp_gmf_err_t esp_gmf_pipeline_pause(esp_gmf_pipeline_handle_t pipeline);

/**
 * @brief  Resume a paused GMF pipeline using `esp_gmf_task_resume`, which blocks by default for `DEFAULT_TASK_OPT_MAX_TIME_MS`
 *         To change the waiting time, use `esp_gmf_task_set_timeout`
 *
 * @param[in]  pipeline  GMF pipeline handle
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  If the pipeline or thread handle is invalid
 *       - ESP_GMF_ERR_NOT_SUPPORT  The state of task is not ESP_GMF_EVENT_STATE_PAUSED
 *       - ESP_GMF_ERR_TIMEOUT      Indicating that the synchronization operation has timed out
 */
esp_gmf_err_t esp_gmf_pipeline_resume(esp_gmf_pipeline_handle_t pipeline);

/**
 * @brief  Reset the GMF pipeline to its initial state, including job lists, port states, and element states
 *         To run the pipeline again, `esp_gmf_pipeline_loading_jobs` must be called
 *
 * @param[in]  pipeline  GMF pipeline handle
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  If the pipeline handle is invalid
 */
esp_gmf_err_t esp_gmf_pipeline_reset(esp_gmf_pipeline_handle_t pipeline);

/**
 * @brief  Seeking to a specific position in the pipeline calls `io_seek`, which means it only
 *         supports streaming audio formats like MP3, AAC, and TS, where each frame can be decoded independently
 *
 * @param[in]  pipeline  GMF pipeline handle
 * @param[in]  pos       Position to seek to
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  If the pipeline handle is invalid
 *       - ESP_GMF_ERR_INVALID_STATE
 */
esp_gmf_err_t esp_gmf_pipeline_seek(esp_gmf_pipeline_handle_t pipeline, uint64_t pos);

/**
 * @brief  Retrieve the linked pipeline from given pipeline
 *
 * @param[in]   connector  Handle to the GMF connector pipeline
 * @param[out]  iterator   A void pointer to the iterative pipeline
 * @param[out]  connectee  Handle to the connected pipeline
 *
 * @return
 *       - ESP_GMF_ERR_OK             Success
 *       - ESP_GMF_ERR_INVALID_ARG    Invalid connector or connectee handle
 *       - ESP_GMF_ERR_NOT_FOUND      No linked pipeline found
 *       - ESP_GMF_ERR_INVALID_STATE  Connector is not in a valid state for linking
 */
esp_gmf_err_t esp_gmf_pipeline_get_linked_pipeline(esp_gmf_pipeline_handle_t connector, const void **iterator, esp_gmf_pipeline_handle_t *connectee);

/**
 * @brief  Get the input I/O of a GMF pipeline
 *
 * @param[in]   pipeline   GMF pipeline handle
 * @param[out]  io_handle  Pointer to store the input I/O handle
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  If the pipeline handle is invalid
 */
esp_gmf_err_t esp_gmf_pipeline_get_in(esp_gmf_pipeline_handle_t pipeline, esp_gmf_io_handle_t *io_handle);

/**
 * @brief  Get the output I/O of a GMF pipeline
 *
 * @param[in]   pipeline   GMF pipeline handle
 * @param[out]  io_handle  Pointer to store the output I/O handle
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  If the pipeline or thread handle is invalid
 */
esp_gmf_err_t esp_gmf_pipeline_get_out(esp_gmf_pipeline_handle_t pipeline, esp_gmf_io_handle_t *io_handle);

/**
 * @brief  Replace the input I/O of a GMF pipeline with a new one
 *
 * @note  Call esp_gmf_pipeline_get_in function before you call this function
 *        Because of the pineline in will be overwritten and the replaced MUST release by user
 *
 * @param[in]  pipeline  GMF pipeline handle
 * @param[in]  new_one   New input I/O handle
 *
 * @return
 *       - ESP_GMF_ERR_OK             On success
 *       - ESP_GMF_ERR_INVALID_ARG    If the pipeline handle or new I/O handle is invalid
 *       - ESP_GMF_ERR_INVALID_STATE  The pipeline is running, replace is not safe
 */
esp_gmf_err_t esp_gmf_pipeline_replace_in(esp_gmf_pipeline_handle_t pipeline, esp_gmf_io_handle_t new_one);

/**
 * @brief  Replace the output I/O of a GMF pipeline with a new one
 *
 * @note  Call esp_gmf_pipeline_get_out function before you call this function
 *        Because of the pineline in will be overwritten and the replaced MUST release by user
 *
 * @param[in]  pipeline  GMF pipeline handle
 * @param[in]  new_one   New output I/O handle
 *
 * @return
 *       - ESP_GMF_ERR_OK             On success
 *       - ESP_GMF_ERR_INVALID_ARG    If the pipeline handle or new I/O port handle is invalid
 *       - ESP_GMF_ERR_INVALID_STATE  The pipeline is running, replace is not safe
 */
esp_gmf_err_t esp_gmf_pipeline_replace_out(esp_gmf_pipeline_handle_t pipeline, esp_gmf_io_handle_t new_one);

/**
 * @brief  Get the header GMF element in the pipeline
 *
 * @param[in]   pipeline  GMF pipeline handle
 * @param[out]  head      Head element of the pipeline
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  If the pipeline handle or head element is invalid
 */
esp_gmf_err_t esp_gmf_pipeline_get_head_el(esp_gmf_pipeline_handle_t pipeline, esp_gmf_element_handle_t *head);

/**
 * @brief  Get the next GMF element in the pipeline
 *
 * @param[in]   pipeline  GMF pipeline handle
 * @param[in]   head      Head element of the pipeline
 * @param[out]  next      Pointer to store the next GMF element handle
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  If the pipeline handle or head element is invalid
 */
esp_gmf_err_t esp_gmf_pipeline_get_next_el(esp_gmf_pipeline_handle_t pipeline, esp_gmf_element_handle_t head, esp_gmf_element_handle_t *next);

/**
 * @brief  Get a element in specific pipeline by its name
 *
 * @param[in]   pipeline    GMF pipeline handle
 * @param[in]   tag         Name of the wanted element
 * @param[out]  out_handle  Pointer to store the element handle
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  If the pipeline handle or tag is invalid
 *       - ESP_GMF_ERR_NOT_FOUND    It's not found the element by given tag
 */
esp_gmf_err_t esp_gmf_pipeline_get_el_by_name(esp_gmf_pipeline_handle_t pipeline, const char *tag, esp_gmf_element_handle_t *out_handle);

/**
 * @brief  Register an I/O port for an element within the pipeline
 *
 * @param[in]  pipeline  GMF pipeline handle
 * @param[in]  tag       Name of the GMF element
 * @param[in]  io_dir    I/O direction type (input or output)
 * @param[in]  port      I/O port handle to register
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  If the pipeline handle, tag, or port handle is invalid
 *       - ESP_GMF_ERR_NOT_SUPPORT  The specified port type is not supported,
 *                                  or the element can't connect more port, or the given direction is not supported
 *       - ESP_GMF_ERR_NOT_FOUND    It's not found the element by given tag
 */
esp_gmf_err_t esp_gmf_pipeline_reg_el_port(esp_gmf_pipeline_handle_t pipeline, const char *tag, esp_gmf_io_dir_t io_dir, esp_gmf_io_handle_t port);

/**
 * @brief  Report information from first element of pipeline
 *
 * @param[in]  pipeline   The handle to the GMF pipeline
 * @param[in]  info_type  The type of information to report, specified by `esp_gmf_info_type_t`
 * @param[in]  value      Pointer to the buffer where the information will be stored
 * @param[in]  len        Length of the buffer pointed to by `value`, in bytes
 *
 * @return
 *       - ESP_GMF_ERR_OK             Success
 *       - ESP_GMF_ERR_INVALID_ARG    Invalid argument(s) provided
 *       - ESP_GMF_ERR_MEMORY_LACK    Insufficient memory to store the information
 *       - ESP_GMF_ERR_NOT_SUPPORTED  The requested information type is not supported
 */
esp_gmf_err_t esp_gmf_pipeline_report_info(esp_gmf_pipeline_handle_t pipeline, esp_gmf_info_type_t info_type, void *value, int len);

/**
 * @brief  Print information about a GMF pipeline
 *
 * @param[in]  handle  GMF pipeline handle
 *
 * @return
 *       - ESP_GMF_ERR_OK           On success
 *       - ESP_GMF_ERR_INVALID_ARG  If the pipeline handle is invalid
 */
esp_gmf_err_t esp_gmf_pipeline_show(esp_gmf_pipeline_handle_t handle);

#ifdef __cplusplus
}
#endif  /* __cplusplus */
