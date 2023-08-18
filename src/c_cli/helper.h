//
// Created by Oliver Epper on 16.03.23.
//

#ifndef PHONE_HELPER_H
#define PHONE_HELPER_H

#include <phone.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <float.h>

void clear_input_buffer(void) {
    int c;
    while ((c = getchar() != '\n') && c != EOF) {}
}

int read_string(char *out, int size) {
    if (fgets(out, size, stdin) == NULL) {
        fprintf(stderr, "could not read from stdin");
        return 1;
    }

    out[strcspn(out, "\n")] = '\0';
    return 0;
}

int read_int(int *in) {
    char input[11];
    long value;
    char *endptr;

    if (read_string(input, sizeof(input)) != 0) return 1;

    value = strtol(input, &endptr, sizeof(input));
    if (*input == '\0' || *endptr != '\0') {
        fprintf(stderr, "invalid input.\n");
        return  1;
    }

    if (value <= INT_MAX && value >= INT_MIN) {
        *in = (int)value;
    } else {
        fprintf(stderr, "value out of range.\n");
        return  1;
    }
    return 0;
}

int read_float(float *in) {
    char input[11];
    double value;
    char *endptr;

    if (read_string(input, sizeof(input)) != 0) return 1;

    value = strtod(input, &endptr);

    if (*input == '\0' || *endptr != '\0') {
        fprintf(stderr, "invalid input.\n");
        return 1;
    }

    if (value <= FLT_MAX && value >= -FLT_MAX) {
        *in = (float)value;
    } else {
        fprintf(stderr, "value out of range.\n");
        return 1;
    }

    return 0;
}

void die(phone_t instance) {
    phone_destroy(instance);
    fprintf(stderr, "%s\n", phone_last_error());
    exit(EXIT_FAILURE);
}

#endif //PHONE_HELPER_H
