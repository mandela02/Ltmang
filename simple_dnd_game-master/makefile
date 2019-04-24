all : server client

server : server.o util.o login.o gameplay.o
	gcc -o server server.o util.o login.o gameplay.o -pthread

client : client.o util.o
	gcc -o client client.o util.o

test : test.o login.o util.o gameplay.o
	gcc -o test test.o login.o util.o gameplay.o

server.o : server.c
	gcc -c server.c

client.o : client.c
	gcc -c client.c 

test.o : test.c 
	gcc -c test.c

login.o : login.c login.h util.o
	gcc -c login.c

gameplay.o : gameplay.c gameplay.h util.o
	gcc -c gameplay.c

util.o : util.c util.h
	gcc -c util.c

clean :
	rm test server client server.o client.o test.o login.o util.o

clean-o :
	rm server.o client.o test.o login.o util.o gameplay.o