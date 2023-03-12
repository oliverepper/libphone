#include <phone.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

void die(phone_t instance) {
    phone_destroy(instance);
    fprintf(stderr, "%s\n", phone_last_error());
    exit(EXIT_FAILURE);
}

struct app_state {
    int last_call_id;
};

void on_incoming_call(int call_id, __attribute__((unused)) void *ctx) {
    printf("Incoming call: %d\n", call_id);
}

void on_call_state(int call_id, int state, void *ctx) {
    char buffer[64];
    phone_state_name(buffer, sizeof(buffer), state);
    struct app_state *s = (struct app_state*)ctx;
    s->last_call_id = call_id;
    printf("Call %d – state: %s\n", call_id, buffer);
}

void clear_input_buffer(void) {
    int c;
    while ((c = getchar() != '\n') && c != EOF);
}

int read_int(int *in) {
    char input[11];
    long value;
    char *endptr;

    clear_input_buffer();
    if (fgets(input, sizeof(input), stdin) == NULL) {
        fprintf(stderr, "could not read from stdin");
        return 1;
    }

    input[strcspn(input, "\n")] = '\0';

    value = strtol(input, &endptr, sizeof(input));
    if (*input == '\0' || *endptr != '\0') {
        fprintf(stderr, "invalid input.\n");
        return  1;
    }

    if (value <= INT_MAX && value >= INT_MIN) {
        *in = (int)value;
    } else {
        fprintf(stderr, "invalid input.\n");
        return  1;
    }
    return 0;
}

int main() {
    struct app_state *state = malloc(sizeof(struct app_state));
    state->last_call_id = -1;

    const char *nameserver[] = {"217.237.148.22", "217.237.150.51"};
    const char *stunserver[] = {"stun.t-online.de"};
    phone_t phone = phone_create("Cli Phone in C", nameserver, 2, stunserver, 1);
    if (!phone)
        die(phone);
    phone_set_log_level(0);

    phone_register_on_incoming_call_callback(phone, on_incoming_call, NULL);
    phone_register_on_call_state_callback(phone, on_call_state, state);

    if (phone_configure_opus(phone, 1, 8, 16000) != PHONE_STATUS_SUCCESS)
        die(phone);
    if (phone_connect(phone,
                      "tel.t-online.de",
                      "+4965191899543", NULL) != PHONE_STATUS_SUCCESS)
        die(phone);

    int command;
    char uri[128];
    int call_id, level;
    do {
        printf("last call id: %d\n> ", state->last_call_id);
        command = getchar();
        switch (command) {
            case 'c':
                printf("please enter number: ");
                clear_input_buffer();
                if (fgets(uri, sizeof(uri), stdin) == NULL) {
                    fprintf(stderr, "could not read number\n");
                    break;
                }
                uri[strcspn(uri, "\n")] = '\0';
                if (phone_make_call(phone, uri) != PHONE_STATUS_SUCCESS) die(phone);
                break;
            case 'C':
                if (phone_make_call(phone, "+4915123595397") != PHONE_STATUS_SUCCESS) die(phone);
                break;
            case 'a':
                printf("please enter call id: ");
                if (read_int(&call_id) != 0) break;
                if (phone_answer_call(phone, call_id) != PHONE_STATUS_SUCCESS) die(phone);
                break;
            case 'h':
                printf("please enter call id: ");
                if (read_int(&call_id) != 0) break;
                if (phone_hangup_call(phone, call_id) != PHONE_STATUS_SUCCESS) die(phone);
                break;
            case 'H':
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