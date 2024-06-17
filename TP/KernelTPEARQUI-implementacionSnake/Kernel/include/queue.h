#ifndef QUEUE_H
#define QUEUE_H

#include <defs.h>
#include <memoryManager.h>

BlockedQueueADT new_queue();
pid_t dequeue_pid(BlockedQueueADT queue);
void enqueue_pid(BlockedQueueADT queue, pid_t pid);
void free_queue(BlockedQueueADT queue);


#endif