#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H

/**
 * @file priorityqueue.h
 * @brief Definitions for a priority queue.
 */

#include "packet.h"

#define MAX_QUEUE_SIZE 100 /**< Maximum size of the priority queue. */

/**
 * @struct QueueNode
 * @brief Structure representing a node in the priority queue.
 */
typedef struct {
    int priority;
    packet_t packet; 
} QueueNode;

/**
 * @struct PriorityQueue
 * @brief Structure representing a priority queue.
 */
typedef struct {
    QueueNode heap[MAX_QUEUE_SIZE]; /**< Array representing the heap structure of the priority queue. */
    int size;                        /**< Current size of the priority queue. */
} PriorityQueue;

/**
 * @brief Creates a priority queue.
 * 
 * @return A pointer to the newly created priority queue.
 */
PriorityQueue* createPriorityQueue();

/**
 * @brief Swaps two queue nodes.
 * 
 * @param a Pointer to the first queue node.
 * @param b Pointer to the second queue node.
 */
void swapQueueNodes(QueueNode* a, QueueNode* b);

/**
 * @brief Heapifies the subtree rooted at the given node.
 * 
 * @param priority_queue The priority queue.
 * @param root The index of the root of the subtree to heapify.
 */
void heapify(PriorityQueue* priority_queue, int root);

/**
 * @brief Enqueues a new element with the given priority into the priority queue.
 * 
 * @param priority_queue The priority queue.
 * @param priority The priority of the element to enqueue.
 * @param data The data associated with the element.
 * @return -1 if the queue is full, otherwise 0.
 */
int enqueue(PriorityQueue* priority_queue, int priority, packet_t packet);

/**
 * @brief Dequeues the element with the highest priority from the priority queue.
 * 
 * @param priority_queue The priority queue.
 * @return The dequeued node.
 */
QueueNode dequeue(PriorityQueue* priority_queue);

/**
 * @brief Returns the priority of the element with the highest priority in the priority queue.
 * 
 * @param priority_queue The priority queue.
 * @return The priority of the element at the front of the queue, or -1 if the queue is empty.
 */
int peak(PriorityQueue* priority_queue);

#endif