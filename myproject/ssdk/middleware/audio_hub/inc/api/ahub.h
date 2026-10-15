/**
 * @file ahub_api.h
 * @author shao yi
 * @brief define audio hub's api
 * @version 0.1
 * @date 2022/11/18
 *
 * @copyright Copyright (c) 2022 Semidrive Semiconductor
 *
 */
#ifndef __AHUB_API_H__
#define __AHUB_API_H__

#include <stdlib.h>
#include <sys/types.h>
#include "ringbuffer.h"
#include <driver/snd_driver.h>

/* common audio hub definition here ,user can not modify this part */

#define AHUB_PLAYBACK_PERIOD_MS (20)
#define AHUB_CAPTURE_PERIOD_MS (20)
#define AHUB_SAMPLE_BYTES (SND_SAMPLE_WIDTH_16BITS / 8)

#define AHUB_PERIODS (2)
#define AHUB_STANDBY_DELAY (2000000)  /**< 2s standby delay */

#define AHUB_RETRY_COUNT (5)
#define MAX_CHANNEL_NUMB (16)
#define NAME_LENGTH (32)

/*-------------------------------------------------------------------*/

/**
 * @brief audio hub endpoint enum.
 *
 */
typedef enum {
    AHUB_ENDPOINT_ID0 = 0,
    AHUB_ENDPOINT_ID1 = 1,
    AHUB_ENDPOINT_ID2 = 2,
    AHUB_ENDPOINT_ID3 = 3,
    AHUB_ENDPOINT_ID4 = 4,
    AHUB_ENDPOINT_ID5 = 5,
    AHUB_ENDPOINT_ID6 = 6,
    AHUB_ENDPOINT_ID7 = 7,
    AHUB_ENDPOINT_ID8 = 8,
    AHUB_ENDPOINT_ID9 = 9,
    AHUB_ENDPOINT_ID10 = 10,
    AHUB_ENDPOINT_ID11 = 11,
    AHUB_ENDPOINT_ID12 = 12,
    AHUB_ENDPOINT_ID13 = 13,
    AHUB_ENDPOINT_ID14 = 14,
    AHUB_ENDPOINT_ID15 = 15,
    AHUB_ENDPOINT_NUMB = 16,
} AHUB_ENDPOINTS;

/**
 * @brief playback stream and capture stream id
 *
 */
typedef enum {
    AHUB_PLAYBACK_STREAM_ID0 = 0,
    AHUB_PLAYBACK_STREAM_ID1 = 1,
    AHUB_PLAYBACK_STREAM_ID2 = 2,
    AHUB_PLAYBACK_STREAM_ID3 = 3,
    AHUB_PLAYBACK_STREAM_ID4 = 4,
    AHUB_PLAYBACK_STREAM_ID5 = 5,
    AHUB_PLAYBACK_STREAM_ID6 = 6,
    AHUB_PLAYBACK_STREAM_ID7 = 7,
    AHUB_CAPTURE_STREAM_ID0 = 8,
    AHUB_CAPTURE_STREAM_ID1 = 9,
    AHUB_CAPTURE_STREAM_ID2 = 10,
    AHUB_CAPTURE_STREAM_ID3 = 11,
    AHUB_STREAM_NUMB = 12
} AHUB_STREAMS;

#define AHUB_PLAYBACK_STREAM_NUMB (8)
#define AHUB_CAPTURE_STREAM_NUMB (4)
#define AHUB_UNUSED (0)
#define AHUB_PLAYBACK_STREAM_MASK (0xff)
#define AHUB_CAPTURE_STREAM_MASK (0xf00)

typedef enum {
    AHUB_RET_OK = 0,
    AHUB_RET_ERR,
    AHUB_RET_ERR_UNKNOWN
} AHUB_RESULT;

typedef enum {
    AHUB_UNINITED = 0,
    AHUB_INITED = 1,
    AHUB_RESETING = 2,
} ahub_status;

#define VOL_STEPS (146)
#define MUTE_EP (0)
#define EP_0DB_GAIN (121)
/* ----------------------------endpoint----------------------------------------*/

/**
 * @brief endpoint_params is used to set endpoint parameters
 *
 */
