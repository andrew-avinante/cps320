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
// -4 on invalid verb
// -5 invalid path
// -6 invalid version
int parseHttp(FILE *in, http_request_t **request) 
{
    const int VERB_SIZE = 4;
    const int PATH_SIZE = 256;
    const int VERSION_SIZE = 10;
    http_request_t *req = NULL;
    int rc = -1;
    char *line = NULL;
    size_t len = 0u;
    char *save;
    char *token;
    int i, blankline = 0;


    if((req = calloc(1, sizeof(http_request_t))) == NULL)   //Allocates memory for req
    {
        rc = -3;
        goto cleanup;
    }

    getline(&line, &len, in);  //Gets first line of file

    token = strtok_r(line, " ", &save);     //Parses first line for VERB
    if(token == NULL)
    {
        rc = -2;
        goto cleanup;
    }
    req->verb = malloc(VERB_SIZE);                  //Allocates memory for VERB
    strlcpy(req->verb, token, VERB_SIZE);    //Coppies token to VERB
    req->verb[VERB_SIZE - 1] = 0;            //Adds null terminator
    token = strtok_r(NULL, " ", &save);     //Parses line for PATH
    if(token == NULL)
    {
        rc = -2;
        goto cleanup;
    }
    req->path = malloc(PATH_SIZE);           //Allocates memory for PATH
    strlcpy(req->path, token, PATH_SIZE);    //Coppies token to PATH
    req->path[PATH_SIZE - 1] = 0;              //Adds null terminator
    token = strtok_r(NULL, " ", &save);     //Parses line for VERSION
    if(token == NULL)
    {
        rc = -2;
        goto cleanup;
    }
    req->version = malloc(VERSION_SIZE);                      //Allocates memory for VERSION 
    strlcpy(req->version, token, VERSION_SIZE);      //Coppies token to VERSION

    req->version[VERSION_SIZE - 1] = 0;

    if(strcmp(req->verb, "GET") != 0 && strcmp(req->verb,"POST") != 0)
    {
        rc = -4;
        goto cleanup;
    }
    if(strchr(req->path, '/') != req->path)
    {
        printf("%s", strstr(req->path, ".."));
        if(strstr(req->path, "..") != NULL)
        {
            rc = -8;
        }
        else
        {
            rc = -5;
        }
        goto cleanup;
    }
     if(strstr(req->version, "HTTP") == 0)
    {
        rc = -6;
        goto cleanup;
    }
    
    i = 0;
    while(getline(&line, &len, fstream) > 0 && i < MAX_HEADERS)
    {

        if(line[0] == 13 && line[1] == 10)
        {
            blankline = 1;
            break;
        }
        if(line[0] != 13)
        {
            req->headers[i].name = malloc(len);
            req->headers[i].value = malloc(len);
            strlcpy(req->headers[i].name, strtok_r(line, ":", &save), len);
            strlcpy(req->headers[i].value, strtok_r(NULL, ":", &save), len);
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
     free(line);

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
    while(getline(&line, &len, fstream) > 0 && i < MAX_HEADERS)
    {

        if(line[0] == 13 && line[1] == 10)
        {
            blankline = 1;
            break;
        }
    }
    free(line);
    free(req);  // It's OK to free() a NULL pointer 
    return rc;
}

int generateResponse(int result, http_request_t *request, FILE *out)
{
    char *line = NULL;
    size_t len = 0u;
    ssize_t recd;
    FILE *fstream = NULL;
    // printf("%d\n", result);
    if(result == 1)
    {
        // printf("WHY IN HERE?\n");
        fstream = fopen(&request->path[1], "r+");
        if(fstream == NULL) { result = -5; }
        if(strcmp(request->verb, "POST") == 0) { result = -7; }
    }
    printf("CONTINUE\n");
    switch (result)
    {
        case 1:
            printf("1\n");
            fputs("HTTP/1.1 200 OK\r\n", out);
            fputs("Content-type: text/html\r\n", out);
            fputs("\r\n", out);
            while ((recd = getline(&line, &len, fstream)) > 0) 
            {
                fputs(line, out); 
            }
            break;
        case -1:
            printf("-1\n");
            fputs("HTTP/1.1 400 Bad Request\r\n", out);
            fputs("Content-type: text/plain\r\n", out);
            fputs("\r\n", out);
            fputs("Illegal HTTP stream\r\n", out);
            break;
        case -2:
            printf("-2\n");
            fputs("HTTP/1.1 500 Internal Server Error\r\n", out);
            fputs("Content-type: text/plain\r\n", out);
            fputs("\r\n", out);
            fputs("I/O error while reading request\r\n", out);
            break;
        case -3:
            printf("-3\n");
            fputs("HTTP/1.1 500 Internal Server Error\r\n", out);
            fputs("Content-type: text/plain\r\n", out);
            fputs("\r\n", out);
            fputs("Malloc failure\r\n", out);
            break;
        case -4:
            printf("-4\n");
            fputs("HTTP/1.1 400 Bad Request\r\n", out);
            fputs("Content-type: text/plain\r\n", out);
            fputs("\r\n", out);
            fputs("Invalid verb\r\n", out);
            break;
        case -5:
            printf("-5\n");
            fputs("HTTP/1.1 404 Not Found\r\n", out);
            fputs("Content-type: text/plain\r\n", out);
            fputs("\r\n", out);
            fputs("Resource not found\r\n", out);
            break;
        case -6:
            printf("-6\n");
            fputs("HTTP/1.1 400 Bad Request\r\n", out);
            fputs("Content-type: text/plain\r\n", out);
            fputs("\r\n", out);
            fputs("Invalid HTTP version\r\n", out);
            break;
        case -7:
            printf("-7\n");
            fputs("HTTP/1.1 501 Not Implemented\r\n", out);
            fputs("Content-type: text/plain\r\n", out);
            fputs("\r\n", out);
            fputs("Verb not implemented\r\n", out);
            break;
        case -8:
            printf("-8\n");
            fputs("HTTP/1.1 403 Forbidden\r\n", out);
            fputs("Content-type: text/plain\r\n", out);
            fputs("\r\n", out);
            fputs("File requested is out of root directory\r\n", out);
            break;
        default:
            printf("DEFAULT\n");
            fputs("HTTP/1.1 500 Internal Server Error\r\n", out);
            fputs("Content-type: text/plain\r\n", out);
            fputs("\r\n", out);
            fputs("Something has gone wrong on our end...\r\n", out);
            break;
    }
    if(fstream)
    {
        free(line);
        fclose(fstream);
    }
    return 0;
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