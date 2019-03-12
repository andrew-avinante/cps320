webserver: webserver.c eznet.o httpv.o
	gcc -owebserver webserver.c eznet.o httpv.o -lbsd

eznet.o : eznet.c eznet.h
	gcc -c eznet.c

httpv.o : httpv.c httpv.h
	gcc -c httpv.c

clean :
	-rm *.o