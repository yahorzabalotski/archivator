#include <stdlib.h>
#include "dbg.h"

#include "pqueue.h"

static node_t *node_t_create(void *data)
{
	node_t *node = malloc(sizeof(*node));

	node->right	= NULL;
	node->data 	= data;

	return node;
}

static void node_t_delete(node_t *node)
{
	free(node);
}

pQueue *pQueue_create(Compare cmp)
{
	pQueue *pq = malloc(sizeof(*pq));
	if(pq == NULL) {
		log_info("Can't allocate memory.");
		return NULL;
	}

	pq->root = NULL;
	pq->cmp = cmp;

	return pq;
}

void pQueue_delete(pQueue *pq)
{
	node_t *tmp = NULL;
	node_t *root = pq->root;

	while(root) {
		tmp = root;
		root = root->right;
		node_t_delete(tmp);
	}

	free(pq);
}

void pQueue_push(pQueue *pq, void *data)
{
	if(!pq->root) {
		pq->root = node_t_create(data);
		return;
	}

	node_t *node = node_t_create(data);
	node_t *tmp = pq->root;
	node_t *prev = NULL;

	if(pq->cmp(tmp->data, data) > 0) {
		node->right = tmp;
		pq->root = node;
		return;
	}

	prev = tmp;
	tmp = tmp->right;

	while(tmp) {
		if(pq->cmp(tmp->data, data) < 0) {
			prev = tmp;
			tmp = tmp->right;
		} else {
			prev->right = node;
			node->right = tmp;
			break;
		}
	}
	
	if(!tmp) {
		prev->right = node;
	}
}


void *pQueue_pop(pQueue *pq)
{
	if(!pq->root) {
		return NULL;
	}

	void *data = pq->root->data;
	node_t *tmp = pq->root->right;

	node_t_delete(pq->root);

	pq->root = tmp;

	return data;
}
