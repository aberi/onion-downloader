#ifndef _TYPES_H_
#define _TYPES_H_

#include "url.h"

#define HTTP_OK 200
#define HTTP_MOVED 301
#define HTTP_FOUND 302
#define HTTP_BAD_REQUEST 400
#define HTTP_NOT_FOUND 404

typedef struct downloaded_file {
	url_t *url;
	int current_size; /* We may not have been able to downlaod the entire file */
	int full_size;	
	const char *local_path;
} downloaded_file_t;

#endif
