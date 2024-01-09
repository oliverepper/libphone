#include "helper.h"
#include <phone.h>
#include <phone/version.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

struct app_state {
    phone_t phone;
    int last_call_index;
    char last_call_id[128];
};

void on_registration_state(int is_registered, int registration_state, __attribute__((unused)) void *ctx) {
    char buffer[64];
    phone_status_name(buffer, sizeof(buffer), registration_state);
    if (is_registered) {
        printf("phone registered: %s\n", buffer);
    } else {
        printf("phone unregistered: %s\n", buffer);
    }
}

void on_incoming_call_with_index_cb(int call_index, __attribute__((unused)) void *ctx) {
    struct app_state *s = (struct app_state*)ctx;
    s->last_call_index = call_index;

    char call_id_buffer[128] = {0};
    if (phone_get_call_id(s->phone, call_index, call_id_buffer, sizeof(call_id_buffer)) != PHONE_STATUS_SUCCESS)
        fprintf(stderr, "%s\n", phone_last_error());

    printf("Incoming call index: %d, id: %s\n", call_index, call_id_buffer);

    size_t incoming_message_length = -1;
    if (phone_call_incoming_message_length_index(s->phone, call_index, &incoming_message_length) != PHONE_STATUS_SUCCESS)
        fprintf(stderr, "%s\n", phone_last_error());
    if (incoming_message_length != -1) {
        printf("SIP Invite Message length: %zu\n", incoming_message_length);
        char buffer[incoming_message_length + 1];
        if (phone_call_incoming_message_index(s->phone, call_index, buffer, sizeof(buffer)) != PHONE_STATUS_SUCCESS)
            fprintf(stderr, "%s\n", phone_last_error());
        else
            printf("%s\n\n", buffer);
    }

    int answer_after;
    phone_call_answer_after_index(s->phone, call_index, &answer_after);
    if (answer_after >= 0) {
        sleep(answer_after);
        phone_answer_call_index(s->phone, call_index);
    } else {
        phone_answer_ringing_call_index(s->phone, call_index);
    }
}

void on_incoming_call_with_id_cb(const char *call_id, __attribute__((unused)) void *ctx) {
    struct app_state *s = (struct app_state*)ctx;
    strncpy(s->last_call_id, call_id, sizeof(s->last_call_id));

    if (phone_get_call_index(s->phone, call_id, &s->last_call_index) != PHONE_STATUS_SUCCESS)
        fprintf(stderr, "%s\n", phone_last_error());

    printf("Incoming call id: %s, index: %d\n", call_id, s->last_call_index);

    size_t incoming_message_length = -1;
    if (phone_call_incoming_message_length_id(s->phone, call_id, &incoming_message_length) != PHONE_STATUS_SUCCESS)
        fprintf(stderr, "%s\n", phone_last_error());
    if (incoming_message_length != -1) {
        printf("SIP Invite Message length: %zu\n", incoming_message_length);
        char buffer[incoming_message_length + 1];
        if (phone_call_incoming_message_id(s->phone, call_id, buffer, sizeof(buffer)) != PHONE_STATUS_SUCCESS)
            fprintf(stderr, "%s\n", phone_last_error());
        else
            printf("%s\n", buffer);
    }

    int answer_after = -1;
    if (phone_call_answer_after_id(s->phone, call_id, &answer_after) != PHONE_STATUS_SUCCESS)
        fprintf(stderr, "%s\n", phone_last_error());

    // call is user initiated if >= 0
    if (answer_after >= 0) {
        sleep(answer_after);
        phone_answer_call_id(s->phone, call_id);
    } else {
        phone_answer_ringing_call_id(s->phone, call_id);
    }
}

void on_call_state_with_index_cb(int call_index, int state, void *ctx) {
    struct app_state *s = (struct app_state*)ctx;
    s->last_call_index = call_index;

    char call_state_buffer[64] = {0};
    phone_call_state_name(call_state_buffer, sizeof(call_state_buffer), state);

    printf("Call %d – state: %s\n", call_index, call_state_buffer);
}

void on_call_state_with_id_cb(const char* call_id, int state, void *ctx) {
    struct app_state *s = (struct app_state*)ctx;
    strncpy(s->last_call_id, call_id, sizeof(s->last_call_id));

    char buffer[64];
    phone_call_state_name(buffer, sizeof(buffer), state);
    printf("Call %s – state: %s\n", call_id, buffer);
}

void log_function(int level, const char *message, long thread_id, const char *thread_name) {
    static FILE *out = {0};
    if (out == 0) {
        char filename[32];
        char fullpath[PATH_MAX];
        strncpy(filename, "last_log.txt", sizeof(filename));
        if ((out = fopen(filename, "w")) == NULL) {
            fprintf(stderr, "could not open file: %s\n", filename);
        }
        realpath(filename, fullpath);
    }
    fprintf(out, "%s(%lu), %d – %s", thread_name, thread_id, level, message);
}

