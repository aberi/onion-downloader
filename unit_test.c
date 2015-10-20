#include "hash.h"
#include "url.h"
#include "opt.h"
#include "utils.h"
#include "request.h"
#include "file.h"
#include "parse.h"
#include "queue.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define LIST_SIZE 16

struct opt options;

static char *names[] = {"Host", "User-Agent", "Connection", NULL};
static char *vals[] = {"www.google.com", "Wget-1.16 (linux-gnu)", "keep-alive", NULL};

int
test_html_tag_list (void)
{
	struct html_tag_list *l = html_tag_list_init (NULL);
	return 0;
}

int
test_parse (void)
{
	/*char *(*func)(char *, struct hash_table *, char *) = build_tag;*/
	struct hash_table *table = hash_table_new (31);
	char *tag, *end;

	hash_table_put (table, "href", "http://cs.umd.edu");
	tag = build_html_tag ("a", table, "UMD CS Homepage");

	printf ("%s\n", tag);
	parse_tag (tag, &end, NULL);

	hash_table_remove (table, "href");
	hash_table_put (table, "src", "http://cs.umd.edu/icon.png");
	tag = build_html_tag ("img", table, "");

	printf ("%s\n", tag);
	parse_tag (tag, &end, NULL);

	hash_table_put (table, "href", "http://cs.umd.edu/index.html");
	tag = build_html_tag ("img", table, "");

	printf ("%s\n", tag);
	
	parse_tag (tag, &end, NULL);

	printf ("\n\n");
	
	return 0;
}


int
test_file_creation (void)
{
	char *filename = "foo.txt";
	exist_t status;
	
	/* make_dirs (filename);*/

	if (strcmp ("foo", strip_tail_from_path ("foo/bar")) != 0 ||
		strcmp ("foo/bar", strip_tail_from_path ("foo/bar/baz")) != 0 ||
		strcmp ("foo/bar/baz", strip_tail_from_path ("foo/bar/baz/blah")) != 0)
		goto fail;	
	
	if ((status = make_dirs ("foo/bar/")) != DIR_EXISTS)
	{
		switch (status)
		{
			case NO_EXIST:
				fprintf (stderr, "Failed to create directory, it should not exist in the current file hierarchy. \
									There may be a file specified in a substring of the path that already exists.\n");
				break;
			case FILE_EXISTS:
				fprintf (stderr, "A file by the name of %s already exists, or a file somewhere along the path \
							of %s exists currently. Failed to create the directory\n", filename, filename);
				break;
			default:
				break;
		}
		goto fail;
	}
	
	return 0;	
		
	fail:
		return -1;
}

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
	
	fprintf (stderr, "What's %s\n", val);

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
test_parse_2 (void)
{
	char *end;
	char *test = "<img src=\"http://cs.umd.edu/icon.png\" href=\"http://cs.umd.edu\"></img>";
	struct html_tag *t = parse_tag (test, &end, NULL);
	struct html_tag_list *l = html_tag_list_init (t);
	printf ("\"%s\"\n", test);
	print_tag (t);
	free (t);

	test = "<img src=\"http://cs.umd.edu/class/fall2015/cmsc351/hwk1.pdf\" href=\"http://cs.umd.edu\"></img>";
	t = parse_tag (test, &end, NULL);
	printf ("\"%s\"\n", test);
	print_tag (t);
	free (t);

	test = "<img src=\"http://linux.die.net/man/3/gethostbyname\" href=\"http://cs.umd.edu\"></img>";
	t = parse_tag (test, &end, NULL);
	printf ("\"%s\"\n", test);
	print_tag (t);
	free (t);

	/* Shouldn't work (as in, the parser should recognize that this is not valid HTML) */
	test = "<img src=\"http://cs.umd.edu/icon.png\" href=\"http://cs.umd.edu\">Testing to see if I'm a beast<img>";
	t = parse_tag (test, &end, NULL);
	printf ("\"%s\"\n", test);
	print_tag (t);
	free (t);
	
	test = "<img src=\"http://cs.umd.edu/icon.png\" href=\"http://cs.umd.edu\">Testing to see if I'm a beast</img><a href=\"aberi.github.io\">\
</a><img src=\"http://cs.umd.edu/icon.png\" href=\"http://cs.umd.edu\">Testing to see if I'm a beast</img>";
	t = parse_tag (test, &end, NULL);
	printf ("\"%s\"\n", test);
	print_tag (t);
	
	free (l);
	l = get_all_tags (test);

	printf ("********* TRYING TO PRINT SUCCESSIVE TAGS **********\n");
	
	print_all_tags (l);
	
	free (t);

	return 0;
}

