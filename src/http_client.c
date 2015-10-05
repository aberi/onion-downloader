#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sysexits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <getopt.h>
#include <assert.h>

#include "hash.h"
#include "opt.h"
#include "url.h"
#include "utils.h"
#include "types.h"
#include "request.h"
#include "file.h"

#define IPV4_ADDR_LEN 4
#define MAX_REDIRECT 5
#define HTTP_PORT 80

static char *optstring = "Rm:o:Op:r";

struct opt options;

/* Make an argument processing function to begin with.
 * Use the global options variable */

void usage(void);

static struct option longopts[] = 
{{"show-response", no_argument,       NULL, 'R'}, 
 {"output-file",   required_argument, NULL, 'o'}};

void
init_opt (int argc, char **argv, char *opts)
{
	int opt;
	options.url = strdup (argv[1]);
	options.recursive = 0;
	options.output_fd = -1;
	options.show_server_response = 0;
	
	#ifdef DEBUG
	fprintf (stderr, "URL to be parsed: %s\n", options.url);	
	#endif

	optind++;	
	while ((opt = getopt_long (argc, argv, opts, longopts, NULL)) != -1)
	{
		switch (opt)
		{
			case 'O':
				options.print_content = 1;
				break;
			case 'o':
				options.output_file = strdup (optarg);
				if ((options.output_fd = open (options.output_file, O_WRONLY | O_TRUNC | O_CREAT, 0666)) < 0)
				{	
						perror ("Output file name not valid... Will attempt to save to a file based on the \
									path of the URL.");	
				}	
				break;	
			case 'p':
				break;
			case 'r':
				options.recursive = 1;
				break;
			case 'R':
				options.show_server_response = 1;
				break;
			case 'm':
				options.method = strdup (optarg);
				break;
			default:
				usage();
				break;
		}	
	}
}

int
create_and_bind_tcp_socket (struct sockaddr_in *addr) 
{
	int sock;
	if ((sock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) 
	{
		perror ("Couldn't create socket locally");
		exit (1);	
	}
	
	addr->sin_addr.s_addr = htonl (INADDR_ANY);
	addr->sin_family = AF_INET; /* TODO: implement IPv6 */
	addr->sin_port = htons (0);
	
	if (bind (sock, (struct sockaddr *) addr, sizeof (struct sockaddr_in)) < 0) 
	{
		perror ("Couldn't bind socket to local machine");
		exit (1);
	}
	
	return sock;
}

int
resolve_host (const char *hostname, struct sockaddr_in *addr, int port) 
{
	struct hostent *host;
	if ((host = gethostbyname (hostname)) == NULL || host->h_length != IPV4_ADDR_LEN || host->h_addr == NULL 
		|| host->h_addrtype != AF_INET) 
	{
		perror ("Couldn't resolve host");
		exit (1);
	}

	addr->sin_family = AF_INET;
	memcpy ((void*) &addr->sin_addr.s_addr, host->h_addr, host->h_length); 
	
	addr->sin_port = htons (port);
	
	return 0;
}

int
connect_to_ip (int sock, struct sockaddr_in *addr)
{
	if (addr->sin_family != AF_INET)
		return -1;
	if (addr->sin_port == htons (0)) /* a 0 here indicates that any port can be selected. We'll use HTTP as our default */
		addr->sin_port = htons (HTTP_PORT); /* Use HTTP as default protocol */
	return connect (sock, (struct sockaddr *) addr, sizeof (struct sockaddr_in));
}

void
usage (void)
{
	fprintf (stderr, "Usage: client <url> [-o output file] [-r] [-m http method]\n");
	exit (1);
}


char *names[] = {"Host", "User-Agent", "Connection", NULL};
char *values[] = {"www.google.com", "Wget-1.16 (linux-gnu)", "Keep-Alive", NULL};

struct *hash_table
fill_header_table (char **names, char **values)
{
	struct hash_table *headers = hash_table_new (31);	
	while (*names)
	{
		hash_table_put (headers, *names++, *values++);	
	}
		
	return headers;
}

int 
main(int argc, char *argv[])
{
	char *method = "GET";
	char *response_body;
	char buf[BUFSIZ];
	int sock, num_redirect = 0;

	url_t u;

	struct sockaddr_in client, server;
	struct request *req;
	struct response *resp = malloc (sizeof (struct response));
	struct content *response_content;
	resp->status = -1;

	if (argc < 2)
		usage ();

	init_opt (argc, argv, optstring);
	parse_url (options.url, &u);

	if (options.output_fd < 0)
	/* Write to a file based on the path of the URL. If the path is /, use index.html by default */
	{
		char *path = strdup (u.path + 1);
		#ifdef DEBUG
		fprintf (stderr, "Writing to %s\n", path);
		#endif
		if (make_dirs (path) != DIR_EXISTS)
			fprintf (stderr, "The path does not exist.\n");
		if (strlen (path) == 0 || (options.output_fd = open (path, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0)
		{
			if ((options.output_fd = open ("index.html", O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0)
				options.output_fd = STDOUT_FILENO;
			else
				options.output_file = strdup ("index.html");	
		}
		else
			options.output_file = strdup (path);
		free (path);	
	}

	while (resp->status != HTTP_OK && num_redirect < MAX_REDIRECT)
	{
		sock = create_and_bind_tcp_socket (&client);
		resolve_host (u.host, &server, HTTP_PORT);
	
		if (connect_to_ip (sock, &server) < 0)
		{
			perror ("Unable to connect to the server");
			exit (1);	
		}
		req = create_request (&u, names, values, method);
		send_request (sock, req);
		response_content = read_response (sock, buf, sizeof (buf));
		response_body = response_content->body;
	
		assert (response_body);	
	
		parse_response (response_content, resp);

		if (options.show_server_response)
			printf ("\n\n************* SERVER RESPONSE ***************\n\n%s\n", resp->header_body);

		if (options.print_content && options.output_fd != STDOUT_FILENO)
			write (STDOUT_FILENO, response_body, response_content->len);  /* Don't do this, just write the content
																			to the file during parsing */

		switch (resp->status)
		{
			char *location;
			case HTTP_OK:
				return 0;
			case HTTP_MOVED:
			case HTTP_FOUND:
				if ((location = hash_table_get (resp->headers, "Location")))
				{	
					#ifdef DEBUG
					fprintf (stderr, "New location is %s\n", location); 
					#endif
					parse_url (location, &u);	
					print_url (&u);
					num_redirect++;
				}
				else
					return 1;
				break;
			case HTTP_NOT_FOUND:
				return 1;
				break;
			default:
				return 1;
				break;
		}
	}

	return 0;
}
