#include "file.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

typedef unsigned int uint32_t;


exist_t
directory_exists (char *dirname)
{
	struct stat st;
	if (stat (dirname, &st) != -1)
	{
		return ((st.st_mode & S_IFMT) == S_IFDIR) ? DIR_EXISTS : FILE_EXISTS;
	}
	return NO_EXIST;
}

int
file_exists (char *filename)
{
	struct stat st;
	char *local_filename = calloc (2 + strlen (filename) + 1, sizeof (char));
	strcpy (local_filename, "./");
	strcpy (local_filename + 2, filename);

	if (stat (local_filename, &st) != -1)
	{
		free (local_filename);
		return ((st.st_mode & S_IFMT) == S_IFREG) ? 1 : 0;
	}

	free (local_filename);
	return -1;
}

int 
create_file (char *filename)
{
	return 0;	
}

char *
strip_tail_from_path (char *path)
{
	char *cur, *prev, *ret;
	cur = strchr (path, '/'); prev = path;
	
	while (cur)
	{	
		prev = cur;	
		cur = strchr (cur + 1, '/');
	}
	
	ret = calloc (1, prev - path + 1);
	strncpy (ret, path, prev - path);
	ret[prev - path] = 0;	

	return ret;
}

exist_t
make_dirs (char *filename)
{
	char *cur;
	char *buf;
	char *local_filename = strdup (filename);
	int length;
	exist_t ret;

	if ((ret = directory_exists (local_filename)) != NO_EXIST)
	{
		fprintf (stderr, "File %s exists\n", local_filename);
		return ret;	
	}
	
	length = strlen (local_filename); 
	
	if (length > 0 && local_filename[length - 1] == '/')
		local_filename[length - 1] = '\0';
	else
		local_filename = strip_tail_from_path (local_filename);

	cur = local_filename;
	
	buf = calloc (1, length + 1);

	while ((cur = strchr (cur + 1, '/')))
	{
		memset (buf, 0, sizeof (buf));	
		strncpy (buf, local_filename, cur - local_filename);
	
		/* fprintf (stderr, "Trying to create %s\n", buf); */

		if (mkdir (buf, 0755) < 0)
		{
			fprintf (stderr, "Couldn't create the file\n");
			if (errno == EEXIST)
			{
				char *cmd = malloc (5 + strlen (local_filename) + 1);		
				sprintf (cmd, "rm ./%s", local_filename);
				system (cmd);
				free (cmd);
				if (mkdir (local_filename, 0755) < 0)
					return NO_EXIST;	
			}
		}
	}
	
	/* All the prerequisites now exist, so create the original directory name that was passed */
	/*fprintf (stderr, "Trying to create %s\n", local_filename); */
	if (mkdir (local_filename, 0755) < 0)
	{
		fprintf (stderr, "Couldn't create the file\n");
		if (errno == EEXIST)
		{
			char *cmd = malloc (5 + strlen (local_filename) + 1);		
			sprintf (cmd, "rm ./%s", local_filename);
			system (cmd);
			free (cmd);
			if (mkdir (local_filename, 0755) < 0)
				return NO_EXIST;	
			else
				return DIR_EXISTS;
		}
	}
	
	return DIR_EXISTS;
}
