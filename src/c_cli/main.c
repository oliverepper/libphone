#include "helper.h"
#include <phone.h>
#include <phone/version.h>
#include <stdio.h>
#include <string.h>

void die(phone_t instance) {
    phone_destroy(instance);
    fprintf(stderr, "%s\n", phone_last_error());
    exit(EXIT_FAILURE);
}

struct app_state {
    phone_t phone;
    int last_call_index;
    char last_call_id[128];
};

void on_incoming_call_with_index_cb(int call_index, __attribute__((unused)) void *ctx) {
    struct app_state *s = (struct app_state*)ctx;
    s->last_call_index = call_index;

    char call_id_buffer[128] = {0};
    if (phone_get_call_id(s->phone, call_index, call_id_buffer, sizeof(call_id_buffer)) != PHONE_STATUS_SUCCESS)
        fprintf(stderr, "%s\n", phone_last_error());

    printf("Incoming call index: %d, id: %s\n", call_index, call_id_buffer);
}

void on_incoming_call_with_id_cb(const char *call_id, __attribute__((unused)) void *ctx) {
    struct app_state *s = (struct app_state*)ctx;
    strncpy(s->last_call_id, call_id, sizeof(s->last_call_id));

    if (phone_get_call_index(s->phone, call_id, &s->last_call_index) != PHONE_STATUS_SUCCESS)
        fprintf(stderr, "%s\n", phone_last_error());

    printf("Incoming call id: %s, index: %d\n", call_id, s->last_call_index);
}

void on_call_state_with_index_cb(int call_index, int state, void *ctx) {
    struct app_state *s = (struct app_state*)ctx;
    s->last_call_index = call_index;

    char call_state_buffer[64] = {0};
    phone_state_name(call_state_buffer, sizeof(call_state_buffer), state);

    printf("Call %d – state: %s\n", call_index, call_state_buffer);
}

void on_call_state_with_id_cb(const char* call_id, int state, void *ctx) {
    char buffer[64];
    phone_state_name(buffer, sizeof(buffer), state);
    struct app_state *s = (struct app_state*)ctx;
    strncpy(s->last_call_id, call_id, sizeof(s->last_call_id));
    printf("Call %s – state: %s\n", call_id, buffer);
}


