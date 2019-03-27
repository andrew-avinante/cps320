/* Echo Server: an example usage of EzNet
 * (c) 2016, Bob Jones University
 */
#define _GNU_SOURCE
#include <bsd/string.h>
#include <limits.h>

#include <stdbool.h>    // For access to C99 "bool" type
#include <stdio.h>      // Standard I/O functions
#include <stdlib.h>     // Other standard library functions
#include <string.h>     // Standard string functions
#include <errno.h>      // Global errno variable

#include <stdarg.h>     // Variadic argument lists (for blog function)
#include <time.h>       // Time/date formatting function (for blog function)

#include <sys/wait.h>

#include <unistd.h>     // Standard system calls
#include <signal.h>     // Signal handling system calls (sigaction(2))

#include "eznet.h"      // Custom networking library

#include "httpv.h"      // Custom HTTP parsing library

#include "utils.h"

// GLOBAL: settings structure instance
struct settings {
    const char *bindhost;   // Hostname/IP address to bind/listen on
    const char *bindport;   // Portnumber (as a string) to bind/listen on
    int socketNum;
} g_settings = {
    .bindhost = "localhost",    // Default: listen only on localhost interface
    .bindport = "8080",         // Default: listen on TCP port 8080
    .socketNum = 5,
};

int connectedCount = 0;

// Signal handler for when children die
void waitchildren(int signum) {
  while (wait3((int *)NULL,
               WNOHANG,
               (struct rusage *)NULL) > 0) {
   connectedCount -= 1;
  }
}

// Parse commandline options and sets g_settings accordingly.
// Returns 0 on success, -1 on false...
int parse_options(int argc, char * const argv[]) {
    int ret = -1; 

    char op;
    while ((op = getopt(argc, argv, "h:p:r:w:")) > -1) {
        switch (op) {
            case 'h':
                g_settings.bindhost = optarg;
                break;
            case 'p':
                g_settings.bindport = optarg;
                break;
            case 'r':
                ret = chdir(optarg);
                if(ret < 0) { goto cleanup; }
            case 'w':
                g_settings.socketNum = atoi(optarg);
                break;
            default:
                // Unexpected argument--abort parsing
                goto cleanup;
        }
    }

    ret = 0;
cleanup:
    return ret;
}

// GLOBAL: flag indicating when to shut down server
volatile bool server_running = false;

// SIGINT handler that detects Ctrl-C and sets the "stop serving" flag
void sigint_handler(int signum) {
    blog("Ctrl-C (SIGINT) detected; shutting down...");
    server_running = false;
}

// Connection handling logic: reads/echos lines of text until error/EOF,
// then tears down connection.
void handle_client(struct client_info *client) {
    FILE *stream = NULL;

    // Wrap the socket file descriptor in a read/write FILE stream
    // so we can use tasty stdio functions like getline(3)
    // [dup(2) the file descriptor so that we don't double-close;
    // fclose(3) will close the underlying file descriptor,
    // and so will destroy_client()]
    if ((stream = fdopen(dup(client->fd), "r+"))== NULL) {
        perror("unable to wrap socket");
        goto cleanup;
    }

    http_request_t *request = NULL;
    int result = 0;

    // Echo all lines
    char *line = NULL;
    size_t len = 0u;
    ssize_t recd;
    result = parseHttp(stream, &request);
    generateResponse(result, request, stream);
    cleanupHttp(request);
    // while ((recd = getline(&line, &len, stream)) > 0) {
    //     printf("\tReceived %zd byte line; echoing...\n", recd);
        
    //     fputs(line, stream); 
    // }

cleanup:
    // Shutdown this client
    if (stream) fclose(stream);
    destroy_client_info(client);
    free(line);
    printf("\tSession ended.\n");
}

int main(int argc, char **argv) {
    int ret = 1;
    int child;

    // signal handler for when child dies
    signal (SIGCHLD, waitchildren);

    // Network server/client context
    int server_sock = -1;

    // Handle our options
    if (parse_options(argc, argv)) {
        printf("usage: %s [-p PORT] [-h HOSTNAME/IP] [-r ROOT DIRECTORY]\n", argv[0]);
        goto cleanup;
    }

    // Install signal handler for SIGINT
    struct sigaction sa_int = {
        .sa_handler = sigint_handler
    };
    if (sigaction(SIGINT, &sa_int, NULL)) {
        LOG_ERROR("sigaction(SIGINT, ...) -> '%s'", strerror(errno));
        goto cleanup;
    }

    // Start listening on a given port number
    server_sock = create_tcp_server(g_settings.bindhost, g_settings.bindport);
    if (server_sock < 0) {
        perror("unable to create socket");
        goto cleanup;
    }
    blog("Bound and listening on %s:%s", g_settings.bindhost, g_settings.bindport);

    server_running = true;
    while (server_running) {
        struct client_info client;

        // Wait for a connection on that socket
        if (wait_for_client(server_sock, &client)) {
            // Check to make sure our "failure" wasn't due to
            // a signal interrupting our accept(2) call; if
            // it was  "real" error, report it, but keep serving.
            if (errno != EINTR) { perror("unable to accept connection"); }
        } else {
            if(connectedCount < g_settings.socketNum)
            child = fork();
            if(child == 0)
            {
                blog("connection from %s:%d with %d client(s) connected", client.ip, client.port, connectedCount);
                handle_client(&client); // Client gets cleaned up in here
                printf("crap");
            }
            else if(child < 0)
            {
                // perror("Failed to fork child\n");
                connectedCount += 1;
            }
        }
    }
    ret = 0;

cleanup:
    if (server_sock >= 0) close(server_sock);
    return ret;
}

