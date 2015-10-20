#ifndef _QUEUE_
#define _QUEUE_

#include "url.h"
#include "types.h"
#include "utils.h"

/* Just a linked list of URLs. We need both a head and a tail
 * for LIFO behavior to work. */
struct url_queue
{
	struct url *head;
	struct url *tail;
	int size;
};

struct url_queue *url_queue_init (void);
int url_queue_is_empty (const struct url_queue *);
struct url *dequeue (struct url_queue *);
int enqueue (struct url_queue *, struct url *url);
void print_queue (const struct url_queue *);

#endif
