#include <string.h>
#include "utils.h"

char *
skip_dirs (char *path) 
{
	char *new_path;
	new_path = strchr (path, '/');
	if (new_path)
		return skip_dirs (new_path + 1);
	else
		return path;
}
