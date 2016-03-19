#ifndef _pqueue_h
#define _pqueue_h

typedef struct node_t {
	struct node_t *right;
	void *data;
} node_t;

typedef int (*Compare)(const void *a, const void *b);

typedef struct pQueue {
	node_t *root;
	Compare cmp;
} pQueue;

pQueue *pQueue_create(Compare cmp);
void pQueue_delete(pQueue *pq);

void pQueue_push(pQueue *pq, void *data);
void *pQueue_pop(pQueue *pq);

#endif
