#include "hash.h"
#include "url.h"
#include "opt.h"
#include "utils.h"
#include "request.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define LIST_SIZE 16

struct opt options;

static char *names[] = {"Host", "User-Agent", "Connection", NULL};
static char *vals[] = {"www.google.com", "Wget-1.16 (linux-gnu)", "keep-alive", NULL};

int
test_request (void)
{
	char *method = "GET";
	url_t u;
	struct request *req;
	char *url = "http://www.google.com";
	parse_url (url, &u);

	req = create_request (&u, names, vals, method);

	print_request (req);

	put_request_header (req, "Compression", "gzip");
	put_request_header (req, "food", "turkey");

	if (!req) goto fail;

	print_request (req);
	
	return 0;
		
	fail:
		return -1;
}

int
test_hash_table (void)
{
	char *val;
	struct hash_table *ht = hash_table_new (17);

	hash_table_put (ht, "Hello", "gregory");
	hash_table_put (ht, "What's", "your name?");
	hash_table_put (ht, "What's", "your favorite memory of 2011?");

	val = hash_table_get (ht, "Hello");

	if (!val) goto fail;	
	
	val = hash_table_get (ht, "What's");

	if (!val) goto fail;	
	
	val = hash_table_remove (ht, "What's");
	
	if (!val) goto fail;	
	
	val = hash_table_remove (ht, "Hello");
	
	if (!val) goto fail;	
	
	return 0;

	fail:
		return -1;
}

int
main (void)
{
	if (test_hash_table () < 0)
		fprintf (stderr, "Hash table test failed\n");

	if (test_request () < 0)
		fprintf (stderr, "Requests not working properly\n");

	return 0;
}
