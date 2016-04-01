/**
 * @file pqueue.h
 * @brief Priority queue API
 * @author Yahor Zabolotski
 * @date 2016-03-31
 */

#ifndef _pqueue_h
#define _pqueue_h

/**
 * @brief This structure uses as pqueue node. 
 */
typedef struct node_t {
	struct node_t *right;	/**< pointer to the right node in the pqueue */
	void *data;				/**< pointer to the info */
} node_t;

/**
 * @brief The cmp func signature, which uses to order pqueue.   
 *
 * @param a pointer to a data field in the node_t
 * @param b pointer to a data field in the node_t
 *
 * @return 1 if *a > *b; 0 if *a == *b, -1 either
 */
typedef int (*Compare)(const void *a, const void *b);

typedef struct pQueue {
	node_t *root;	/**< pointer to the first element into pqueue */
	Compare cmp;	/**< pointer to cmp func for the data */
} pQueue;

/**
 * @brief create priority queue
 *
 * @param cmp compare function
 *
 * @return pointer to queue
 */
pQueue *pQueue_create(Compare cmp);


/**
 * @brief remove priority queue
 *
 * @param pq pointer to queue
 */
void pQueue_delete(pQueue *pq);

/**
 * @brief push data to the priority queue pq
 *
 * @param pq pointer to the priority queue
 * @param data pointer to info, which will be pushed
 */
void pQueue_push(pQueue *pq, void *data);


/**
 * @brief pop data from the priority queue pq
 *
 * @param pq pointer to the priority queue
 *
 * @return pointer to a data, if pq not empty, either NULL
 */
void *pQueue_pop(pQueue *pq);

#endif
