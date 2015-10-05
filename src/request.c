#include "types.h"
#include "utils.h"
#include "url.h"
#include "hash.h"
#include "request.h"
#include "opt.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define HT_SIZE 31 /* sufficiently large prime */

#define APPEND(p, str, len) strncpy (p, str, len); \
							p += len

struct opt options;  /* global options variable */

struct request *
make_request (url_t *url, struct hash_table *table, char *method)
{
	char *ptr;
	struct request *req = malloc (sizeof (struct request));
	
	int content_len = 0;
	int i;

	req->position = 0;
	req->url = url;
	req->method = strdup (method);
	req->http_version = strdup ("HTTP/1.0");
	req->headers = hash_table_copy (table);

	content_len += strlen (method) + 1 + strlen (url->path) + 1 + strlen (req->http_version) + 2;	

	for (i = 0; i < table->size; i++)
	{
		if (table->table[i].state == OCCUPIED)
		{
			char *key = table->table[i].key;
			char *value = hash_table_get (table, key);

			content_len += strlen (key) + 2 + strlen (value) + 2;
		}	
	}

	content_len += 2; /* Final \r\n */
	
	req->content = malloc (content_len + 1);
	if (!req->content)
		return NULL;

	req->content_len = content_len;
	ptr = req->content;

	APPEND(ptr, method, strlen (method)); *ptr++ = ' ';
	APPEND(ptr, url->path, strlen (url->path)); *ptr++ = ' ';
	APPEND(ptr, req->http_version, strlen (req->http_version));
	APPEND(ptr, "\r\n", 2);

	for (i = 0; i < table->size; i++)
	{
		if (table->table[i].state == OCCUPIED)
		{
			char *key = table->table[i].key;
			char *value = hash_table_get (table, key);	

			APPEND(ptr, key, strlen (key)); APPEND(ptr, ": ", 2);
			APPEND(ptr, value, strlen (value)); APPEND(ptr, "\r\n", 2);
		}
	}

	APPEND(ptr, "\r\n", 2);

	#ifdef DEBUG	
	fprintf (stderr, "%s\n", req->content);
	#endif
		
	return req;
	
}

struct request *
create_request (url_t *url, char **names, char **values, char *method)
{
	char **names_temp, **values_temp;
	char *ptr; /* Pointer to current position in the request content */
	struct request *req = malloc (sizeof (struct request));	
	struct hash_table *table;
	int content_len = 0;
	int i;

	req->position = 0;
	req->url = url;
	req->method = strdup (method);
	req->http_version = strdup ("HTTP/1.0");

	/* Create a hash table based on the names and values given */	

	req->headers = hash_table_new (HT_SIZE);

	names_temp = names;
	values_temp = values;

	if (names && values)
	{
		do
		{
			hash_table_put (req->headers, *names_temp, *values_temp);
			values_temp++;
			names_temp++;
		}
		while (*names_temp); /* Assume the two arrays are of the same length. We want some errors to occur if they
						 	* exist rather than pretend as if they aren't there */
	}

	content_len += strlen (method) + 1 + strlen (url->path) + 1 + strlen (req->http_version) + 2;	

	table = req->headers;

	for (i = 0; i < table->size; i++)
	{
		if (table->table[i].state == OCCUPIED)
		{
			char *key = table->table[i].key;
			char *value = hash_table_get (table, key);

			content_len += strlen (key) + 2 + strlen (value) + 2;
		}	
	}

	content_len += 2; /* Final \r\n */
	
	req->content = malloc (content_len + 1);
	if (!req->content)
		return NULL;

	req->content_len = content_len;
	ptr = req->content;

	APPEND(ptr, method, strlen (method)); *ptr++ = ' ';
	APPEND(ptr, url->path, strlen (url->path)); *ptr++ = ' ';
	APPEND(ptr, req->http_version, strlen (req->http_version));
	APPEND(ptr, "\r\n", 2);

	for (i = 0; i < table->size; i++)
	{
		if (table->table[i].state == OCCUPIED)
		{
			char *key = table->table[i].key;
			char *value = hash_table_get (table, key);	

			APPEND(ptr, key, strlen (key)); APPEND(ptr, ": ", 2);
			APPEND(ptr, value, strlen (value)); APPEND(ptr, "\r\n", 2);
		}
	}

	APPEND(ptr, "\r\n", 2);
		
	return req;
}

void 
print_request (struct request *req)
{
	printf ("%s", req->content);
}

int 
write_to_socket (int sock, char *buf, int len) 
{
	int n_written = 0;
	char *bufp = buf;
	
	while (n_written < len) 
	{
		int n_bytes;
		if ((n_bytes = write (sock, bufp, len - n_written)) < 0 || (n_bytes == 0 && n_written < len)) 
		{
			fprintf (stderr, "Couldn't completely write to socket. Wrote %d bytes of %d\n", n_written, len);
			exit (1);
		}
		n_written += n_bytes;
	}	
	return 0;
}

#define SKIP_WS(p) if (p) while (*p == ' ' || *p == '\r' || *p == '\n') p++

#ifndef IS_DIGIT
	#define  IS_DIGIT(d)(((d - '0') >= 0) && ((d - '0') <= 9))
#endif

#ifndef xstrdup
	#define xstrdup(s) (s) ? strdup (s) : NULL
#endif
	
