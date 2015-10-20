#include "parse.h"
#include "hash.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int
html_tag_list_is_empty (const struct html_tag_list *list)
{
	if (list)
		return !list->head;
	return 0;
}

struct html_tag *
html_tag_list_remove_head (struct html_tag_list *list)
{
	if (list && list->head)
	{
		struct html_tag *ret = xnew (struct html_tag), *temp;
		memcpy (ret, list->head, sizeof (struct html_tag));
		temp = list->head;
		list->head = list->head->next;
		free (temp);
		list->count--;
		return ret;
	}
	else return NULL;
}

struct html_tag_list *
merge_lists (struct html_tag_list *l1, struct html_tag_list *l2)
{
	while (!html_tag_list_is_empty (l2))
	{
		struct html_tag *tag = html_tag_list_remove_head (l2);
		html_tag_list_add (l1, tag);
	}
	
	return l1;
}

/* List can be initialized with a NULL node or what is effectively
 * an existing list */
struct html_tag_list *
html_tag_list_init (struct html_tag *tag)
{
	struct html_tag_list *list = xnew (struct html_tag_list);
	list->tail = list->head = tag;
	if (tag) tag->next = NULL;
	list->count = 0;
	
	return list;
}

void
print_all_tags (const struct html_tag_list *list)
{
	struct html_tag *cur = list->head;
	while (cur)
	{
		print_tag (cur);
		cur = cur->next;
	}
}

void
print_all_attribute (const struct html_tag_list *list, char *attr_name, int (*filter)(const char *))
{
	struct html_tag *cur = list->head;
	while (cur)
	{
		char *attr_value = hash_table_get (cur->attributes, attr_name);
		int can_print = filter ? filter (attr_value) : 1;
		if (attr_value && can_print)
			printf ("%s=%s\n", attr_name, attr_value);
		cur = cur->next;
	}
}

char **
get_all_attribute (const struct html_tag_list *list, char *attr_name)
{
	char **values = calloc (list->count, sizeof (char *));
	int count = 0;
	struct html_tag *cur = list->head;
	while (cur)
	{
		char *attr_value = hash_table_get (cur->attributes, attr_name);
		if (attr_value)
			values[count++] = strdup (attr_value);
		cur = cur->next;
	}
	return values;
}

int html_tag_list_add (struct html_tag_list *list, struct html_tag *tag)
{
	if (tag) 
	{
		tag->next = NULL;
		if (list->tail) 
		{
			list->tail->next = tag;
			list->tail = tag;
		}
		else
			list->head = list->tail = tag;
		list->count++;
	}
	return 0;
}

