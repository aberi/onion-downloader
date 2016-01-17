#include "hash.h"
#include "utils.h"
#include "url.h"
#include "queue.h"
#include "request.h"
#include "parse.h"
#include "file.h"
#include "opt.h"
#include "types.h"

int
test_split_path(void)
{
	char **subdirs = split_path	("columbia/files/slideshow/../slideshow");

	while (*subdirs)
	{
		printf ("%s -> ", *subdirs);
	}
	
	return 0;

	fail:
		return -1;
}

int
test_queue (void)
{
	struct url_queue *queue = url_queue_init ();
	
	struct url u, *url = &u;
	parse_url ("http://ftp.gnu.org/gnu/wget", url);
	enqueue (queue, url);
	print_queue (queue, 3);

	parse_url ("http://www.ucla.edu", url);
	enqueue (queue, url);
	print_queue (queue, 3);

	parse_url ("http://www.ucla.edu", url);
	enqueue (queue, url);
	print_queue (queue, 3);

	url = dequeue (queue);
	print_queue (queue, 3);

	url = dequeue (queue);
	print_queue (queue, 3);

	return 0;	
		
	fail: return -1;
}

int
test_url_shorten (void)
{
	struct url u, *url = &u;
	char *orig_url = "http://google.com/webhp?search=who\%20is\%20Lord\%20Voldemort", *s_url;
	
	parse_url (orig_url, url);
	s_url = shortened_url (url);
	
	printf ("Shortened version of %s is %s\n", orig_url, s_url);

	orig_url = "http://google.com/webhp#findfood";
	parse_url (orig_url, url);
	s_url = shortened_url (url);
	
	printf ("Shortened version of %s is %s\n", orig_url, s_url);

	return 0;

	fail: return -1;
}

int 
main (void)
{

	if (test_queue () < 0) goto fail;	
	if (test_url_shorten () < 0) goto fail;	
	if (test_split_path () < 0) 
	{
		printf( "Failed to split the path properly\n");
		goto fail;
	}
		
	return 0;
	
	fail: 
		return -1;
}
