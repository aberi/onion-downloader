#ifndef _URL_H_
#define _URL_H_

#include <stdio.h>

#define IS_RELATIVE(path) (*(path) == '/')

typedef enum {
	URL_OK,
	MALFORMED_URL,
	PROTO_NOT_SUP
} url_err_t;

typedef struct url {
	char *full_url; /* Complete URL made up of the components below */

	char *local_filename;
	
	int port;

	char *host; /* Hostname of the URL */
	char *protocol; 
	char *path; /* Resource requested */
	enum {
		PATH_ABS,
		PATH_REL	
	} path_type;
	char *query; /* "?name&value..." */
	int length; 

	/* If the query contains relevant information, it will be here */
	int is_referred; /* Did we get to this URL from a redirection? */
	char *reference; /* URL from which we were redirected to the current one */

	struct url *next; /* We will be using URLs in lists quite often. This field may or may not be used */
} url_t;

char *get_url_directory (struct url *);
int not_outgoing (const char *);
int is_relative (const char *);
int is_absolute (const char *);
int is_outgoing (const char *);
int is_outgoing_http (const char *);
int is_outgoing_https (const char *);
int create_directories (char *);
char *url_file_name(url_t *);
url_err_t parse_url (char *, url_t *);
void print_url (url_t *);

#endif
