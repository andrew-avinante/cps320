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
#include "httpv.h"

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

        if(buff[0] == 13 && buff[1] == 10)
        {
            blankline = 1;
            break;
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
            printf("EXIT\n");
    if(blankline == 0)
    {
                printf("FAIL\n");
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

int generateResponse(int result, http_request_t *request, FILE *out)
{
    int bufSize = 80;
    char *line = NULL;
    size_t len = 0u;
    ssize_t recd;
    char *flOpen[bufSize];
    flOpen[0] = ".";
    strlcat(flOpen, request->path, bufSize);
    FILE *fstream = fopen(*flOpen, "r+");
    if(fstream == NULL) { result = -5; }
    switch (result)
    {
        case 1:
            fputs("HTTP/1.1 200 OK\r\n", out);
            fputs("Content-type: text/html\r\n", out);
            fputs("\r\n", out);
            while ((recd = getline(&line, &len, fstream)) > 0) 
            {
                fputs(line, out); 
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
            fputs("HTTP/1.1 404 Not Found\r\n", out);
            fputs("Content-type: text/plain\r\n", out);
            fputs("\r\n", out);
            fputs("Resource not found\r\n", out);
            break;
        case -6:
            fprintf(stderr, "** ERROR: Illegal HTTP stream (Missing version).\n");
            break;
        default:
            printf("Unexpected return code %d.\n", result);
    }
}

int cleanupHttp(http_request_t *request)
{
    int rtrn = -1;
    if(request != NULL)
    {
        free(request->verb);
        free(request->path);
        free(request->version);
        for (int i = 0; i < request->num_headers; ++i) {
            free(request->headers[i].name);
            free(request->headers[i].value);
        }
        rtrn = 0;
    }
    free(request);
}