typedef struct endpoint_params{
    u_int32_t endpoint_id;
    u_int32_t channels; /**< fixed to 2*/
    u_int32_t sample_bytes; /**<audio sample width 8bits: 1 16bits: 2 32bits :4 */
    enum snd_sample_rate sample_rate; //Up to 48000
} endpoint_params_t;

typedef enum {
    MIX_NORMAL = 0, /**< one channel to one channel*/
    MONO_EXPAND = 1,/**< expand one mono channel to several channels */
    EXC_MODE = 2, /**< exclusive mode, only one resource will be playback  */
    MAY_DUCK_MODE = 3,/**< transient may duck mode, only one resource will
      be playback, other resource may duck. Apply a factor 0.2 or -14dB */
} ahub_mix_ctrl;

/**
 * @brief mix policy structure for playback stream.
 *       for example:
 *       {AHUB_ENDPOINT_ID1, 2, 2, 0},
 *       means AHUB_ENDPOINT_ID1 slot0 mix to playback stream slot2,
 *       slot1 mix to playback stream slot3
 */
typedef struct ep_mix_params {
    u_int32_t endpoint_id;  /**< enum AHUB_INPUT_ENDPOINT */
    u_int32_t start_slot;   /**< from 0 to max output stream max slot.*/
    u_int32_t slot_len;     /**<  1 or 2 */
    u_int32_t control_bit;  /**<  Reserved for mix policy */
} ep_mix_params_t;
/**
 * @brief fetch policy structure for playback stream.
 *        used to fetch captrue stream to endpoint.
 *        for example:
 *       {AHUB_ENDPOINT_ID8, 0, 2, 0},
 *       means fetch capture stream slot0 to endpoint AHUB_ENDPOINT_ID8 slot0,
 *       fetch capture stream slot1 to endpoint AHUB_ENDPOINT_ID8 slot1,
 */
typedef struct ep_fetch_params {
    u_int32_t endpoint_id; /**< enum AHUB_INPUT_ENDPOINT */
    u_int32_t start_slot;  /**<  from 0 to max output stream max slot.*/
    u_int32_t slot_len;    /**<  1 or 2 */
    u_int32_t control_bit; /**<  Reserved for fetch policy */
} ep_fetch_params_t;

/**
 * @brief used for fetch a multi-channel audio buffer data to endpoint
 *
 */
typedef struct eps_fetch_policy {
    u_int32_t eps_count; /**< policy count*/
    u_int32_t archor_endpoint; /**< the endpoint deside function ahub_write_to_eps block time*/
    u_int32_t sample_bytes;
    ep_fetch_params_t fetch_policy[AHUB_ENDPOINT_NUMB];
} eps_fetch_policy_t;
/**
 * @brief used for mix endpoint to a multi-channel audio buffer
 *
 */
typedef struct eps_mix_policy {
    u_int32_t eps_count; /**< policy count*/
    u_int32_t archor_endpoint;/**< the endpoint deside function ahub_read_from_eps block time*/
    u_int32_t sample_bytes;
    ep_mix_params_t mix_policy[AHUB_ENDPOINT_NUMB];
} eps_mix_policy_t;
/**
 * @brief stream attribution
 *
 */
typedef struct stream_params {
    u_int32_t channels;
    enum snd_sample_rate sample_rate; /**< audio sample rate*/
    u_int32_t sample_bytes;  /**< audio sample width 8bits: 1 16bits: 2 32bits :4*/
    u_int32_t ep_policy_count;   /**< TODO: need check whether need this, if ep
                                 cannot allow duplicate.*/
    ep_mix_params_t *mix_policy; /**<only for playback stream.*/
    ep_fetch_params_t *fetch_policy; /**< only for capture stream.*/
} stream_params_t;
/**
 * @brief audio hub hardware interface
 *
 */
typedef struct ahub_hw_dev {
    int (*init)(u_int32_t stream_id); /**< init hw dev for stream*/
    int (*release)(u_int32_t stream_id);/**< release hw dev for stream*/

    int (*open_playback_stream)(u_int32_t stream_id); /**< open hw dev for playback stream*/
    int (*close_playback_stream)(u_int32_t stream_id);/**< close hw dev for playback stream*/
    int (*open_capture_stream)(u_int32_t stream_id);  /**< open hw dev for capture stream*/
    int (*close_capture_stream)(u_int32_t stream_id); /**< close hw dev for capture stream*/

    size_t (*read)(u_int32_t stream_id, void *buffer, size_t bytes);/**< read from hw dev for capture stream*/
    size_t (*write)(u_int32_t stream_id, const void *buffer, size_t bytes);/**< write to hw dev for playback stream*/

} ahub_hw_dev_t;
/**
 * @brief ahub playback stream internal structure
 *
 */
