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

dict_t dict = {{{"html", "text/html"}, {"htm", "text/html"}, {"gif", "image/gif"}, {"jpeg", "image/jpeg"}, {"jpg", "image/jpeg"}, {"png", "image/png"}, {"css", "text/css"}, {"txt", "text/plain"}}};

int verifyInput(http_request_t *req)
{
    if(strcmp(req->verb, "GET") != 0 && strcmp(req->verb,"POST") != 0) // test for valid verb
    {
        return -4;
    }
    if(strchr(req->path, '/') != req->path) // test for valid path
    {
        return (strstr(req->path, "..") != NULL) ? -5 : -6; // if path goes outside of root directory then -5 else -6
    }
    if(strstr(req->version, "HTTP") == 0) // test for invalid version
    {
        return -7;
    }
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

// Returns 1 on success,
// -1 on invalid HTTP request,
// -2 on I/O error,
// -3 on malloc failure
// -4 on invalid verb
// -5 outside of root directory
// -6 invalid path
// -7 invalid version
int parseHttp(FILE *in, http_request_t **request) 
{
    http_request_t *req = NULL;
    size_t len = 0u;
    const int VERB_SIZE = 4;
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
    
    if((rc = verifyInput(req)) != -1)
    {
        goto cleanup;
    }

    while(getline(&line, &len, in) > 0 && i < MAX_HEADERS)
    {
        if(strcmp(line, "\r\n") == 0)
        {
            blankline = 1;
            break;
        }
        if(line[0] != 13)
        {
            req->headers[i].name = malloc(len);
            req->headers[i].value = malloc(len);
            strlcpy(req->headers[i].name, strtok_r(line, ":", save), len);
            strlcpy(req->headers[i].value, strtok_r(NULL, ":", save), len);
            i++;
        }
    }
    if(blankline == 0)
    {
        rc = -1;
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
    for (int i = 0; i < req->num_headers; ++i)
    {
        free(req->headers[i].name);
        free(req->headers[i].value);
    }
    while(getline(&line, &len, in) > 0 && i < MAX_HEADERS)
    {
        if(line[0] == 13 && line[1] == 10)
        {
            blankline = 1;
            break;
        }
        i++;
    }

    free(line);
    free(req);  // It's OK to free() a NULL pointer 
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
    if(result == 1)
    {
        fstream = fopen(&request->path[1], "r+");
        if(fstream == NULL) 
        { 
            result = -6; 
        }
        strtok_r(request->path, ".", &fileExt);
        for(int i = 0; i < DICT_SIZE; i++)
        {
            if(strcmp(dict.node[i].key.key, fileExt) == 0)
            {
                snprintf(contentType, CONTENT_SIZE, "Content-type: %s\r\n", dict.node[i].value.value);
                printf("%s\n", dict.node[i].value.value);
            }
        }

        if(strcmp(request->verb, "POST") == 0) 
        {
            result = -8;
        } // -8 verb not implimented
    }
    
    if(result != 1)
    {
        snprintf(contentType, CONTENT_SIZE, "Content-type: %s\r\n", "text/plain");
    }

    switch (result)
    {
        case 1:
            fputs("HTTP/1.1 200 OK\r\n", out);
            fputs(contentType, out);
            fputs("\r\n", out);
            while ((recd = getline(&line, &len, fstream)) > 0) 
            {
                fputs(line, out); 
            }
            break;
        case -1:
            fputs("HTTP/1.1 400 Bad Request\r\n", out);
            fputs(contentType, out);
            fputs("\r\n", out);
            fputs("Illegal HTTP stream\r\n", out);
            break;
        case -2:
            fputs("HTTP/1.1 500 Internal Server Error\r\n", out);
            fputs(contentType, out);
            fputs("\r\n", out);
            fputs("I/O error while reading request\r\n", out);
            break;
        case -3:
            fputs("HTTP/1.1 500 Internal Server Error\r\n", out);
            fputs(contentType, out);
            fputs("\r\n", out);
            fputs("Malloc failure\r\n", out);
            break;
        case -4:
            fputs("HTTP/1.1 400 Bad Request\r\n", out);
            fputs(contentType, out);
            fputs("\r\n", out);
            fputs("Invalid verb\r\n", out);
            break;
        case -5:
            fputs("HTTP/1.1 403 Forbidden\r\n", out);
            fputs(contentType, out);
            fputs("\r\n", out);
            fputs("File requested is out of root directory\r\n", out);
            break;
        case -6:
            fputs("HTTP/1.1 404 Not Found\r\n", out);
            fputs(contentType, out);
            fputs("\r\n", out);
            fputs("Resource not found\r\n", out);
            break;
        case -7:
            fputs("HTTP/1.1 400 Bad Request\r\n", out);
            fputs(contentType, out);
            fputs("\r\n", out);
            fputs("Invalid HTTP version\r\n", out);
            break;
        case -8:
            fputs("HTTP/1.1 501 Not Implemented\r\n", out);
            fputs(contentType, out);
            fputs("\r\n", out);
            fputs("Verb not implemented\r\n", out);
            break;
        default:
            fputs("HTTP/1.1 500 Internal Server Error\r\n", out);
            fputs(contentType, out);
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