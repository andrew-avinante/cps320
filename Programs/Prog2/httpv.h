// File: httpv.c
// Submitted by: Andrew Avinante (aavin894)
//
// Build command: gcc -fsanitize=address -g -fno-omit-frame-pointer httpv.c -lbsd -ohttpv

#include <stdio.h>

#define MAX_HEADERS 10
#define DICT_SIZE 7

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

typedef struct Key {
    char *key;
} key_t;

typedef struct Value {
    char *value;
} value_t;

typedef struct Node {
    struct key_t key;
    struct value_t value;
} node_t;

typedef struct dict {
    struct node_t node[DICT_SIZE];
} dict_t;

// Returns 1 on success,
// -1 on invalid HTTP request,
// -2 on I/O error,
// -3 on malloc failure
int parseHttp(FILE *in, http_request_t **request);

int generateResponse(int result, http_request_t *request, FILE *out);

// Cleans up memory allocated by request
int cleanupHttp(http_request_t *request);