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
#include "file.h"
#include "parse.h"
#include "queue.h"

#define IPV4_ADDR_LEN 4
#define MAX_REDIRECT 5
#define HTTP_PORT 80

static char *optstring = "Rm:o:Op:r";
static int n_downloaded = 0;

/* static struct hash_table *dl_url_file_map;*/

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
	unsigned char *ad;
	fprintf (stderr, "Trying to resolve %s...\n", hostname);

	if ((host = gethostbyname (hostname)) == NULL || host->h_length != IPV4_ADDR_LEN || host->h_addr == NULL 
		|| host->h_addrtype != AF_INET) 
	{
		perror ("Couldn't resolve host");
		exit (1);
	}
	
	ad = (unsigned char *) host->h_addr;
	fprintf (stderr, "Success, resolved %s to address %d.%d.%d.%d\n", hostname, ad[0], ad[1], ad[2], ad[3]);

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


char *names[] = {"Host", NULL};
char *values[] = {"www.google.com", NULL};

struct hash_table *
fill_header_table (char **names, char **values)
{
	struct hash_table *headers = hash_table_new (31);	
	while (*names)
	{
		hash_table_put (headers, *names++, *values++);	
	}
		
	return headers;
}

void
create_output_file (char *url_path)
{
	char *path = strdup (url_path + 1); /* It must always be the case that URL paths within the url_t struct 
												begin with a '/', otherwise this will cause serious problems! */
	char *temp = path;

	if (strlen(path) && path[strlen (path) - 1] == '/') /* Assume we are doing file creation! If the request is
											to a "directory" of a webpage, treat it like a 
											regular webpage by default  */
		path[strlen (path) - 1] = '\0';


	if (!options.recursive)
		path = skip_dirs (path);
	else if (make_dirs (path) != DIR_EXISTS)
		fprintf (stderr, "The directories necessary to create the file in the location given by %s do not exist.\n", path);
		/* It's definitely possible (likely) that we are just going to use index.html as the filename if this
 		 * has happened. */
	#ifdef DEBUG
	fprintf (stderr, "Writing to %s\n", path);
	#endif
	if (strlen (path) == 0 || (options.output_fd = open (path, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0)
	{
		if ((options.output_fd = open ("index.html", O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0)
			options.output_fd = STDOUT_FILENO;
		else
			options.output_file = strdup ("index.html");	
	}
	else
		options.output_file = strdup (path);
	free (temp);	
}

struct content *
download_file (int sock, struct url *url, char *method, struct hash_table *headers, struct response **response)
{
	struct request *req;	
	char buf[BUFSIZ];
	struct content *response_content;
	struct response *resp = malloc (sizeof (struct response));

	create_output_file (url->path);

	req = make_request (url, headers, method);
	send_request (sock, req);

	response_content = read_response (sock, buf, sizeof (buf));

	parse_response (response_content, resp);
	*response = resp;	

	if (options.show_server_response)
			printf ("\n\n************* SERVER RESPONSE ***************\n\n%s\n", resp->header_body);

	if (resp->status == 200)	
		n_downloaded++;
	
	close (options.output_fd);

	return response_content;
}

int
make_connection (struct url *url, struct sockaddr_in *client, struct sockaddr_in *server)
{
	int sock;
	resolve_host (url->host, server, HTTP_PORT);

	options.sock = sock = create_and_bind_tcp_socket (client);
	if (connect_to_ip (sock, server) < 0) 
	{
		perror ("Unable to connect to the server");
		exit (1);	
	}
		
	return sock;
}

/* Given a list of hrefs (URL paths), 
 * download the files created by combining
 * the host with the appropriate path (this
 * depends on whether the hrefs/links
 * are relative or absolute */
void
http_loop (int sock, 
		   struct url *u, 
		   struct sockaddr_in *client, 
		   struct sockaddr_in *server, 
		   struct html_tag_list *the_list, 
		   char **hrefs,
		   char *base, 
		   char *method, 
		   struct hash_table *headers,
		   struct response **resp)
{
	int k;
	for (k = 0; k < the_list->count; k++)
	{
		if (hrefs[k] && strchr (hrefs[k], '@') == NULL && strstr (hrefs[k], "://") == NULL) /* Avoid downloading "mailto" links */
		{
			char *new_url;
			char *temp;

			if ( is_absolute (hrefs[k]) )
				new_url = create_new_url_absolute_path (u->host, hrefs[k]);

			else if ( is_relative (hrefs[k]) )
				new_url = create_new_url_relative_path (base, hrefs[k]);

			fprintf (stderr, "New url is %s\n", new_url);

			temp = strdup (new_url);
			parse_url (temp, u);
			free (temp);

			close (sock);

			if ( file_exists (u->path) != 1)
			{   /* Eventually we want to be using persistent connections instead
						of reconnecting for every new file. This will be especially
						important over secure connections to reduce latency. */
				sock = make_connection (u, client, server);
				download_file (sock, u, method, headers, resp);
			}
		}
	}
}

/* Retrieve all the files specified by relative or absolute links
 * within the downloaded file, i.e., options.output_file should
 * have already been written to. */
void
retrieve_links (int sock, 
			    struct url *u, 
			    struct sockaddr_in *client, 
			    struct sockaddr_in *server, 
				char *method, 
				struct hash_table *headers,
				struct response **resp)
{
	int new_fd;
	struct html_tag_list *the_list;
	close (options.output_fd);
	if ((new_fd = open (options.output_file, O_RDONLY, 0)) != -1)
	{
		char **hrefs, *base;
	
		the_list = get_links_from_file (new_fd);

		hrefs = get_all_attribute (the_list, "href", not_outgoing);
		base = get_url_directory (u);

		http_loop (sock, u, client, server, the_list, hrefs, base, method, headers, resp);
	}
}

int 
main(int argc, char *argv[])
{
	char *method = "GET";
	char buf[BUFSIZ];
	int sock, num_redirect = 0; /* Socket that is connected to the host and the number of redirections (3xx codes)
									that have taken place during the current attempt to download a webpage */
	url_t u;
	
	struct sockaddr_in client, server; /* Addresses of the local and remote host */
	struct response *resp = malloc (sizeof (struct response)); /* Response given by remote host */
	struct content *response_content; /* File that would be displayed on a web browser. */
	struct hash_table *headers; 
	resp->status = -1; /* Indicate that the server has not responded yet */

	if (argc < 2)
		usage ();

	init_opt (argc, argv, optstring);
	parse_url (options.url, &u);
		
	headers = fill_header_table (names, values); /* We may as well not have a default "Host" value because of the next line */
	hash_table_put (headers, "Host", u.host);		

	if (options.recursive)
	{
		mkdir (u.host, 0755);
		chdir (u.host);
	}
	
	while (resp->status != HTTP_OK && num_redirect < MAX_REDIRECT)
	{
		sock = make_connection (&u, &client, &server);
		response_content = download_file (sock, &u, method, headers, &resp);

		switch (resp->status)
		{
			char *location, *fmt;
			case HTTP_OK:

					if (options.recursive)
						retrieve_links (sock, &u, &client, &server, method, headers, &resp);	
					printf ("Retrieved %d files from %s.\n", n_downloaded, u.host);
					return 0;

			case 301:
			case 302:
			case 307:
			case 308:
				if ((location = hash_table_get (resp->headers, "Location")))
				{	
					#ifdef DEBUG
					fprintf (stderr, "New location is %s\n", location); 
					#endif

					parse_url (location, &u);	
					if (options.recursive)
					{
						chdir ("..");
						mkdir (u.host, 0755);
						chdir (u.host);
					}
					hash_table_put (headers, "Host", u.host);
					create_output_file (u.path); /* Truncate the previous file... we don't want
					"301 Moved Permanently" showing up at the top of the file */
					num_redirect++;
				}
				else
					return 1;
				break;	
					
			case 400: /* Could rm the output file */
			case 403:
			case 404:
				fmt = "rm %s";
				memset (buf, 0, sizeof (buf));
				sprintf (buf, fmt, options.output_file);
				system (buf);
				return 1;
				break;

			default:
				return 1;
				break;

		}
	}

	return 0;
}
