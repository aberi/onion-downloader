#ifndef _UTILS_
#define _UTILS_

#define APPEND(p, str, len) strncpy (p, str, len); 	\
									p += len

#define SKIP_WS(p) if (p) while (*p == ' ' || *p == '\r' || *p == '\n') p++

#ifndef IS_DIGIT
	#define  IS_DIGIT(d)(((d - '0') >= 0) && ((d - '0') <= 9))
#endif

#define xstrdup(s) (s) ? strdup (s) : NULL;

#define LOG_PRINTF(s, fmt) if (fmt == NULL) 	 \
							fprintf (stderr, s); \
							else 				 \
								fprintf (stderr, s, fmt);

char *skip_dirs (char *);
char *file_in_path (char *);

#define xnew(type) (type*) calloc (1, sizeof (type))

#endif