int main() {
    struct app_state *state = malloc(sizeof(struct app_state));
    state->last_call_index = -1;
    memset(state->last_call_id, 0, sizeof(state->last_call_id));

    const char *nameserver[] = {"217.237.148.22", "217.237.150.51"};
    const char *stunserver[] = {"stun.t-online.de"};
    state->phone = phone_create("Cli Phone in C", nameserver, 2, stunserver, 1);
    if (!state->phone)
        die(state->phone);
    phone_set_log_level(0);

    // callbacks
    phone_register_on_incoming_call_callback(state->phone, on_incoming_call_with_index_cb, state);
    phone_register_on_incoming_call_index_callback(state->phone, on_incoming_call_with_index_cb, state);
    phone_register_on_incoming_call_id_callback(state->phone, on_incoming_call_with_id_cb, state);

    phone_register_on_call_state_callback(state->phone, on_call_state_with_index_cb, state);
    phone_register_on_call_state_index_callback(state->phone, on_call_state_with_index_cb, state);
    phone_register_on_call_state_id_callback(state->phone, on_call_state_with_id_cb, state);

    // opus
    if (phone_configure_opus(state->phone, 1, 8, 16000) != PHONE_STATUS_SUCCESS)
        die(state->phone);

    // connect
    if (phone_connect(state->phone,
                      "tel.t-online.de",
                      "+4965191899543", NULL) != PHONE_STATUS_SUCCESS)
        die(state->phone);

    // repl
    int command;
    char git_hash[128];
    char git_description[128];
    phone_git_hash(git_hash, sizeof(git_hash));
    phone_git_description(git_description, sizeof(git_description));
    do {
        printf("last call index: %d\n", state->last_call_index);
        printf("last call id: %s\n", state->last_call_id);
        printf("libphone version %d.%d.%d (%s)\n",
               phone_version_major(), phone_version_minor(), phone_version_patch(), git_hash);
        printf("%s\n\n", git_description);

        command = getchar();

        switch (command) {
            case 'c':
                clear_input_buffer();
                {
                    char number_buffer[128];
                    printf("please enter number: ");
                    if (read_string(number_buffer, sizeof(number_buffer)) != 0) break;
                    if (phone_make_call(state->phone, number_buffer) != PHONE_STATUS_SUCCESS)
                        fprintf(stderr, "%s\n", phone_last_error());
                }
                break;
            case 'C':
                clear_input_buffer();
                if (phone_make_call(state->phone, "+491804100100") != PHONE_STATUS_SUCCESS)
                    fprintf(stderr, "%s\n", phone_last_error());
                break;
            case 'a':
                clear_input_buffer();
                {
                    int call_index;
                    printf("please enter call index: ");
                    if (read_int(&call_index) != 0) break;
                    if (phone_answer_call(state->phone, call_index) != PHONE_STATUS_SUCCESS)
                        fprintf(stderr, "%s\n", phone_last_error());
                }
                break;
            case 'A':
                clear_input_buffer();
                {
                    char call_id[128];
                    printf("please enter call id: ");
                    if (read_string(call_id, sizeof(call_id)) != 0) break;
                    if (phone_answer_call_id(state->phone, call_id) != PHONE_STATUS_SUCCESS)
                        fprintf(stderr, "%s\n", phone_last_error());
                }
                break;
            case 'h':
                clear_input_buffer();
                {
                    int call_index;
                    printf("please enter call index: ");
                    if (read_int(&call_index) != 0) break;
                    if (phone_hangup_call(state->phone, call_index) != PHONE_STATUS_SUCCESS)
                        fprintf(stderr, "%s\n", phone_last_error());
                }
                break;
            case 'H':
                clear_input_buffer();
                {
                    char call_id[128];
                    printf("please enter call id: ");
                    if (read_string(call_id, sizeof(call_id)) != 0) break;
                    if (phone_hangup_call_id(state->phone, call_id) != PHONE_STATUS_SUCCESS)
                        fprintf(stderr, "%s\n", phone_last_error());
                }
                break;
            case 'e':
                clear_input_buffer();
                phone_hangup_calls(state->phone);
                break;
            case 'l':
                clear_input_buffer();
                {
                    int log_level;
                    printf("please enter new log level 0..6: ");
                    if (read_int(&log_level) != 0) break;
                    phone_set_log_level(log_level);
                }
                break;
            case 'd':
                clear_input_buffer();
                {
                    phone_refresh_audio_devices();
                    size_t count = phone_get_audio_devices_count();
                    size_t max_device_name_length = phone_get_audio_device_info_name_length();
                    char *device_names[count];
                    char data[count][max_device_name_length];

                    int i;
                    for (i = 0; i < count; i++) {
                        device_names[i] = data[i];
                        memset(data[i], 0, sizeof(max_device_name_length));
                    }

                    if (phone_get_audio_device_names(device_names, &count, max_device_name_length, DEVICE_FILTER_NONE) != PHONE_STATUS_SUCCESS)
                        fprintf(stderr, "%s\n", phone_last_error());

                    for (i = 0; i < count; i++) {
                        printf("%d - %s\n", i, device_names[i]);
                    }
                }
                break;
            case 'D':
                clear_input_buffer();
                {
                    int capture_device;
                    int playback_device;
                    printf("please enter desired capture device: ");
                    if (read_int(&capture_device) != 0) break;
                    printf("please enter desired playback device: ");
                    if (read_int(&playback_device) != 0) break;
                    if (phone_set_audio_devices(capture_device, playback_device) != PHONE_STATUS_SUCCESS)
                        fprintf(stderr, "%s\n", phone_last_error());
                }
                break;
            default:
                clear_input_buffer();
                break;
        }
    } while (command != 'q' && command != EOF);
    printf("shutting down...\n");
    phone_destroy(state->phone);
    return 0;
}