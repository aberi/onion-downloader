#include "queue.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>

/* Insert at the head of the list, remove from the tail */

int
is_in_queue (const struct url_queue *queue, struct url *url)
{
	struct url *cur = queue->head;
		
	while (cur)
	{
		if (are_the_same (url, cur))
			return 1;	
		cur = cur->next;
	}
	return 0;
}

struct url_queue *
url_queue_init (void)
{
	struct url_queue *queue = calloc (1, sizeof (struct url_queue));
	queue->size = 0; /* Unnecessary but oh well */
	queue->tail = queue->head = NULL;
	return queue;
}

int
url_queue_is_empty (const struct url_queue *q)
{
	return q->size == 0;
}

struct url *
dequeue (struct url_queue *q)
{
	struct url *ret;
	if (q && (ret = q->head))
	{
		q->head = q->head->next;
		if (q->head == NULL) /* If the list is now empty, the tail as well as the head must change */
			q->tail = NULL; 
		q->size--;
		return ret;
	}
	return NULL;
}

int
enqueue (struct url_queue *q, struct url *url)
{
	struct url *local_url = calloc (1, sizeof (struct url));

	if (!q) return -1;
	if (!url) return 0;

	if (!local_url)
	{
		perror ("Unable to make a local copy of the URL\n");
		exit (1);
	}
	memcpy (local_url, url, sizeof (struct url)); /* shallow copy of the url */

	if (q->tail) /* The tail will only be NULL when the head is also NULL */
	{
		q->tail->next = local_url;
		q->tail = q->tail->next;
	}
	else
	{	
		q->head = q->tail = local_url;
		q->head->next = NULL;
	}
	q->size++;
	return 0;
}


void
print_queue (const struct url_queue *queue, int n)
{
	int c = 0;
	if (queue)
	{
		struct url *u = queue->head;
		while (u && c < n)
		{
			printf ("\nURL #%d:\n", ++c);
			printf ("%s\n", u->full_url);
			u = u->next;
		}
	}
	printf ("\n\n");
}


