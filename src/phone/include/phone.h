#ifndef PHONE_H
#define PHONE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <phone_export.h>
#include <stdlib.h>

#define PHONE_STATUS_SUCCESS 0
#define PHONE_STATUS_FAILURE 1

typedef struct phone_instance_t *phone_t;
typedef int phone_status_t;

typedef enum {
    // IDEA ABI breaking change better names: DEVICE_FILTER_NO_FILTER/ALL_DEVICES, DEVICE_FILTER_INPUT_DEVICES
    DEVICE_FILTER_NONE,
    DEVICE_FILTER_INPUT,
    DEVICE_FILTER_OUTPUT
} device_filter_t;

typedef struct {
    int id;
    char *driver;
    char *name;
    unsigned input_count;
    unsigned output_count;
} audio_device_info_t;

PHONE_EXPORT phone_t phone_create(const char *user_agent,
                                  const char * const nameserver[], size_t nameserver_count,
                                  const char * const stunserver[], size_t stunserver_count);
PHONE_EXPORT phone_t phone_create_with_system_nameserver(const char *user_agent,
                                  const char * const stunserver[], size_t stunserver_count);

PHONE_EXPORT void phone_destroy(phone_t instance);

PHONE_EXPORT void phone_register_on_registration_state_callback(phone_t instance, void (*cb)(int is_registered, int registration_state, void *ctx), void *ctx);

PHONE_DEPRECATED_EXPORT void phone_register_on_incoming_call_callback(phone_t instance, void (*cb)(int call_id, void *ctx), void *ctx);
PHONE_EXPORT void phone_register_on_incoming_call_index_callback(phone_t instance, void (*cb)(int call_index, void *ctx), void *ctx);
PHONE_EXPORT void phone_register_on_incoming_call_id_callback(phone_t instance, void (*cb)(const char *call_id, void *ctx), void *ctx);

PHONE_DEPRECATED_EXPORT void phone_register_on_call_state_callback(phone_t instance, void (*cb)(int call_id, int call_state, void *ctx), void *ctx);
PHONE_EXPORT void phone_register_on_call_state_index_callback(phone_t instance, void (*cb)(int call_index, int call_state, void *ctx), void *ctx);
PHONE_EXPORT void phone_register_on_call_state_id_callback(phone_t instance, void (*cb)(const char *call_id, int call_state, void *ctx), void *ctx);

PHONE_EXPORT phone_status_t phone_configure_opus(phone_t instance, int channel_count, int complexity, int sample_rate);
PHONE_EXPORT phone_status_t phone_connect(phone_t instance, const char *server, const char *user, const char *password);

PHONE_EXPORT phone_status_t phone_make_call(phone_t instance, const char *uri);

PHONE_DEPRECATED_EXPORT phone_status_t phone_answer_call(phone_t instance, int call_id);
PHONE_EXPORT phone_status_t phone_answer_call_index(phone_t instance, int call_index);
PHONE_EXPORT phone_status_t phone_answer_call_id(phone_t instance, const char *call_id);

PHONE_DEPRECATED_EXPORT phone_status_t phone_start_ringing_call_index(phone_t instance, int call_index);
PHONE_DEPRECATED_EXPORT phone_status_t phone_start_ringing_call_id(phone_t instance, const char *call_id);

PHONE_EXPORT phone_status_t phone_answer_ringing_call_index(phone_t instance, int call_index);
PHONE_EXPORT phone_status_t phone_answer_ringing_call_id(phone_t instance, const char *call_id);

PHONE_DEPRECATED_EXPORT phone_status_t phone_hangup_call(phone_t instance, int call_id);
PHONE_EXPORT phone_status_t phone_hangup_call_index(phone_t instance, int call_index);
PHONE_EXPORT phone_status_t phone_hangup_call_id(phone_t instance, const char *call_id);

PHONE_EXPORT phone_status_t phone_play_dtmf_call_index(phone_t instance, int call_index, const char *digits);
PHONE_EXPORT phone_status_t phone_play_dtmf_call_id(phone_t instance, const char *call_id, const char *digits);

PHONE_EXPORT void phone_hangup_calls(phone_t instance);

PHONE_EXPORT phone_status_t phone_get_call_id(phone_t instance, int call_index, char *call_id, size_t size);
PHONE_EXPORT phone_status_t phone_get_call_index(phone_t instance, const char *call_id, int *out);

PHONE_EXPORT void phone_refresh_audio_devices(void);
PHONE_EXPORT size_t phone_get_audio_devices_count(void);
PHONE_EXPORT size_t phone_get_audio_device_driver_name_length(void);
PHONE_EXPORT size_t phone_get_audio_device_info_name_length(void);
PHONE_DEPRECATED_EXPORT phone_status_t phone_get_audio_device_names(char **device_names, size_t *devices_count, size_t max_device_name_length, device_filter_t filter);

