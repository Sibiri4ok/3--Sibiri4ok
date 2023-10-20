#pragma once

#include <stdio.h>

#define MAX_MESSAGE_LEN 256

int read_message(FILE *stream, void *buf);
int write_message(FILE* stream, const void *buf, size_t nbyte);