int
parse_response (struct content *resp_content, struct response *resp)
{
	char *p = resp_content->body;
	char *body = resp_content->body;

	/* The response should begin with HTTP/\d.\d \d\d\d, that is, HTTP version and status code */
	if (strncmp (p, "HTTP", 4) != 0) 
	{
		fprintf (stderr, "Protocol does not appear to be HTTP\n");
		return -1;
	}
	resp->protocol = xstrdup ("HTTP");
	p += 4;
	if (p[0] != '/' || !IS_DIGIT(p[1]) || p[2] != '.' || !IS_DIGIT(p[3])) 
	{
		fprintf (stderr, "HTTP Version not found\n");
		return -1;	
	}
	p += 4;
	SKIP_WS(p);

	/* Get the status code */	
	
	if (!IS_DIGIT(p[0]) || !IS_DIGIT(p[1]) || !IS_DIGIT(p[2])) 
	{
		fprintf (stderr, "Couldn't find the status code in the response body\n");	
		return -1;
	}
	resp->status = 100 * (p[0] - '0') + 10 * (p[1] - '0') + p[2] - '0';
	fprintf (stderr, "Found status code %d\n", resp->status);
	
	/* Find the next line. This is where the headers should begin */

	resp->headers = hash_table_new (31);

	/* Make sure we are not at the end of the header */

	while ((p = strstr (p, "\r\n")))
	{
		char *name, *name_end;
		char *value, *value_end;
		if (strstr (p + 2, "\r\n") == p + 2)
		{
			p += 4;	
			break;
		}
	
		p = p + 2;
	
		name_end = strchr (p, ':');	
		name = malloc (name_end - p + 1);
		strncpy (name, p, name_end - p);
	
		p = name_end + 2;
		value_end = strstr (p, "\r\n");
		value = malloc (value_end - p + 1);
		strncpy (value, p, value_end - p);
	
		hash_table_put (resp->headers, name, value);
	}	

	resp->header_body = calloc (1, p - body + 1);
	strncpy (resp->header_body, body, p - body);

	return 0;
}

void
print_content (const struct content *cont)
{
	int n_printed = 0;
	int l;
	char *cur = cont->body;

	while (n_printed < cont->len)
	{
		l = strlen (cur);
		printf ("%s\n", cur);
		n_printed += (l + 1);
		cur += (l + 1);
	}
}

/* See how easy it is... oh my goodness */
int
add_header_to_request (struct request *req, char *name, char *value)
{
	return hash_table_put (req->headers, name, value);
}

/* Remove null characters that come before LENGTH in the string.
 * Assume str + length is in a valid portion of memory. */
char *
remove_null_characters (char *str, int length)
{
	int i;
	for (i = 0; i < length; i++)
	{
		if (str[i] == '\0')
			memmove (&str[i], &str[i] + 1, (str + length) - (&str[i] + 1));
	}
	return str;
}

struct content*
read_response (int sock, char *buf, int len)
{
	/* Read the header of the response */
	
	char *response_body = NULL;
	struct content *ret;
	int n_bytes, n_read = 0;
	int is_content = 0;
	
	memset (buf, 0, len);
	while ((n_bytes = read (sock, buf, len)) > 0)
	{
		int current_length = n_read;
		char *brk;
		if (is_content)
			write (options.output_fd, buf, n_bytes);	
		else
		{
			brk = strstr (buf, "\r\n\r\n");
			if (brk)
			{
				is_content = 1;	
				write (options.output_fd, brk + 4, (buf + len) - brk - 4);
			}
		}
		n_read += n_bytes;
		response_body = realloc (response_body, n_read);
		if (!response_body)
		{
			perror ("Couldn't allocate more memory for response body");
			exit (1);
		}

		/* remove_null_characters (buf, n_bytes); */
		strncpy (response_body + current_length, buf, n_bytes);

		/*fprintf (stderr, "%s\n", buf);	 */
	
		response_body[n_read] = '\0';
		memset (buf, 0, len);
	}

	/* Determine whether we want to read the content */
	
	/* Read the content and write to the proper output stream if applicable */
	#ifdef DEBUG
	fprintf (stderr, "Length of response: %d\n", n_read);
	#endif
	
	ret = malloc (sizeof (struct content));
	ret->body = response_body;
	ret->len = n_read;
	return ret;
}

/* needs serious testing */
int
put_request_header (struct request *req, char *name, char *value)
{
	/* Change the content of the actual request. An alternative would be to only
		create a new content string when a request is being sent */
	if (hash_table_get (req->headers, name) == NULL)
	{	
		char *ptr;
		int new_length = req->content_len + strlen (name) + 2 + strlen (value) + 1;
		req->content = realloc (req->content, new_length);
		hash_table_put (req->headers, name, value);

		ptr = req->content + req->content_len - 2;
		memset (ptr, 0, new_length - req->content_len);
		APPEND(ptr, name, strlen (name)); APPEND(ptr, ": ", 2);
		APPEND(ptr, value, strlen (value)); APPEND(ptr, "\r\n", 2); APPEND(ptr, "\r\n", 2);
		req->content_len = new_length - 1;
	}	
	else
	{
		hash_table_put (req->headers, name, value);	
	
		printf ("Key-value pair (%s, %s)\n", name, hash_table_get (req->headers, name));
		req = make_request (req->url, req->headers, req->method);
	}

	return 0;
}

int send_request (int sock, struct request *req)
{
	#ifdef DEBUG
	fprintf (stderr, "%s\n", req->content);	
	#endif
	return write_to_socket (sock, req->content, req->content_len);		
}

