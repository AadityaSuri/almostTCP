#define MAX_QUEUE_SIZE 100

typedef struct {
    int priority;
    char data [64];    
} QueueNode;

typedef struct {
    QueueNode heap[MAX_QUEUE_SIZE];
    int size;
} PriorityQueue;

PriorityQueue* createPriorityQueue();

void swapQueueNodes(QueueNode* a, QueueNode* b);

void heapify(PriorityQueue* priority_queue, int root);

int enqueue(PriorityQueue* priority_queue, int priority, char* data);

QueueNode dequeue(PriorityQueue* priority_queue);

int peak(PriorityQueue* priority_queue);

