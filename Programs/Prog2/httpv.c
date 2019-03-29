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
#include "utils.h"

// Dictionary for content type
dict_t contentDict = {{"html", "text/html"}, {"htm", "text/html"}, {"gif", "image/gif"}, {"jpeg", "image/jpeg"}, {"jpg", "image/jpeg"}, {"png", "image/png"}, {"css", "text/css"}, {"txt", "text/plain"}};

// Map for errors
dict_t errorMap = {{"400 Bad Request\r\n", "Illegal HTTP stream\r\n"}, {"500 Internal Server Error\r\n", "I/O error while reading request\r\n"}, {"500 Internal Server Error\r\n", "Malloc failure\r\n"}, {"400 Bad Request\r\n", "Invalid verb\r\n"}, {"403 Forbidden\r\n", "File requested is out of root directory\r\n"}, {"404 Not Found\r\n", "Resource not found\r\n"}, {"400 Bad Request\r\n", "Invalid HTTP version\r\n"}, {"501 Not Implemented\r\n", "Verb not implemented\r\n"}};

// This function verifies the request line of the http request
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
    if(strcmp(req->verb, "POST") == 0) 
    {
        return -8;
    }
    return -1;
}

// This function parses a portion of the http request and stores it in the variable `reqWord`
int parseRequestLine(char *line, char *reqWord, char **save, const int WORD_SIZE)
{
    char parse[250] = line;
    // strlcpy(parse, line, strlen(line));
    char *rest = parse;

    char *token = strtok_r(rest, " ", &rest);
    if(token == NULL)
    {
        return -2;
    }
    strlcpy(reqWord, token, WORD_SIZE);    //Coppies token to VERB
    reqWord[WORD_SIZE - 1] = 0;            //Adds null terminator
    return -1;
}

// This function eats excess input
int eatInput(size_t len, FILE *in)
{
    char *line = NULL;
    alarm(5);
    while(getline(&line, &len, in) > 0)
    {
        alarm(0);
        if(strcmp(line, "\r\n") == 0)
        {
            free(line);
            return 1;
        }
    }
    alarm(0);
    free(line);
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
// -8 verb not implimented
// This function parses the http request
int parseHttp(FILE *in, http_request_t **request) 
{
    http_request_t *req = NULL;
    size_t len = 0u;
    const int VERB_SIZE = 5;
    const int PATH_SIZE = 256;
    const int VERSION_SIZE = 10;
    int blankline = 0;
    int rc = -1;
    char *line = NULL;
    char **save;

    if((req = calloc(1, sizeof(http_request_t))) == NULL)   //Allocates memory for req
    {
        rc = -3;
        goto cleanup;
    }
    
    alarm(5);
    if(getline(&line, &len, in) <= 0)  //Gets first line of file
    {
        alarm(0);
        rc = -2;
        goto cleanup;
    }
    alarm(0);
    
    req->verb = malloc(VERB_SIZE); 
    req->path = malloc(PATH_SIZE); 
    req->version = malloc(VERSION_SIZE);
    if((rc = parseRequestLine(line, req->verb, save, VERB_SIZE)) != -1) goto cleanup;
    if((rc = parseRequestLine(NULL, req->path, save, PATH_SIZE)) != -1) goto cleanup;
    if((rc = parseRequestLine(NULL, req->version, save, VERSION_SIZE)) != -1) goto cleanup;
    
    if((rc = verifyInput(req)) != -1) goto cleanup;

    if((rc = eatInput(len, in)) != 1) goto cleanup;

    free(line);

    *request = req;

    return rc;

cleanup:
    cleanupHttp(req);
    eatInput(len, in);

    free(line);
    return rc;
}

// This function generates and sends the response to the client
int generateResponse(int result, http_request_t *request, FILE *out)
{
    char *line = NULL;
    const int CONTENT_SIZE = 150;
    size_t len = 0u;
    FILE *fstream = NULL;
    char *fileExt;
    char contentType[CONTENT_SIZE];
    char errOutput[CONTENT_SIZE];

        if(result == 1)
        {
            fstream = fopen(&request->path[1], "r+");
            fileExt = &strrchr(request->path, '.')[1];
            for(int i = 0; i < DICT_SIZE; i++)
            {
                if(strcmp(contentDict[i].key, fileExt) == 0)
                {
                    snprintf(contentType, CONTENT_SIZE, "Content-type: %s\r\n", contentDict[i].value);
                }
            }
            fputs("HTTP/1.1 200 OK\r\n", out);
            fputs(contentType, out);
            fputs("\r\n", out);
            if(strstr(contentType, "text"))
            {
                alarm(5);
                while (getline(&line, &len, fstream) > 0) 
                {
                    alarm(0);
                    fputs(line, out); 
                }
                alarm(0);
            }
            else
            {
                line = malloc(1);
                while (fread(line, 1, 1, fstream)) 
                {
                    fwrite(line, 1, 1, out); 
                }
            }
        }
        else
        {
            snprintf(errOutput, CONTENT_SIZE, "HTTP/1.1 %sContent-type: text/plain\r\n\r\n%s", errorMap[abs(result + 1)].key, errorMap[abs(result + 1)].value);
            fputs(errOutput, out);
        }
    if(fstream)
    {
        free(line);
        fclose(fstream);
    }
    return 0;
}

// This function cleans allocated memory
int cleanupHttp(http_request_t *request)
{
    int rtrn = -1;
    if(request != NULL)
    {
        free(request->verb);
        free(request->path);
        free(request->version);
        rtrn = 0;
    }
    free(request);
}