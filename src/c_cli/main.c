#include "helper.h"
#include <phone.h>
#include <stdio.h>
#include <string.h>

void die(phone_t instance) {
    phone_destroy(instance);
    fprintf(stderr, "%s\n", phone_last_error());
    exit(EXIT_FAILURE);
}

struct app_state {
    int last_call_index;
    char last_call_id[128];
};

void on_incoming_call_with_index_cb(int call_index, __attribute__((unused)) void *ctx) {
    printf("Incoming call: %d\n", call_index);
}

void on_incoming_call_with_id_cb(const char *call_id, __attribute__((unused)) void *ctx) {
    printf("Incoming call: %s\n", call_id);
}

void on_call_state_with_index_cb(int call_index, int state, void *ctx) {
    char buffer[64];
    phone_state_name(buffer, sizeof(buffer), state);
    struct app_state *s = (struct app_state*)ctx;
    s->last_call_index = call_index;
    printf("Call %d – state: %s\n", call_index, buffer);
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
    bzero(state->last_call_id, sizeof(state->last_call_id));

    const char *nameserver[] = {"217.237.148.22", "217.237.150.51"};
    const char *stunserver[] = {"stun.t-online.de"};
    phone_t phone = phone_create("Cli Phone in C", nameserver, 2, stunserver, 1);
    if (!phone)
        die(phone);
    phone_set_log_level(0);

    // callbacks
    phone_register_on_incoming_call_callback(phone, on_incoming_call_with_index_cb, NULL);
    phone_register_on_incoming_call_index_callback(phone, on_incoming_call_with_index_cb, NULL);
    phone_register_on_incoming_call_id_callback(phone, on_incoming_call_with_id_cb, NULL);

    phone_register_on_call_state_callback(phone, on_call_state_with_index_cb, state);
    phone_register_on_call_state_index_callback(phone, on_call_state_with_index_cb, state);
    phone_register_on_call_state_id_callback(phone, on_call_state_with_id_cb, state);

    // opus
    if (phone_configure_opus(phone, 1, 8, 16000) != PHONE_STATUS_SUCCESS)
        die(phone);

    // connect
    if (phone_connect(phone,
                      "tel.t-online.de",
                      "+4965191899543", NULL) != PHONE_STATUS_SUCCESS)
        die(phone);

    // repl
    int command;
    char buffer[128];
    int call_id, level;
    do {
        printf("last call index: %d\n> ", state->last_call_index);
        printf("last call id: %s\n> ", state->last_call_id);
        command = getchar();
        switch (command) {
            case 'c':
                printf("please enter number: ");
                clear_input_buffer();
                if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                    fprintf(stderr, "could not read number\n");
                    break;
                }
                buffer[strcspn(buffer, "\n")] = '\0';
                if (phone_make_call(phone, buffer) != PHONE_STATUS_SUCCESS) die(phone);
                break;
            case 'C':
                if (phone_make_call(phone, "+4915123595397") != PHONE_STATUS_SUCCESS) die(phone);
                break;
            case 'a':
                printf("please enter call index: ");
                if (read_int(&call_id) != 0) break;
                if (phone_answer_call(phone, call_id) != PHONE_STATUS_SUCCESS) die(phone);
                break;
            case 'A':
                printf("please enter call id: ");
                clear_input_buffer();
                if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                    fprintf(stderr, "could not read id\n");
                    break;
                }
                buffer[strcspn(buffer, "\n")] = '\0';
                if (phone_answer_call_id(phone, buffer) != PHONE_STATUS_SUCCESS) die(phone);
                break;
            case 'h':
                printf("please enter call index: ");
                if (read_int(&call_id) != 0) break;
                if (phone_hangup_call(phone, call_id) != PHONE_STATUS_SUCCESS) die(phone);
                break;
            case 'H':
                printf("please enter call id: ");
                clear_input_buffer();
                if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                    fprintf(stderr, "could not read id\n");
                    break;
                }
                buffer[strcspn(buffer, "\n")] = '\0';
                if (phone_hangup_call_id(phone, buffer) != PHONE_STATUS_SUCCESS) die(phone);
                break;
            case 'e':
                phone_hangup_calls(phone);
                break;
            case 'l':
                printf("please enter new log level 0..6: ");
                if (read_int(&level) != 0) break;
                phone_set_log_level(level);
                break;
            default:
                break;
        }
    } while (command != 'q' && command != EOF);
    printf("shutting down...\n");
    phone_destroy(phone);
    return 0;
}