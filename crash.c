#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <unistd.h> 

int main()
{
    fork();
    fork();
    fork();
    fork();
    while(true)
    {
        system("eho GET / HTTP/1.0\r\n\r\n | nc -C 10.10.26.50:8080 8080");
        system("echo MUSH\r\n\r\n | nc -C 10.10.26.50:8080");
        system("echo MUSH /stuff HTTP/1.0\r\n\r\n| nc -C 10.10.26.50:8080 8080");
        system("echo GET ../../ HTTP/1.0\r\n\r\n | nc -C 10.10.26.50:8080 8080"); 
        system("echo GET / HTTP/1.0 | nc -C 10.10.26.50:8080 8080");
    }
}