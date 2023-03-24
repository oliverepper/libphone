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
PHONE_EXPORT void phone_destroy(phone_t instance);

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

PHONE_DEPRECATED_EXPORT phone_status_t phone_hangup_call(phone_t instance, int call_id);
PHONE_EXPORT phone_status_t phone_hangup_call_index(phone_t instance, int call_index);
PHONE_EXPORT phone_status_t phone_hangup_call_id(phone_t instance, const char *call_id);

PHONE_EXPORT void phone_hangup_calls(phone_t instance);

PHONE_EXPORT phone_status_t phone_get_call_id(phone_t instance, int call_index, char *out, size_t size);
PHONE_EXPORT phone_status_t phone_get_call_index(phone_t instance, const char *call_id, int *out);

PHONE_EXPORT void phone_refresh_audio_devices(void);
PHONE_EXPORT size_t phone_get_audio_devices_count(void);
PHONE_EXPORT size_t phone_get_audio_device_info_name_length(void);
PHONE_EXPORT phone_status_t phone_get_audio_device_names(char **device_names, size_t *devices_count, size_t max_device_name_length, device_filter_t filter);
PHONE_EXPORT phone_status_t phone_get_audio_devices(audio_device_info_t *devices, size_t *devices_count, size_t max_device_name_length, size_t max_driver_name_length);
PHONE_EXPORT phone_status_t phone_set_audio_devices(int capture_device, int playback_device);

PHONE_EXPORT phone_status_t phone_call_answer_after_index(phone_t instance, int call_index, int *answer_after);
PHONE_EXPORT phone_status_t phone_call_answer_after_id(phone_t instance, const char *call_id, int *answer_after);

PHONE_EXPORT const char* phone_last_error(void);
PHONE_EXPORT void phone_state_name(char *buffer, size_t buffer_size, int state);
PHONE_EXPORT void phone_set_log_level(int level);

#ifdef __cplusplus
}
#endif

#endif //PHONE_H