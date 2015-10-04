#ifndef _FILE_
#define _FILE_

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sysexits.h>
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>	

typedef enum existence
{
	DIR_EXISTS,
	FILE_EXISTS,
	NO_EXIST
} exist_t;

exist_t directory_exists(char *dirname);
int create_file (char *filename);
exist_t make_dirs (char *filename);
char *strip_tail_from_path (char *);
char *load_file_into_string (char *filename);
	
#endif
