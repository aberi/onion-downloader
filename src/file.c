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
		/*if (ret == DIR_EXISTS)
			fprintf (stderr, "Found directory %s\n", local_filename);
		else
			fprintf (stderr, "Found file %s\n", local_filename); */
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
			if (errno != EEXIST) /* Don't exit just because some of the prequisites already exist */
				return NO_EXIST;
	}
	
	/* All the prerequisites now exist, so create the original directory name that was passed */
	/*fprintf (stderr, "Trying to create %s\n", local_filename); */
	if (mkdir (local_filename, 0755) < 0)
		return NO_EXIST;				
	
	return DIR_EXISTS;
}
