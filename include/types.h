#ifndef _TYPES_H_
#define _TYPES_H_

#include "url.h"

typedef struct downloaded_file {
	url_t *url;
	int current_size; /* We may not have been able to downlaod the entire file */
	int full_size;	
	const char *local_path;
} downloaded_file_t;

#endif
