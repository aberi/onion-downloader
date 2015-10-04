CC = gcc
CFLAGS = -g -ansi -pedantic -Wall

INCLUDE = -I ./include

client: http_client.o url.o utils.o hash.o request.o
	$(CC) -o client http_client.o url.o utils.o hash.o request.o

request.o: src/request.c
	$(CC) -c src/request.c $(CFLAGS) $(INCLUDE)

hash.o: src/hash.c
	$(CC) -c src/hash.c $(CFLAGS) $(INCLUDE)	

url.o: src/url.c 
	$(CC) -c src/url.c  $(CFLAGS) $(INCLUDE) 

utils.o: src/utils.c
	$(CC) -c src/utils.c $(CFLAGS) $(INCLUDE) 

http_client.o: src/http_client.c 
	$(CC) -c src/http_client.c $(CFLAGS) $(INCLUDE) 

unit_test: unit_test.o hash.o url.o utils.o request.o
	$(CC) -o unit_test unit_test.o hash.o url.o utils.o request.o

unit_test.o: unit_test.c
	$(CC) $(CFLAGS) $(INCLUDE) -c unit_test.c

clean:
	rm *.o client
