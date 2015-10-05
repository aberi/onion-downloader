CC = gcc
CFLAGS = -g -ansi -pedantic -Wall # -DDEBUG

INCLUDE = -I ./include

client: http_client.o hash.o url.o utils.o request.o file.o
	$(CC) -o $@ http_client.o hash.o url.o utils.o request.o file.o

unit_test: unit_test.o hash.o url.o utils.o request.o file.o 
	$(CC) -c unit_test.c $(CFLAGS) $(INCLUDE)
	$(CC) -o $@ unit_test.o hash.o url.o utils.o request.o file.o 

%.o: src/%.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

clean:
	rm *.o client