int test_merge_lists (void)
{
	struct html_tag_list *l1, *l2;
		
	l1 = find_all_tags ("<img src=\"http://cs.umd.edu/icon.png\" href=\"http://cs.umd.edu\">Testing to see if I'm a beast</img><a href=\"aberi.github.io\
\"></a><img src=\"http://cs.umd.edu/icon.png\" href=\"http://cs.umd.edu\">Testing to see if I'm a beast</img>", "img");

	l2 = find_all_tags ("<img src=\"http://cs.umd.edu/icon.png\" href=\"http://cs.umd.edu\">Testing to see if I'm a beast</img><a href=\"aberi.github.io\
\"></a><img src=\"http://cs.umd.edu/icon.png\" href=\"http://cs.umd.edu\">Testing to see if I'm a beast</img>", "img");

	merge_lists (l1, l2);
	print_all_tags (l1);	
	
	return 0;
}

int
test_parse_file (void)
{
	int fd = open ("index.html", O_RDONLY, 0);
	struct html_tag_list *the_list = get_links_from_file (fd);

	print_all_tags (the_list);	
	
	return 0;
}


int
test_find_tag_among_other (void)
{
	struct html_tag_list *htl;
	char *test = "<img src=\"/icon.png\" href=\"/\">Hello all</img><img src=\"/icon.png\" href=\"/\">Hello all</img><a href=\"/index.html\"></a><img\
 src=\"/img.png\" href=\"/index.html\"></img>";
	char *names[] = {"img", "a",  NULL};
	char *f = find_tag (test, "img");
	
	htl = find_all_tags (test, "img");
	print_all_tags (htl);
	
	printf ("\n\nTRYING THE DIFFICULT ALGORITHM\n\n");
		
	htl = find_tags_by_name (test, names, NULL);
	print_all_tags (htl);
	
	return 0;
}

int
test_queue (void)
{
	struct url_queue *queue = url_queue_init ();
	url_t u, *p;
	p = &u;
	parse_url ("http://www.google.fr", p);
	enqueue (queue, p);
	print_queue (queue);
	parse_url ("http://www.amazon.com", p);
	enqueue (queue, p);
	print_queue (queue);
	return 0;
	
	fail: return -1;
}

int
main (void)
{	
/*	if (test_hash_table () < 0) 
		fprintf (stderr, "Hash table test failed\n"); 

	if (test_request () < 0)
		fprintf (stderr, "Requests not working properly\n");

	if (test_file_creation () < 0)
		fprintf (stderr, "File creation not working properly\n");
	
	if (test_parse_2 () < 0)
		fprintf (stderr, "Parsing not working properly\n");

	if (test_html_tag_list () < 0)
		fprintf (stderr, "Linked list of HTML tags is not working\n");	*/ 

/*	if (test_find_tag_among_other () < 0)
		fprintf (stderr, "Can't find tags properly\n"); */
	
	/* if (test_merge_lists () < 0)
		fprintf (stderr, "Lists are not merging properly\n"); */
	
	if (test_parse_file () < 0) 
		fprintf (stderr, "Cannot parse the file\n"); 

	if (test_queue () < 0)
		fprintf (stderr, "Queue structure is not working properly\n");

	return 0;
}
