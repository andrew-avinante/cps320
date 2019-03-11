// File: httpv.c
// Submitted by: Andrew Avinante (aavin894)
//
// Build command: gcc -fsanitize=address -g -fno-omit-frame-pointer httpv.c -lbsd -ohttpv
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <bsd/string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>


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


FILE *parseArgs(int argc, char **argv)
{
    FILE *in;
    if(argc < 2)
    {
        printf("Reading from stdin...\n");
        return stdin;
    }
    (in = fopen(argv[1], "r")) != NULL ? printf("Reading from %s...\n", argv[1]) :  printf("Unable to open %s\n", argv[1]);
    return in;
}

// Returns 1 on success,
// -1 on invalid HTTP request,
// -2 on I/O error,
// -3 on malloc failure
int parseHttp(FILE *in, http_request_t **request) 
{
    http_request_t *req = NULL;
    int rc = -1;
    int buffSize = 256;
    int verbSize = 5;
    int versionSize = 10;
    char *save;
    char *buff = malloc(buffSize);
    char *token;
    int i, blankline = 0;


    if((req = calloc(4, sizeof(http_request_t))) == NULL)   //Allocates memory for req
    {
        rc = -3;
        goto cleanup;
    }

    fgets(buff, buffSize, in);  //Gets first line of file

    token = strtok_r(buff, " ", &save);     //Parses first line for VERB
    if(token == NULL)
    {
        rc = -2;
        goto cleanup;
    }
    req->verb = malloc(verbSize);                  //Allocates memory for VERB
    strlcpy(req->verb, token, verbSize);    //Coppies token to VERB
    req->verb[verbSize - 1] = 0;            //Adds null terminator
    token = strtok_r(NULL, " ", &save);     //Parses line for PATH
    if(token == NULL)
    {
        rc = -2;
        goto cleanup;
    }
    req->path = malloc(buffSize);           //Allocates memory for PATH
    strlcpy(req->path, token, buffSize);    //Coppies token to PATH
    req->path[buffSize-1] = 0;              //Adds null terminator
    token = strtok_r(NULL, " ", &save);     //Parses line for VERSION
    if(token == NULL)
    {
        rc = -2;
        goto cleanup;
    }
    req->version = malloc(versionSize);                      //Allocates memory for VERSION 
    strlcpy(req->version, token, versionSize);      //Coppies token to VERSION

    req->version[versionSize - 1] = 0;

    if(strcmp(req->verb, "GET") != 0 && strcmp(req->verb,"POST") != 0)
    {
        rc = -4;
        goto cleanup;
    }
     if(strchr(req->path, '/') != req->path)
    {
        rc = -5;
        goto cleanup;
    }
     if(strstr(req->version, "HTTP") == 0)
    {
        rc = -6;
        goto cleanup;
    }
    
    i = 0;
    while(fgets(buff, buffSize, in) && i < MAX_HEADERS)
    {
        if(buff[0] == 13)
        {
           
            blankline = 1;
        }
        if(buff[0] != 13)
        {
            req->headers[i].name = malloc(buffSize);
            req->headers[i].value = malloc(buffSize);
            strlcpy(req->headers[i].name, strtok_r(buff, ":", &save), buffSize);
            strlcpy(req->headers[i].value, strtok_r(NULL, ":", &save), buffSize);
            i++;
        }
    }
    if(blankline == 0)
    {
        rc = -2;
        req->num_headers = i;
        goto cleanup;
    }
    req->num_headers = i;
     free(buff);

    *request = req;
    
    rc = 1;
    return rc;

cleanup:
    if(req != NULL)
    {
        free(req->verb);
        free(req->path);
        free(req->version);
    }
    for (int i = 0; i < req->num_headers; ++i) {
        
        free(req->headers[i].name);
        free(req->headers[i].value);
    }
    free(buff);
    free(req);  // It's OK to free() a NULL pointer 
    return rc;

}

int main(int argc, char **argv)
{
    FILE *f = parseArgs(argc, argv);
    if (f == NULL) {
        exit(1);
    }

    http_request_t *request = NULL;
    int result = 0;

    result = parseHttp(f, &request);

    switch (result) {
    case 1:
        printf("Verb: %s\n", request->verb);
        printf("Path: %s\n", request->path);
        printf("Version: %s\n", request->version);
        printf("\n%d header(s):\n", request->num_headers);

        for (int i = 0; i < request->num_headers; ++i) {
             printf("* %s is %s\n", request->headers[i].name, request->headers[i].value);
        }
        break;
    case -1:
        fprintf(stderr, "** ERROR: Illegal HTTP stream.\n");
        break;
    case -2:
        fprintf(stderr, "** ERROR: I/O error while reading request.\n");
        break;
    case -3:
        fprintf(stderr, "** ERROR: malloc failure.\n");
        break;
    case -4:
        fprintf(stderr, "** ERROR: Illegal HTTP stream (Invalid verb).\n");
        break;
    case -5:
        fprintf(stderr, "** ERROR: Illegal HTTP stream (Invalid path).\n");
        break;
    case -6:
        fprintf(stderr, "** ERROR: Illegal HTTP stream (Missing version).\n");
        break;
    default:
        printf("Unexpected return code %d.\n", result);
    }
    if(request != NULL)
    {
        free(request->verb);
        free(request->path);
        free(request->version);
        for (int i = 0; i < request->num_headers; ++i) {
             free(request->headers[i].name);
             free(request->headers[i].value);
        }
    }
    free(request);
    fclose(f);
}