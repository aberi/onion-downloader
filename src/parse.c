#include "parse.h"
#include "hash.h"
#include "utils.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define TAG_START_FMT "<%s"
#define TAG_END_FMT "</%s>"

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
	char *start = strchr (tag, '<');

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
	int size = 0;
	char *cur = text, *next;
	do
	{
		struct html_tag *t = parse_tag (cur, &next);
		html_tag_list_add (tags, t);	
	}
	while (*(cur = next));
	
	return tags;
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