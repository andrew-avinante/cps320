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
#include <unistd.h>
#include "httpv.h"

dict_t contentDict = {{{"html", "text/html"}, {"htm", "text/html"}, {"gif", "image/gif"}, {"jpeg", "image/jpeg"}, {"jpg", "image/jpeg"}, {"png", "image/png"}, {"css", "text/css"}, {"txt", "text/plain"}}};
dict_t errorDict = {{{-1, "400"}, {-2, "500"}, {-3, "500"}, {-4, "400"}, {-5, "403"}, {-6, "404"}, {-7, "400"}, {-8, "501"}}};

int verifyInput(http_request_t *req)
{        
    if(strcmp(req->verb, "GET") != 0 && strcmp(req->verb,"POST") != 0) // test for valid verb
    {
        return -4;
    }
    if(strchr(req->path, '/') != req->path || access(&req->path[1], F_OK) == -1) // test for valid path
    {
        return (strstr(req->path, "..") != NULL) ? -5 : -6; // if path goes outside of root directory then -5 else -6
    }
    if(strstr(req->version, "HTTP") == 0) // test for invalid version
    {
        return -7;
    }
    if(strcmp(req->verb, "POST") == 0) 
    {
        return -8;
    }
    printf("daqrap\n");
    return -1;
}

int parseRequestLine(char *line, char *reqWord, char **save, const int WORD_SIZE)
{
    char *token = strtok_r(line, " ", save);
    if(token == NULL)
    {
        return -2;
    }
    strlcpy(reqWord, token, WORD_SIZE);    //Coppies token to VERB
    reqWord[WORD_SIZE - 1] = 0;            //Adds null terminator
    return -1;
}

int eatInput(char *line, size_t len, FILE *in)
{
    while(getline(&line, &len, in) > 0)
    {
        if(strcmp(line, "\r\n") == 0)
        {
            return 1;
        }
    }
    return 0;
}

// Returns 1 on success,
// -1 on invalid HTTP request,
// -2 on I/O error,
// -3 on malloc failure
// -4 on invalid verb
// -5 outside of root directory
// -6 invalid path
// -7 invalid version
// -8 verb not implimented
int parseHttp(FILE *in, http_request_t **request) 
{
    http_request_t *req = NULL;
    size_t len = 0u;
    const int VERB_SIZE = 5;
    const int PATH_SIZE = 256;
    const int VERSION_SIZE = 10;
    int i = 0, blankline = 0;
    int rc = -1;
    char *line = NULL;
    char **save;

    if((req = calloc(1, sizeof(http_request_t))) == NULL)   //Allocates memory for req
    {
        rc = -3;
        goto cleanup;
    }
    
    getline(&line, &len, in);  //Gets first line of file
    
    req->verb = malloc(VERB_SIZE); 
    req->path = malloc(PATH_SIZE); 
    req->version = malloc(VERSION_SIZE);
    if((rc = parseRequestLine(line, req->verb, save, VERB_SIZE)) != -1) goto cleanup;
    if((rc = parseRequestLine(NULL, req->path, save, PATH_SIZE)) != -1) goto cleanup;
    if((rc = parseRequestLine(NULL, req->version, save, VERSION_SIZE)) != -1) goto cleanup;
    
    if((rc = verifyInput(req)) != -1) goto cleanup;

    if(eatInput(line, len, in) == 0) goto cleanup;

    req->num_headers = i;
    free(line);

    *request = req;

    rc = 1;
    return rc;

cleanup:
    cleanupHttp(req);
    eatInput(line, len, in);

    free(line);
    return rc;
}

int generateResponse(int result, http_request_t *request, FILE *out)
{
    char *line = NULL;
    const int CONTENT_SIZE = 50;
    size_t len = 0u;
    ssize_t recd;
    FILE *fstream = NULL;
    char *fileExt;
    char contentType[CONTENT_SIZE];

    switch (result)
    {
        case 1:
            fstream = fopen(&request->path[1], "r+");
            strtok_r(request->path, ".", &fileExt);
            for(int i = 0; i < DICT_SIZE; i++)
            {
                if(strcmp(contentDict.node[i].key, fileExt) == 0)
                {
                    snprintf(contentType, CONTENT_SIZE, "Content-type: %s\r\n", contentDict.node[i].value.value);
                    printf("%s\n", contentDict.node[i].value);
                }
            }

            fputs("HTTP/1.1 200 OK\r\n", out);
            fputs(contentType, out);
            fputs("\r\n", out);

            while ((recd = getline(&line, &len, fstream)) > 0) 
            {
                fputs(line, out); 
            }
            break;
        case -1:
            fputs("HTTP/1.1 400 Bad Request\r\nContent-type: text/plain\r\n\r\nIllegal HTTP stream\r\n", out);
            break;
        case -2:
            fputs("HTTP/1.1 500 Internal Server Error\r\nContent-type: text/plain\r\n\r\nI/O error while reading request\r\n", out);
            break;
        case -3:
            fputs("HTTP/1.1 500 Internal Server Error\r\nContent-type: text/plain\r\n\r\nMalloc failure\r\n", out);
            break;
        case -4:
            fputs("HTTP/1.1 400 Bad Request\r\nContent-type: text/plain\r\n\r\nInvalid verb\r\n", out);
            break;
        case -5:
            fputs("HTTP/1.1 403 Forbidden\r\nContent-type: text/plain\r\n\r\nFile requested is out of root directory\r\n", out);
            break;
        case -6:
            fputs("HTTP/1.1 404 Not Found\r\nContent-type: text/plain\r\n\r\nResource not found\r\n", out);
            break;
        case -7:
            fputs("HTTP/1.1 400 Bad Request\r\nContent-type: text/plain\r\n\r\nInvalid HTTP version\r\n", out);
            break;
        case -8:
            fputs("HTTP/1.1 501 Not Implemented\r\nContent-type: text/plain\r\n\r\nVerb not implemented\r\n", out);
            break;
        default:
            fputs("HTTP/1.1 500 Internal Server Error\r\nContent-type: text/plain\r\n\r\nSomething has gone wrong on our end...\r\n", out);
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