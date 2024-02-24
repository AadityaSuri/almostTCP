#define MAX_QUEUE_SIZE 100

typedef struct {
    int priority;
    packet_t packet;    
} QueueNode;

typedef struct {
    QueueNode heap[MAX_QUEUE_SIZE];
    int size;
} PriorityQueue;

PriorityQueue* createPriorityQueue();

void swapQueueNodes(QueueNode* a, QueueNode* b);

void heapify(PriorityQueue* priority_queue, int root);

int enqueue(PriorityQueue* priority_queue, int priority, packet_t packet);

QueueNode dequeue(PriorityQueue* priority_queue);

int peak(PriorityQueue* priority_queue);