typedef struct ahub_playback_stream {
    u_int32_t stream_id;
    char name[NAME_LENGTH];
    stream_params_t stream_param;
    u_int32_t exit;          /**< 1; Exit*/
    u_int32_t standby;       /**< 0: no 1: yes*/
    u_int32_t standby_delay; /**< 20ms ~ 2s*/
    osThreadId_t *thread;
    /* input parameter*/
    u_int32_t input_ep_bitmap; /**<bit map for input endpoint eg: 0b100001001*/
    u_int32_t input_ep_count;

    ringbuf_t cbuf;
    u_int32_t periods; /**< Must > 2 */
    u_int32_t frames;  /**< 10ms sample_rate*10/1000 */
    size_t (*do_mix)( ep_mix_params_t *mix_params, void *in_buf, size_t in_ch, void *out_buf,
                     size_t out_ch, size_t frames);

} ahub_playback_stream_t; // playback thread

typedef int (*ahub_write_t)(ahub_playback_stream_t *playback_stream,
                            const void *buffer, size_t bytes);

/**
 * @brief ahub capture stream internal structure
 *
 */
typedef struct ahub_capture_stream {
    u_int32_t stream_id;
    char name[NAME_LENGTH];
    stream_params_t stream_param;
    u_int32_t exit;          /**<  1; Exit */
    u_int32_t standby;       /**<0: no 1: yes*/
    u_int32_t standby_delay; /**< 20ms ~ 2s */
    osThreadId_t *thread;

    u_int32_t output_ep_bitmap; /** <bit map for input endpoint eg: 0b100001001*/
    u_int32_t output_ep_count;
    ringbuf_t cbuf;
    u_int32_t periods; /**< Must > 2 */
    u_int32_t frames;  /**< 10ms sample_rate*10/1000 */
    size_t (*do_fetch)(void *out_buf, ep_fetch_params_t *fetch_params,
                       const void *in_buf, size_t in_ch, size_t frames);

} ahub_capture_stream_t;

typedef int (*ahub_read_t)(ahub_capture_stream_t *capture_stream,
                           const void *buffer, size_t bytes);

/**
 * @brief Initialize audio hub
 *
 * @return int return AHUB_RET_OK or error type
 */
int ahub_init(void);

/**
 * @brief deinitialize audio hub
 *
 * @return int return AHUB_RET_OK or error type
 */
int ahub_deinit(void);

/**
 * @brief create a playback stream to audio hub
 *
 * @param stream_id : playback stream id
 * @param params : playback streparametersam
 * @return int : return AHUB_RET_OK or error type
 */
int ahub_create_playback_stream(u_int32_t stream_id, stream_params_t params);

/**
 * @brief remove playback stream from audio hub
 *
 * @param stream_id : playback stream id.
 */
void ahub_remove_playback_stream(u_int32_t stream_id);

/**
 * @brief register an input endpoint to playback stream
 *
 * @param stream_id : register a endpoint to a playback stream.
 * @param params : input endpoint parameters
 * @return int : return AHUB_RET_OK or error type
 */
int ahub_register_ep_to_playback_stream(u_int32_t stream_id,
                                        endpoint_params_t params);

/**
 * @brief unregister an input endpoint from playback stream
 *
 * @param stream_id : unregister endpoint from this playback stream.
 * @param endpoint_id : unregister endpoint id.
 * @return int : return AHUB_RET_OK or error type
 */
int ahub_unregister_ep_from_playback_stream(u_int32_t stream_id,
                                            u_int32_t endpoint_id);

/**
 * @brief write data to endpoint
 *
 * @param endpoint_id :
 * @param buffer: src audio buffer
 * @param bytes: buffer length
 * @return int : return AHUB_RET_OK or error type
 */
int ahub_write_to_ep(u_int32_t endpoint_id, const void *buffer, size_t bytes);

