webserver: webserver.c eznet.o
	gcc -owebserver webserver.c eznet.o

eznet.o : eznet.c eznet.h
	gcc -c eznet.c

clean :
	-rm *.o