#ifndef _PARSE_
#define _PARSE_

struct html_tag
{
	char *name;
	struct html_tag *next;
	struct hash_table *attributes;
	char *content;
};

/* is this really necessary ? */
struct html_tag_list
{
	struct html_tag *head;
	struct html_tag *tail; /* We want to be able to add directly to the end of the list. */
									
};

struct html_tag_list *get_all_tags (char *);
struct html_tag_list *html_tag_list_init (struct html_tag *tag);
int html_tag_list_add (struct html_tag_list *, struct html_tag *);

void print_all_tags (const struct html_tag_list *);
struct html_tag *get_tags_of_name (char *name);
char *build_html_tag (char *name, struct hash_table *table, char *content);
struct html_tag *parse_tag (const char *, char **);
void print_tag (const struct html_tag *);

#endif