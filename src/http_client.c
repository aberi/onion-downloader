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

#include "hash.h"
#include "opt.h"
#include "url.h"
#include "utils.h"
#include "types.h"
#include "request.h"

#define IPV4_ADDR_LEN 4

#define HTTP_PORT 80

static char *optstring = "Rm:o:p:r";

struct opt options;

/* Make an argument processing function to begin with.
 * Use the global options variable */

void usage(void);

static struct option longopts[] = {{"show-response", no_argument, NULL, 'R'}, {"output-file", required_argument, NULL, 'o'}};

void
init_opt (int argc, char **argv, char *opts)
{
	int opt;
	options.url = strdup (argv[1]);
	options.recursive = 0;
	options.output_fd = STDOUT_FILENO;
	options.show_server_response = 0;

	fprintf (stderr, "URL to be parsed: %s\n", options.url);	

	optind++;	
	while ((opt = getopt_long (argc, argv, opts, longopts, NULL)) != -1)
	{
		switch (opt)
		{
			case 'o':
				options.output_file = strdup (optarg);
				if ((options.output_fd = open (options.output_file, O_WRONLY | O_TRUNC | O_CREAT, 0666)) < 0)
				{	
					if ((options.output_fd = open ("./index.html", O_WRONLY | O_TRUNC | O_CREAT, 0666)) < 0)
						perror ("Output file name not valid... Using standard output instead!");	
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
	
	if (options.output_fd < 0)
		options.output_fd = STDOUT_FILENO;
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

int 
main(int argc, char *argv[])
{
	char *method = "GET";
	char *response_body;
	char buf[BUFSIZ];
	int sock, i;

	url_t u;

	struct sockaddr_in client, server;
	struct request *req;
	struct response *resp = malloc (sizeof (struct response));

	if (argc < 2)
		usage ();

	init_opt (argc, argv, optstring);

	parse_url (options.url, &u);

	sock = create_and_bind_tcp_socket (&client);
	resolve_host (u.host, &server, HTTP_PORT);

	if (connect_to_ip (sock, &server) < 0)
	{
		perror ("Unable to connect to the server");
		exit (1);	
	}
	
	req = create_request (&u, names, values, method);
	send_request (sock, req);
	response_body = read_response (sock, buf, sizeof (buf));
	parse_response (response_body, resp);

	write (options.output_fd, resp->content, resp->content_len); 

	if (options.show_server_response)
	{
		printf ("\n\n************* SERVER RESPONSE ***************\n\n");
		for (i = 0; i < resp->headers->size; i++)
		{
			if (resp->headers->table[i].key && resp->headers->table[i].state == OCCUPIED)
				printf ("%s: %s\n", resp->headers->table[i].key, resp->headers->table[i].value);
		}
	}

	return 0;
}
