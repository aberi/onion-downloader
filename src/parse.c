#include "parse.h"
#include "hash.h"
#include "utils.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#define TAG_START_FMT "<%s"
#define TAG_END_FMT "</%s>"

/* Subject to change */
static char *important_tags[] = { "a", "src", NULL};

/* This is it. */
struct html_tag_list *
get_links_from_file (int fd)
{
	char buf[BUFSIZ]; /* If a tag is partially in one buffer and partially in the next,
							the buffer should be big enough that it will not trickle into
							a third buffer i.e. at most 2 buffers for a single tag */	

	struct html_tag_list *the_list = html_tag_list_init (NULL); /* Master list */
	struct html_tag_list *round_list = html_tag_list_init (NULL); /* Links from the current buffer. Merge them with the master list */
	int n_bytes, n_read = 0;

	while ((n_bytes = read (fd, buf, sizeof (buf))) > 0)
	{
		n_read += n_bytes;
		round_list = find_tags_by_name (buf, important_tags);
		merge_lists (the_list, round_list);	
	}

	return the_list;	
}

char *
tag_start (char *tag_name)
{
	/* 							"<%s"					 tag_name     '\0', subtract format string */
	char *ret = malloc (strlen (TAG_START_FMT) + strlen (tag_name) + 1 - 2);
	
	sprintf (ret, "<%s", tag_name);
	return ret;
}

char *
tag_end (char *tag_name)
{
	char *ret = malloc (strlen (TAG_START_FMT) + strlen (TAG_END_FMT) + 1 - 2);
	
	sprintf (ret, "</%s>", tag_name);
	return ret;
}

int arr_size (char **arr)
{
	int c = 0;
	while (*arr++)
		c++;
	return c;
}

char *
build_html_tag (char *name, struct hash_table *table, char *content)
{
	char *tag, *p;
	char *start = tag_start (name), *end = tag_end (name);
	char **attr_names = keys (table);
	char **temp;
	int size = 0;

	size += strlen (start);
	
	temp = attr_names;	
	while (*attr_names)
	{
		char *value = hash_table_get (table, *attr_names);
		/* " href=\"http://link.org/\" " will be added on if the attribute
 			href has the value http://link.org/ */
		size += 1 + strlen (*attr_names) + 1 + 1 + strlen (value) + 1 + 1;
		attr_names++;
	}
	attr_names = temp;
	/* ">" but we have one extra space, so delete that space and replace it with ">". 
 		This is why the following line is commented out */
	/* size += 1; */
	size += strlen (content);
	size += strlen (end);
	
	tag = malloc (size + 1);	

	p = tag;	
	APPEND(p, start, strlen (start));
	while (*attr_names)
	{
		char *value = hash_table_get (table, *attr_names);
		APPEND(p, " ", 1); 
		APPEND(p, *attr_names, strlen (*attr_names)); 
		APPEND(p, "=\"", 2);
		APPEND(p, value, strlen (value));
		APPEND(p, "\"", 1);
		attr_names++;
	}
	
	APPEND(p, ">", 1);
	APPEND(p, content, strlen (content));
	APPEND(p, end, strlen (end));

	return tag;	
}

char *
end_quote (char *text)
{
	char *cur = text;
	while ((cur = strchr (cur + 1, '"')))
		if (cur[-1] != '\\') /* I'm not sure if this what it would look like in the actual hypertext */
			return cur;	
	
	return NULL;
}

#define CURRENT(start, end) printf ("Start=%s\nEnd=%s\n", start, end)

/* Return a parsed html tag with a name, hash table of attributes, and 
 * the content of the html tag (plain text displayed on the webpage) 
 * as well as the point in the text where the tag ends. */
struct html_tag *
parse_tag (const char *tag, char **endpoint)
{
	char buf[BUFSIZ];
	struct html_tag *ret = calloc (1, sizeof (struct html_tag));
	struct hash_table *attributes = hash_table_new (31);
	char *name;
	char *content;
	char *end;
	char *ws;
	int has_attributes;
	char *start;
	if (!tag) return NULL;
	start = strchr (tag, '<');

	if (start == NULL)
		return NULL;

	end = strchr (start, '>');	
	ws = strchr (start, ' ');
	if (ws < end)
	{
		char *temp = ws;
		SKIP_WS(temp);
		if (*temp != '>')
			has_attributes = 1;
	
		end = ws; /* The end of the name will be at the beginning of the whitespace */
	}
	
	/* Otherwise, there must be no attributes and the name will end where the character '>'
 		is found */

	memset (buf, 0, sizeof (buf));
	strncpy (buf, start + 1, end - start - 1);
	name = strdup (buf);
	ret->name = name;

	while (has_attributes) /* attributes should begin after the end of the name */
	{
		char *attr_name, *attr_value;
		SKIP_WS(end);
		start = end;
		end = strchr (end, '=');
		if (!end) goto err;
		
		/* Get the name value */
		attr_name = malloc (end - start + 1);
		strncpy (attr_name, start, end - start);
		/* Out of paranoia */
		attr_name[end - start] = '\0';

		end++; /* Skip the '=' */
	
		if (*end++ != '"') goto err;
	
		start = end;
		end = end_quote (end);

		attr_value = malloc (end - start + 1);	
		strncpy (attr_value, start, end - start);
		buf[end - start] = '\0';

		hash_table_put (attributes, attr_name, attr_value);
	
		free (attr_name);
		free (attr_value);
	
		if (*++end == '>')
			break;
			
		/* We have to find the end of the name-value pair by finding the second double quotation mark.
 			From there we can see whether there is whitespace left before the content begins or if
			there are no attributes left to parse */	
		
	}

	/* Start small by assuming that no HTML tags are within the content body. This will clearly
 	 * not hold in practice */
	ret->attributes = attributes;
		
	start = end + 1;
	if (!(*start)) goto err;	
	else if (*start == '<')
	{
		ret->content = strdup ("");
		end = start;
	}
	else
	{	
		end = strchr (start, '<');
		if (!end) goto err;
	
		content = malloc (end - start + 1);
		strncpy (content, start, end - start);
		content[end - start] = '\0';
	
		ret->content = content;
	}	

	end++;
	if (*end++ != '/' || strncmp (end, name, strlen (name)) != 0 || end[strlen (name)] != '>') goto err;
	
	*endpoint = (end + strlen (name) + 1); /* Indicate where in the text the tag ended */
	
	return ret;

	err:
		fprintf (stderr, "Doesn't look like an HTML tag!\n");
		return NULL;
}

