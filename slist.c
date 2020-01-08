#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"slist.h"

//implement the functions in slist.h

void slist_print(slist_t* list)
{
	slist_node_t *temp;
	if (list == NULL)
		return;
	printf("the list of output:\n");
	for (temp = slist_head(list); temp != NULL; temp = slist_next(temp))
	{
		printf("%s ", (char*)(slist_data(temp)));
	}
	printf("\n");
}
//----------------------------------------------------------
void slist_init(slist_t * l)
{
	slist_head(l) = NULL;
	slist_tail(l) = NULL;
	slist_size(l) = 0;
}
//----------------------------------------------------------
void slist_destroy(slist_t * l, slist_destroy_t flag)
{
	slist_node_t* tmp;
	if (l == NULL)
		return;
	while (slist_head(l))
	{
		tmp = slist_head(l);
		slist_head(l) = slist_next(slist_head(l));
		if (flag == SLIST_FREE_DATA)
			free(slist_data(tmp));
		free(tmp);
	}
	free(l);
}
//----------------------------------------------------------
void *slist_pop_first(slist_t * l)
{
	slist_node_t* tmp;
	void* date_temp;
	if (l == NULL)
		return NULL;
	tmp = slist_head(l);
	if (tmp == NULL)
		return NULL;
	date_temp = tmp->data;
	slist_head(l) = slist_next(slist_head(l));
	free(tmp);
	slist_size(l)--;
	return date_temp;
}
//----------------------------------------------------------
int slist_append(slist_t * l, void * d)
{
	slist_node_t* new_node;
	if (l == NULL)
		return -1;
	new_node = (slist_node_t*)malloc(sizeof(slist_node_t));
	if (new_node == NULL)
	{
	    perror("failed malloc node\n");
	    return -1;
	}
	slist_data(new_node) = d;
	slist_next(new_node) = NULL;
	if (slist_head(l) == NULL)
	{
		slist_head(l) = new_node;
	}
	else slist_next(slist_tail(l)) = new_node;
	slist_tail(l) = new_node;
	slist_size(l)++;

	return 0;
}
//----------------------------------------------------------
int slist_prepend(slist_t * l, void *d)
{
	slist_node_t* new_node;
	if (l == NULL)
		return -1;
	new_node = (slist_node_t*)malloc(sizeof(slist_node_t));
	if (new_node == NULL)
    {
        perror("failed malloc node\n");
        return -1;
    }
	slist_data(new_node) = d;
	slist_next(new_node) = slist_head(l);

	slist_head(l) = new_node;
	slist_size(l)++;

	return 0;
}
//----------------------------------------------------------
int slist_append_list(slist_t* l1, slist_t* l2)
{
	slist_node_t* tmp = slist_head(l2);
	while (tmp)
	{
		if (slist_append(l1, slist_data(tmp)) < 0)
			return -1;
		tmp = slist_next(tmp);
	}
	slist_tail(l1) = slist_tail(l2);
	return 0;
}
