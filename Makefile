CC = gcc
CFLAGS = -g 

INCLUDE = -I ./include

ondl: http_client.o parse.o hash.o url.o utils.o request.o file.o html_tag_list.o queue.o 
	$(CC) -o $@ http_client.o hash.o url.o utils.o request.o file.o parse.o html_tag_list.o queue.o 

parse_test: unit_test.o hash.o url.o utils.o request.o file.o parse.o html_tag_list.o queue.o
	$(CC) -o parse_test unit_test.o hash.o url.o utils.o request.o file.o parse.o html_tag_list.o queue.o

unit_test.o: unit_test.c
	$(CC) -c $(CFLAGS) $(INCLUDE) unit_test.c

%.o: src/%.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

clean:
	rm -r *.o ondl *.html *.pdf  *.htm *.com *.edu *.org *.gov *.net *.io