/* We're going to be extremely inefficient and realloc
 * every time we find a new attribute. This will suffice
 * for now but will become a problem with large 
 * documents */
struct html_tag_list * 
get_all_tags (char *text)
{	
	struct html_tag_list *tags = html_tag_list_init (NULL);
	char *cur = text, *next;
	do
	{
		struct html_tag *t = parse_tag (cur, &next);
		html_tag_list_add (tags, t);	
	}
	while (*(cur = next));
	
	return tags;
}

int
n_strings (char **strings) 
{
	int c = 0;
	while (*strings++)
	{
		c++;	
	}
	return c;
}

/* Returns false iff all the values of the area are 0 */
int
all_false (int *bools, int len)
{
	int i, cur = 0;
	for (i = 0; i < len; i++)
	{
		cur = (cur || bools[i]);
	}
	
	return !cur;
}

int ptr_comp (const void *c1, const void *c2)
{
	char *p1 = (char *) c1;
	char *p2 = (char *) c2;
		
	return p1 < p2 ? -1 : (p1 == p2 ? 0 : 1);
}

/* Returns all tags of each attribute. This seems like it'll be
 * really inefficient.
 * This function has been extremely difficult to write.
 * After writing it and having it work without a huge amount
 * of debugging, I feel like a master. */
struct html_tag_list *
find_tags_by_name (char *text, char **names)
{
	struct html_tag_list *full_list = html_tag_list_init (NULL); /* List that will hold all tags in order */
	int i, n_names = n_strings (names);

	/* Indicate which attributes have been exhausted i.e. those of which we will not find any more in TEXT */
	int *finished = calloc (n_names, sizeof (int)); 
	char **curs = calloc (n_names + 1, sizeof (char *)); /* Ensure NULL termination */
	char **next = calloc (n_names + 1, sizeof (char *));
	
	memset (finished, 0xff, n_names * sizeof (int)); /* Default value is true */
	for (i = 0; i < n_names; i++)
		curs[i] = text; /* every pointer begins at the beginning of the text */

	while (!all_false (finished, n_names))
	{	
		for (i = 0; i < n_names; i++)
		{
			if (curs[i])
				curs[i] = find_tag (curs[i], names[i]);
			if (!curs[i] || !(*curs[i]))
			{
				finished[i] = 0;
				if (all_false (finished, n_names)) /* Check if there are any more attributes to find */
					break;
			}
		
		}
		
		/* Sort the tags by the order in which they appear in the text,
			then add them according to that order */	
		qsort ((void *) &curs, n_names, sizeof (char *), ptr_comp);
		for (i = 0; i < n_names; i++)
		{
			struct html_tag *tag = parse_tag (curs[i], &next[i]);
			html_tag_list_add (full_list, tag);	
			curs[i] = next[i];
		}	
	}
	
	return full_list;
}

struct html_tag_list *
find_all_tags (char *text, char *name)
{
	struct html_tag_list *list = html_tag_list_init (NULL);
	char *p = text;
	if (!text) return NULL;
	while (p && *p)
	{
		char *next;
		struct html_tag *t;

		p = find_tag (p, name);
		if (!p) 
			break;

		t = parse_tag (p, &next);
		html_tag_list_add (list, t);	
		p = next;
	}
	
	return list;
}

/* Return a char * pointing to the beginning
 * of the first tag found of name NAME */

char *
find_tag (char *text, char *name)
{
	while (text && *text)
	{
		while (*text != '<')
			text++;
		if (*text == '\0') break;	
		text++;
		if (*text == '\0') break;	
	
		if (strncmp (text, name, strlen (name)) == 0 &&
			( *(text + strlen (name)) == ' ' || *(text + strlen (name)) == '>'))
		{	
			return text - 1;			
		}	
	}
	return NULL;
}

void
print_tag (const struct html_tag *tag)
{
	if (tag)
	{
		printf ("Name: \"%s\"\n", tag->name);
		hash_table_print (tag->attributes);
		printf ("Content: \"%s\"\n", tag->content);
	}
	else
		printf ("Tag is null\n");
	printf ("\n");
}

struct html_tag *
get_tags_by_name (char *name)
{
	return NULL;
}

struct html_tag *
get_next_tag (char *file_content, char *tag_name) 
{
	return NULL;	
}
