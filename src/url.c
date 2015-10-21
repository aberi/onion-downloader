#include "opt.h"
#include "url.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#define MAX_URL_LENGTH 512 /* We won't worry too much about query strings for the moment. */

#ifndef IS_DIGIT
#define IS_DIGIT(x) (((x) - '0') >= 0 && ((x) - '0' <= 9))
#endif
#define min(a,b) ((a) < (b) ? (a) : (b))

#ifndef HTTP_PORT
#define HTTP_PORT 80
#endif

#define COPY_TO_BUF(buf, src, len) do { \
	memset (buf, 0, sizeof (buf)); 		\
	strncpy (buf, src, len);			\
} 										\
while (0)


static char buf[MAX_URL_LENGTH];

/* We already have a buffer so don't allocate another one on the stack
 * within the function. Use 0 to indicate success and -1 to indicate
 * anything else.*/

#ifndef MAX_DIR_SIZE
#define MAX_DIR_SIZE 128
#endif

#ifndef DIR_PERMS
#define DIR_PERMS 0755
#endif

char *
get_url_directory (struct url *url)
{
	char *end = skip_dirs (url->full_url);
	char *ret;
	if (strcmp (end, url->host) == 0)
	{
		ret = malloc (url->full_url + 2);
		strcpy (ret, url->full_url);
		ret[ strlen (url->full_url) ] = '/';
		return ret;
	}
	ret = malloc ((end - url->full_url) + 1);

	strncpy (ret, url->full_url, end - url->full_url);
	return ret;
}


int
not_outgoing (const char *link)
{
	return is_relative (link) || is_absolute (link);
}

/* Assume for now that we never encounter any FTP links */
int 
is_relative (const char *link)
{
	return *link != ' ' 
			&& *link != '/'  /* absolute path that is located on the same remote host */
			&& strncmp (link, "http", 4) != 0; /* Outgoing link to new host */
}

int
is_absolute (const char *link)
{
	return *link == '/';
}

int
is_outgoing (const char *link)
{
	return strncmp (link, "http", 4) == 0;
}

int
is_outgoing_http (const char *link)
{
	return strncmp (link, "http:", 5) == 0;
}

int
is_outgoing_https (const char *link)
{
	return strncmp (link, "https:", 5) == 0;
}

int
create_directories (char *pathname)
{
	int status;
	struct stat st;
	char *dir_start, *dir_end;
	char *file_part; /* Beginning of the part of PATHNAME indicating the file */
	char *local_path; /* Path that is being used on the local host */
	
	/* Assume that the user is specifying the full path. We do this
     * so that we can pass relative links from HTML pages and have
     * the files created according to those links. If there is
     * no slash at the end, chop off the tail of the path (the
     * part indicating the filename) because we have no use for it 
     * (we are only creating prerequisite directories). If there is no tail, 
     * assume that all parts of the path are directories that may
     * or may not exist yet. */

	local_path = strdup (pathname); /* Don't modify the original path */
	if (local_path[0] == '/')
		local_path++;

	file_part = skip_dirs (local_path);
	if (strcmp (file_part, local_path) == 0)
		return 0; /* Nothing needs to be done, the entire path is just a filename with no directories above it */

	if (strcmp (file_part, "") != 0) /* If we have a non-empty string, discard the filename */
	{
		local_path[file_part - local_path] = '\0';
		fprintf (stderr, "Directory structure that may need to be created: %s\n", local_path);
	}
		
	status = stat (local_path, &st);
	if (status == 0)
		return 0;	
	
	else 
	{ 
		int exists;
		dir_start = local_path;
		dir_end = strchr (local_path, '/');
	
		while (dir_end) /* We are assured to be at the end of some directory because we have found the '/' character */
		{
			char dir[MAX_DIR_SIZE + 1];
			memset (dir, 0, MAX_DIR_SIZE + 1);
			strncpy (dir, dir_start, dir_end - dir_start);	

			fprintf (stderr, "Trying to create directory %s\n", dir);	
		
			exists = stat (dir, &st);
			if (exists < 0) /* We need to create it  */
			{
				if (mkdir (dir, DIR_PERMS) < 0)
				{	
					perror ("Unable to create directory");	
					exit (1);
				}
			}
			dir_end++;
			dir_end = strchr (dir_end, '/');
		}
	}
	
	return 0;
}

