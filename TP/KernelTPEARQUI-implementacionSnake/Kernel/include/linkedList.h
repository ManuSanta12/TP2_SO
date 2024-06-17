#ifndef LINKEDLIST_H
#define LINKEDLIST_H

typedef struct LinkedListCDT *LinkedList;

typedef struct listNode {
	void *data;
	struct listNode *prev;
	struct listNode *next;
} listNode;

LinkedList createLinkedList();
listNode *appendElement(LinkedList list, void *data);
listNode *appendlistNode(LinkedList list, listNode *listNode);
listNode *prependlistNode(LinkedList list, listNode *listNode);
void *removelistNode(LinkedList list, listNode *listNode);
listNode *getFirst(LinkedList list);
int isEmpty(LinkedList list);
void begin(LinkedList list);
int hasNext(LinkedList list);
void *next(LinkedList list);
void freeLinkedListDeep(LinkedList list);
void freeLinkedList(LinkedList list);
int getLength(LinkedList list);

#endif