/**
 * Get the audio devices that are available in the system.
 *
 * @param devices                   Out parameter. Array of phone_audio_device_t
 * @param devices_count             Out parameter. Number of devices available in the system.
 *                                  The value of devices_count might be smaller after the function returned depending
 *                                  on the \p filter parameter.
 *                                  Can be retrieved via \p phone_get_audio_devices_count.
 * @param max_driver_name_length    Maximum length of the audio drivers name.
 *                                  Can be retrieved via \p phone_get_audio_device_driver_name_length.
 * @param max_device_name_length    Maximum length of the audio devices name.
 *                                  Can be retrieved via \p phone_get_audio_device_info_name_length.
 * @param filter                    Filter the devices that should be returned. \p device_filter_t
 */
PHONE_EXPORT phone_status_t
phone_get_audio_devices(audio_device_info_t *devices, size_t *devices_count, size_t max_driver_name_length,
                        size_t max_device_name_length, device_filter_t filter);
PHONE_EXPORT phone_status_t phone_set_audio_devices(int capture_device, int playback_device);

/**
 * Check if the SIP INVITE had a \p Call-Info header that included a value like this:
 * <pre>
 *  Call-Info: \<sip:SERVER\>;answer-after=0
 * </pre>
 * If so this value will be returned via the out parameter \p answer-after, -1 otherwise.
 *
 * @param instance      phone instance
 * @param call_index    the call index
 * @param answer_after  out parameter for the \p answer-after value. -1 if no \p answer-after value exists.
 *
 * @return              phone_status_t
 */
PHONE_EXPORT phone_status_t phone_call_answer_after_index(phone_t instance, int call_index, int *answer_after);
PHONE_EXPORT phone_status_t phone_call_answer_after_id(phone_t instance, const char *call_id, int *answer_after);

PHONE_EXPORT const char* phone_last_error(void);
PHONE_EXPORT void phone_status_name(char *out, size_t buffer_size, int code);

/**
 * @brief Get the human-readable name for a given phone call state.
 *
 * This function retrieves the human-readable name for the provided phone call
 * state and stores it in the provided output buffer.
 *
 * @param[out] out Pointer to the output buffer where the state name will be stored.
 * @param[in] buffer_size Size of the output buffer in bytes.
 * @param[in] state Numeric value of the phone call state.
 *
 * @note Use this function instead of the deprecated phone_state_name().
 */
PHONE_EXPORT void phone_call_state_name(char *out, size_t buffer_size, int state);

/**
 * @brief Get the human-readable name for a given phone state (deprecated).
 *
 * This function retrieves the human-readable name for the provided phone state
 * and stores it in the provided output buffer. It is deprecated and the
 * phone_call_state_name() function should be used instead.
 *
 * @deprecated This function is deprecated, use phone_call_state_name() instead.
 *
 * @param[out] out Pointer to the output buffer where the state name will be stored.
 * @param[in] buffer_size Size of the output buffer in bytes.
 * @param[in] state Numeric value of the phone state.
 */
PHONE_DEPRECATED_EXPORT void phone_state_name(char *out, size_t buffer_size, int state);
PHONE_EXPORT void phone_set_log_level(int level);
PHONE_EXPORT void phone_set_log_function(phone_t instance, void (*fn)(int level, const char *message, long thread_id, const char *thread_name));

PHONE_EXPORT phone_status_t phone_register_thread(phone_t instance, const char *name);
PHONE_EXPORT int phone_is_thread_registered(phone_t instance);

PHONE_EXPORT phone_status_t phone_play_call_waiting(phone_t instance);
PHONE_EXPORT phone_status_t phone_stop_call_waiting(phone_t instance);

PHONE_EXPORT unsigned phone_get_call_count(phone_t instance);

PHONE_EXPORT phone_status_t phone_handle_ip_change(void);

PHONE_EXPORT phone_status_t phone_get_rx_level_call_index(phone_t instance, int call_index, unsigned int *level);
PHONE_EXPORT phone_status_t phone_get_rx_level_call_id(phone_t instance, const char *call_id, unsigned int *level);

PHONE_EXPORT phone_status_t phone_set_rx_level_call_index(phone_t instance, int call_index, float level);
PHONE_EXPORT phone_status_t phone_set_rx_level_call_id(phone_t instance, const char *call_id, float level);

PHONE_EXPORT phone_status_t phone_set_rx_level_capture_device(phone_t instance, float level);

#ifdef __cplusplus
}
#endif

#endif //PHONE_H