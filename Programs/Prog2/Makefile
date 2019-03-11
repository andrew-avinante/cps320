webserver: webserver.c eznet.o
	gcc -o myprog webserver.c eznet.o

eznet.o : eznet.c eznet.h
	gcc -c eznet.c eznet.h

clean :
	rm *.o