#include "parse.h"
#include "hash.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* List can be initialized with a NULL node or what is effectively
 * an existing list */
struct html_tag_list *
html_tag_list_init (struct html_tag *tag)
{
	struct html_tag_list *list = xnew (struct html_tag_list);
	list->tail = list->head = tag;
	if (tag) tag->next = NULL;
	
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
	}
	return 0;
}

