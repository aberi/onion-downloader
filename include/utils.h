#ifndef _UTILS_
#define _UTILS_

#define xstrdup(s) (s) ? strdup (s) : NULL;

#define LOG_PRINTF(s, fmt) if (fmt == NULL) 	 \
							fprintf (stderr, s); \
							else 				 \
								fprintf (stderr, s, fmt);

char *skip_dirs (char *);

#endif
