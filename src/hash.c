#include "hash.h"
#include "utils.h"
#include "url.h"

#include <stdlib.h>
#include <string.h>

unsigned long
hash (char *key)
{
	int i;
	unsigned long ret = 0;
	size_t len = strlen (key);
	
	for (i = 0; i < len; i++)
	{
		ret = ret * 31 + key[i];		
	}
		
	return ret;
}

pair *
pair_new (void)
{
	return malloc (sizeof (pair));
}

pair *
init_pair (char *key, char *value)
{
	pair *p = pair_new();
	if (!p)
		return NULL;
	p->key = strdup (key);
	p->value = strdup (value);
	
	return p;
}

struct hash_table *
hash_table_new (size_t size)
{
	int i;
	struct hash_table *table = malloc (sizeof (struct hash_table));
		
	table->size = size;
	table->table = malloc (sizeof (pair) * size);
	
	for (i = 0; i < size; i++)
		table->table[i].state = EMPTY;

	if (!table->table)
		return NULL;
	table->count = 0;
		
	return table;
}

char *
hash_table_get (const struct hash_table *table, char *key)
{
	unsigned long hash_code = hash (key) % table->size;
	unsigned long index = hash_code;

	do 
	{
		pair current = table->table[index];
		switch (current.state)
			{
			case EMPTY:
				return NULL;
				break;
			case DELETED:
				if (strcmp (current.key, key) == 0) /* The key is present, but the value has been deleted */
					return NULL;
				break;
			case OCCUPIED:
				if (strcmp (current.key, key) == 0)
					return current.value;
				break;
		}
		index = (index + 1) % table->size;
	}
	while (index != hash_code);
	
	return NULL;
}

struct hash_table *
hash_table_copy (struct hash_table *rhs)
{
	int i;
	struct hash_table *ret = hash_table_new (rhs->size);
	
	for (i = 0; i < rhs->size; i++)
	{
		ret->table[i].key = xstrdup (rhs->table[i].key);	
		ret->table[i].value = xstrdup (rhs->table[i].value);	
		ret->table[i].state = rhs->table[i].state;
	}
	
	ret->count = rhs->count;
	ret->size = rhs->size;
	
	return ret;
}

/* Really clean code. Nice job */
int 
hash_table_put (struct hash_table *table, char *key, char *value)
{
	unsigned long hash_code = hash (key) % table->size;
	unsigned long index = hash_code;

	do 
	{
		pair *list = table->table;
		switch (list[index].state)
			{
			case EMPTY:
			case DELETED: 
				list[index].key = strdup (key);
				list[index].value = strdup (value);
				list[index].state = OCCUPIED;
				table->count++;
				return 0;
				break;
			case OCCUPIED:
				if (strcmp (key, list[index].key) == 0)
				{
					free (list[index].value);
					list[index].value = strdup (value);
					return 0;
				}
				else
					index = (index + 1) % table->size;
				break;
		}
	}
	while (index != hash_code);

	/* Hash table is full */
	
	return 0;
}

char *hash_table_remove (struct hash_table *table, char *key)
{
	unsigned long hash_code = hash (key) % table->size;
	unsigned long index = hash_code;
	pair *list = table->table;
	
	do 
	{
		switch (list[index].state)
		{	
			case EMPTY: /* Key not in hash table */
				return NULL;
				break;
			case DELETED:
				if (strcmp (key, list[index].key) == 0) /* Key found, but the entry has been deleted */
					return NULL;	
				/* Otherwise, if we haven't found the key yet, keep trying */
				break;
			case OCCUPIED:
				if (strcmp (key, list[index].key) == 0)
				{	
					char *value = strdup (list[index].value);
					free (list[index].value);
					list[index].value = NULL;
					list[index].state = DELETED;
	
					table->count--;
					return value;
				}
				break;
		}
	
		index = (index + 1) % table->size;
	}
	while (index != hash_code);

	/* Key not in hash table, every entry is DELETED or OCCUPIED */

	return NULL;
}