/**
 * @brief create a capture stream
 *
 * @param stream_id : capture stream id
 * @param params : capture stream parameters
 * @return int : return AHUB_RET_OK or error type
 */
int ahub_create_capture_stream(u_int32_t stream_id, stream_params_t params);

/**
 * @brief remove stream from capture
 *
 * @param stream_id : capture stream id
 */
void ahub_remove_capture_stream(u_int32_t stream_id);

/**
 * @brief register a endpoint to capture stream
 *
 * @param stream_id : capture stream id
 * @param params : endpoint attribution
 * @return int : return AHUB_RET_OK or error type
 */
int ahub_register_ep_to_capture_stream(u_int32_t stream_id,
                                       endpoint_params_t params);
/**
 * @brief unregister a endpoint from capture stream
 *
 * @param stream_id : capture stream id
 * @param endpoint_id
 * @return int : return AHUB_RET_OK or error type
 */
int ahub_unregister_ep_from_capture_stream(u_int32_t stream_id,
                                           u_int32_t endpoint_id);
/**
 * @brief read data from endpoint
 *
 * @param endpoint_id
 * @param buffer : dst audio buffer
 * @param bytes : data length
 * @return int : return AHUB_RET_OK or error type
 */
int ahub_read_from_ep(u_int32_t endpoint_id, void *buffer, size_t bytes);

/**
 * @brief audio hub get endpoint size
 *
 * @param ep_params
 * @return size_t: ep size
 */
size_t ahub_get_ep_size(endpoint_params_t ep_params);

/**
 * @brief audio hub get stream size
 *
 * @param stream_params
 * @return size_t: stream size
 */
size_t ahub_get_stream_size(stream_params_t stream_params);

/**
 * @brief write data to several endpoints as eps_policy
 *
 * @param eps_policy : a policy fetch data from buffer
 * @param buffer : src audio buffer
 * @param bytes : buffer length
 * @param channels : channel number
 * @return int : return AHUB_RET_OK or error type
 */
int ahub_write_to_eps(eps_fetch_policy_t eps_policy, const void *buffer,
                      size_t bytes, u_int32_t channels);

/**
 * @brief read data from several endpoints as eps_policy
 *
 * @param eps_policy : a policy mix audio ep data to a audio buffer
 * @param buffer : dst audio buffer
 * @param bytes : buffer length
 * @param channels : channel number
 * @return int : return AHUB_RET_OK or error type
 */
int ahub_read_from_eps(eps_mix_policy_t eps_policy, void *buffer, size_t bytes,
                       u_int32_t channels);

/**
 * @brief
 *
 * @param endpoint_id  endpoint id
 * @param gain 146 VOL_STEPS (0 ~ 145)
 *        0: MUTE
 *        1: -60.0 dB
 *        ....
 *        121: 0.0 dB
 *        ....
 *        145: 12.0 dB
 *
 * @return int : return AHUB_RET_OK or error type
 */
int ahub_set_ep_input_gain(u_int32_t endpoint_id, u_int32_t gain);

/**
 * @brief
 *
 * @param endpoint_id  endpoint id
 * @param gain 146 VOL_STEPS (0 ~ 145)
 *        0: MUTE
 *        1: -60.0 dB
 *        ....
 *        121: 0.0 dB
 *        ....
 *        145: 12.0 dB
 *
 * @return int : return AHUB_RET_OK or error type
 */
int ahub_set_ep_output_gain(u_int32_t endpoint_id, u_int32_t gain);

/**
 * @brief audio hub get endpoint channels
 *
 * @param endpoint_id
 * @return channels: SND_CHANNEL_NUM_MONO,SND_CHANNEL_NUM_STEREO
 */
int ahub_get_ep_channels(u_int32_t endpoint_id);

/**
 * @brief this function will invoke AHUB_HW_DEV.open_playback_stream
 *        and will set standby to false.
 *
 * @param stream_id : playback stream id
 * @return int : return AHUB_RET_OK or error type
 */
int ahub_open_playback_stream(u_int32_t stream_id);

/**
 * @brief this function will invoke AHUB_HW_DEV.open_capture_stream
 *        and will set standby to false.
 *
 * @param stream_id : capture stream id
 * @return int : return AHUB_RET_OK or error type
 */
int ahub_open_capture_stream(u_int32_t stream_id);
#endif
