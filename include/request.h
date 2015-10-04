#ifndef _REQUEST_
#define _REQUEST_

#include "url.h"

#define NO_RESPONSE -1
#define HTTP_OK 200
#define HTTP_MOVED_PERM 301
#define HTTP_FOUND 302
#define HTTP_BAD_REQ 400
#define HTTP_NOT_FOUND 404

struct response
{
	char *protocol;
	struct hash_table *headers;
	char *content; /* Actual response (this is what we actually read) */
	int content_len;
	int position;
	url_t *url;
	int status;	 /* Server response code */
	char *method;
};

struct request
{
	struct hash_table *headers;	
	char *content; /* Actual request string (this is what we really send) */
	int content_len;
	int position; /* Current position of the content while being read */
	url_t *url; /* Where the request is going */
	char *method;
	char *http_version;
};

/* Null-terminated list of names and their corresponding values for the headers */
struct request *create_request (url_t *url, char **names, char **values, char *method);

/* Read the response from the server and parse it */
int parse_response (char *body, struct response *resp);

char *read_response (int sock, char *buf, int len);

int put_request_header (struct request *, char *, char *);
int send_request (int sock, struct request *req);
void print_request (struct request *);

#endif
