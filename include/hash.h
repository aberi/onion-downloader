#ifndef _HASH_
#define _HASH_

#include <stdio.h>

typedef struct hash_table_entry
{
	char *key;
	char *value;
	enum {
		OCCUPIED,
		EMPTY,
		DELETED
	} state;
} pair;
	
struct hash_table	
{
	pair *table;
	size_t size;
	size_t count;
};

typedef struct hash_table_iterator
{
	struct hash_table *table;
	pair *begin;
	pair *end;
	int current;
	pair *(*next)(struct hash_table *, int current);
} hash_iterator;

pair *advance (struct hash_table *, int current);

#define HASH_TABLE_COUNT(ht) (ht)->count
#define HASH_TABLE_SIZE(ht) (ht)->size

pair *init_pair (char *, char *);

char **keys (const struct hash_table *);
unsigned long hash (char *);
struct hash_table *hash_table_new (size_t);
int hash_table_put (struct hash_table *, char *, char *);
char *hash_table_get (const struct hash_table *, char *);
char *hash_table_remove (struct hash_table *, char *);
struct hash_table *hash_table_copy (struct hash_table *);

void hash_table_print (const struct hash_table *);
	
#endif
