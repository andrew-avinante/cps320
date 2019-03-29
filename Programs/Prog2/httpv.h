// File: httpv.c
// Submitted by: Andrew Avinante (aavin894)
//
// Build command: gcc -fsanitize=address -g -fno-omit-frame-pointer httpv.c -lbsd -ohttpv

#include <stdio.h>

#define MAX_HEADERS 10

typedef struct http_header {
    char *name;
    char *value;
} http_header_t;

typedef struct http_request {
    char *verb;
    char *path;
    char *version;
    int num_headers;
    http_header_t headers[MAX_HEADERS];
} http_request_t;

int verifyInput(http_request_t *req);

int parseRequestLine(char *line, char **reqWord, char **save, size_t len);

// Returns 1 on success,
// -1 on invalid HTTP request,
// -2 on I/O error,
// -3 on malloc failure
int parseHttp(FILE *in, http_request_t **request);

int generateResponse(int result, http_request_t *request, FILE *out);

// Cleans up memory allocated by request
int cleanupHttp(http_request_t *request);