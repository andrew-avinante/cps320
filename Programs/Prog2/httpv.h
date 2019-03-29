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

// This function verifies the request line of the http request
int verifyInput(http_request_t *req);

// This function parses a portion of the http request and stores it in the variable `reqWord`
int parseRequestLine(char *token, char *reqWord, const int WORD_SIZE);

// Checks for formatting and throws out rest of lines
int eatInput(size_t len, FILE *in);

// Returns 1 on success,
// -1 on invalid HTTP request,
// -2 on I/O error,
// -3 on malloc failure
int parseHttp(FILE *in, http_request_t **request);

int generateResponse(int result, http_request_t *request, FILE *out);

// Cleans up memory allocated by request
int cleanupHttp(http_request_t *request);