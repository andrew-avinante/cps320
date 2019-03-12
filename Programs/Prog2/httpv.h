// File: httpv.c
// Submitted by: Andrew Avinante (aavin894)
//
// Build command: gcc -fsanitize=address -g -fno-omit-frame-pointer httpv.c -lbsd -ohttpv

#include <stdio.h>

#define MAX_HEADERS 10

// Returns 1 on success,
// -1 on invalid HTTP request,
// -2 on I/O error,
// -3 on malloc failure
int parseHttp(FILE *in, http_request_t **request);

int cleanupHttp(http_request_t **request);