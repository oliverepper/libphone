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

void clear_input_buffer(void);

int read_string(char *out, int size);

int read_int(int *in);

int read_float(float *in);

void die(phone_t instance);

#endif //PHONE_HELPER_H