int main() {
    struct app_state *state = malloc(sizeof(struct app_state));
    state->last_call_index = -1;
    memset(state->last_call_id, 0, sizeof(state->last_call_id));

    // create phone in app state
    const char *stunserver[] = {"stun.t-online.de"};
    state->phone = phone_create_with_system_nameserver("Cli Phone in C", stunserver, 1);
    if (!state->phone)
        die(state->phone);

    // logging
    phone_set_log_level(0);
    phone_set_log_function(state->phone, log_function);

    // callbacks
    phone_register_on_registration_state_callback(state->phone, on_registration_state, NULL);

    phone_register_on_incoming_call_index_callback(state->phone, on_incoming_call_with_index_cb, state);
    phone_register_on_incoming_call_id_callback(state->phone, on_incoming_call_with_id_cb, state);

    phone_register_on_call_state_index_callback(state->phone, on_call_state_with_index_cb, state);
    phone_register_on_call_state_id_callback(state->phone, on_call_state_with_id_cb, state);

    // opus
    if (phone_configure_opus(state->phone, 1, 8, 16000) != PHONE_STATUS_SUCCESS)
        die(state->phone);

    // connect
//    if (phone_connect(state->phone, SERVER, USER, PASSWORD) != PHONE_STATUS_SUCCESS)
//        die(state->phone);

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
                if (phone_make_call(state->phone, BUDDY_NUMBER) != PHONE_STATUS_SUCCESS)
                    fprintf(stderr, "%s\n", phone_last_error());
                break;
            case 'a':
                clear_input_buffer();
                {
                    int call_index;
                    printf("please enter call index: ");
                    if (read_int(&call_index) != 0) break;
                    if (phone_answer_call_index(state->phone, call_index) != PHONE_STATUS_SUCCESS)
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
                    if (phone_hangup_call_index(state->phone, call_index) != PHONE_STATUS_SUCCESS)
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
                    size_t max_driver_name_length;
                    if (phone_get_audio_device_driver_name_length(&max_driver_name_length) != PHONE_STATUS_SUCCESS) {
                        fprintf(stderr, "%s\n", phone_last_error());
                        break;
                    }
                    // +1 for zero termination!
                    ++max_driver_name_length;

                    size_t max_device_name_length =
                            phone_get_audio_device_info_name_length() + 1; // +1 for zero termination

                    audio_device_info_t devices[count];
                    char driver_names[count][max_driver_name_length];
                    char device_names[count][max_device_name_length];

                    int i;
                    for (i = 0; i < count; i++) {
                        devices[i].driver = driver_names[i];
                        devices[i].name = device_names[i];
                    }

                    if (phone_get_audio_devices(devices, &count, max_driver_name_length, max_device_name_length,
                                                DEVICE_FILTER_NONE) != PHONE_STATUS_SUCCESS)
                        fprintf(stderr, "%s\n", phone_last_error());

                    for (i = 0; i < count; i++) {
                        printf("%d - %s/%s (%d/%d)\n", devices[i].id, devices[i].driver, devices[i].name,
                               devices[i].input_count, devices[i].output_count);
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
            case 'S':
                clear_input_buffer();
                {
                    printf("Disconnecting audio devices from bridge\n");
                    phone_disconnect_sound_device();
                }
                break;
            case 'n':
                clear_input_buffer();
                {
                    printf("Setting no sound devices\n");
                    phone_set_no_sound_devices();
                }
                break;
            case 'p':
                clear_input_buffer();
                {
                    char dtmf_chars[128];
                    printf("please dtmf characters: ");
                    if (read_string(dtmf_chars, sizeof(dtmf_chars)) != 0) break;
                    if (phone_play_dtmf_call_id(state->phone, state->last_call_id, dtmf_chars) != PHONE_STATUS_SUCCESS)
                        fprintf(stderr, "%s\n", phone_last_error());
                }
                break;
            case 'b':
                phone_play_call_waiting(state->phone);
                break;
            case 'B':
                phone_stop_call_waiting(state->phone);
                break;
            case '#':
                printf("call count: %d\n", phone_get_call_count(state->phone));
                break;
            case 'i':
                printf("handle ip change\n");
                phone_handle_ip_change();
                break;
            case 't':
                clear_input_buffer();
                {
                    float level;
                    if (phone_get_tx_level_adjustment_for_capture_device(state->phone, &level) != PHONE_STATUS_SUCCESS)
                        fprintf(stderr, "%s\n", phone_last_error());
                    printf("tx level adjustment for capture device: %.1f\n", level);
                }
                break;
            case 'r':
                clear_input_buffer();
                {
                    float level;
                    if (phone_get_rx_level_adjustment_for_capture_device(state->phone, &level) != PHONE_STATUS_SUCCESS)
                        fprintf(stderr, "%s\n", phone_last_error());
                    printf("last rx level for call: %.1f\n", level);
                }
                break;
            case 'm':
                clear_input_buffer();
                printf("adjust tx level for capture device to 0\n");
                if (phone_adjust_tx_level_for_capture_device(state->phone, 0) != PHONE_STATUS_SUCCESS)
                    fprintf(stderr, "%s\n", phone_last_error());
                break;
            case 'M':
                clear_input_buffer();
                printf("adjust tx level for capture device to 1\n");
                if (phone_adjust_tx_level_for_capture_device(state->phone, 1) != PHONE_STATUS_SUCCESS)
                    fprintf(stderr, "%s\n", phone_last_error());
                break;
            case '0':
                clear_input_buffer();
                printf("disconnecting\n");
                if (phone_disconnect(state->phone) != PHONE_STATUS_SUCCESS)
                    fprintf(stderr, "%s\n", phone_last_error());
                break;
            case '9':
                clear_input_buffer();
                printf("connecting\n");
                if (phone_connect(state->phone, SERVER, USER, PASSWORD) != PHONE_STATUS_SUCCESS)
                    fprintf(stderr, "%s\n", phone_last_error());
                break;
            case '!':
                clear_input_buffer();
                phone_crash();
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