url_err_t
parse_url (char *url, url_t *u)
{

	/* Make sure it is a valid URL. We are only using HTTP for now so make sure that
 	 * either (a) the URL begins with "http://" or (b) the URL begins with a host. */

 	/* We will assume that if the char sequence "://" is not present then we have
 	 * a host name that we will assume is using HTTP (port 80) */

	char *protocol_begin, *protocol_end;
	char *host_begin, *host_end;
	char *port_begin, *port_end;
	char *path_begin, *path_end;
	int has_protocol = 0;
	
	memset (u, 0, sizeof (url_t));
	protocol_begin = url;
	protocol_end = strstr (url, "://");	
	
	if (protocol_end)
	{
		if (strncmp (protocol_begin, "http", 4) != 0)
		{
			fprintf (stderr, "Protocol of url not supported\n");
			return PROTO_NOT_SUP;
		}
		if (3 >= strlen (protocol_end))
		{
			fprintf (stderr, "Host not found in URL\n");
			exit (1);
		}
		host_begin = protocol_end + 3;
		u->protocol = malloc (protocol_end - protocol_begin + 1);
		strncpy (u->protocol, protocol_begin, protocol_end - protocol_begin);
	
		has_protocol = 1;
	}
	else
	{
		host_begin = url;
		u->protocol = strdup ("http");
	}
	#ifndef HTTPS_PORT
		#define HTTPS_PORT 443
	#endif

	u->port = strcmp (u->protocol, "https") ? HTTP_PORT : HTTPS_PORT;
	
	port_begin = strchr (host_begin, ':');	
	if (port_begin)
	{
		char *tmp;

		port_begin++;
		tmp = port_begin;
		while (IS_DIGIT(*tmp++));
		if (tmp == port_begin)
		{	
			fprintf (stderr, "No port specified\n");
			return MALFORMED_URL;	
		}
		
		port_end = tmp - 1;
	
		COPY_TO_BUF (buf, port_begin, port_end - port_begin);
		/* fprintf (stderr, "Found port %s\n", buf); */
		u->port = atoi (buf); 
	}
	
	host_end = port_begin ? port_begin - 1 : strchr (host_begin, '/');	
	
	if (!host_end)
	{	
		u->path = strdup ("/");	
		u->host = strdup (host_begin); /* Don't do this because we end up having a slash on the end of hostnames when the path is "/' */
		if (host_end && host_end[1] == '\0')
			u->host[strlen (u->host)] = '\0';
	}
	else  /* Construct the path */
	{
		char *query_begin;
		char *fragment_begin;

		path_begin = (port_begin ? port_end : host_end);
		if (path_begin && strlen (path_begin) > 0) 
		{	
			query_begin = strchr (path_begin, '?');
			fragment_begin = strchr (path_begin, '#');
			if (query_begin && fragment_begin)
				path_end = min (query_begin, fragment_begin);
			else
				path_end = query_begin ? query_begin : (fragment_begin ? fragment_begin : path_begin + strlen (path_begin));

			if (query_begin)
			{	
				query_begin++;
				u->query = strdup (query_begin);
			}
		
			COPY_TO_BUF(buf, path_begin, path_end - path_begin);
			u->path = strdup (buf);
			/* if (u->path[0] == '/')
				u->path++; */
		}
		else
			u->path = strdup ("");
	
		COPY_TO_BUF(buf, host_begin, host_end - host_begin);
		/* fprintf (stderr, "Found host %s\n", buf); */
		u->host = strdup (buf);
	}

	if (has_protocol) 
		u->full_url = strdup (url);	
	else
	{
		memset (buf, 0, sizeof (buf));
		sprintf (buf, "http://%s", url);
		u->full_url = strdup (buf);
		if (u->full_url[strlen (u->full_url) - 1] == '/')
			u->full_url[strlen (u->full_url) - 1] = '\0';
	}
	u->length = strlen (u->full_url);
	
	return URL_OK;	
}

/* Assign the url to a file on the local host. If necessary, create directories
 * so that the filename can be created according to the path. */

char *url_file_name (url_t *u)
{
	char *name, *tmp, *path = u->path;
	int path_end;
	if (strcmp ("", path) == 0 || strcmp ("/", path) == 0)
	{
		name = malloc (strlen ("index.html") + 1);	
		strcpy (name, "index.html");
		return name;
	}
	
	tmp = strdup (path);
	
	/* Remove any trailing slashes */
	while ((path_end = strlen (tmp) - 1) && tmp[path_end] == '/')
	{	
		tmp[path_end] = '\0';
	}
	
	/* Create prerequisite directories if recursive option has been specified. Do NOT 
 	 * create the file. Keep that functionality separate. */
	
	if (options.recursive)
	{	
		/* Use the original path because we want the trailing slash.
		 * Otherwise create_directories() will think that the last part of the path is a directory
		 * and will create it as a directory. */

		if (create_directories (path) != -1)
		{
			free (tmp);
			return path;
		}
		else
			return NULL; /* Indicate that something went wrong and we will not be able to create
				* the file yet */
	}
	else /* We aren't creating a directory structure */
	{	
		char *filename = strdup (skip_dirs (tmp));
		free (tmp);
		return filename;
	}
		
	/* Write the function so that if all the prerequisites are there, nothing happens. File creation
  	 * needs to be done separately */
	
}

void print_url (url_t *u)
{
	printf ("Full URL: %s\n", u->full_url);
	printf ("Length of full url: %d\n", u->length);
	printf ("Protocol: %s\n", u->protocol);
	printf ("Hostname: %s\n", u->host);
	printf ("Port number: %d\n", u->port);
	printf ("Path: %s\n", u->path);
	printf ("Query string: %s\n", u->query);
}
