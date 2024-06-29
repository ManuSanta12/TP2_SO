#ifndef LINKEDLIST_H
#define LINKEDLIST_H

typedef struct LinkedListCDT *LinkedListADT;

typedef struct ListNode {
	void *data;
	struct ListNode *prev;
	struct ListNode *next;
} ListNode;

LinkedListADT createLinkedListADT();
ListNode *appendElement(LinkedListADT list, void *data);
ListNode *appendNode(LinkedListADT list, ListNode *node);
ListNode *prependNode(LinkedListADT list, ListNode *node);
void *removeNode(LinkedListADT list, ListNode *node);
ListNode *getFirst(LinkedListADT list);
int isEmpty(LinkedListADT list);
void begin(LinkedListADT list);
int hasNext(LinkedListADT list);
void *next(LinkedListADT list);
void freeLinkedListADTDeep(LinkedListADT list);
void freeLinkedListADT(LinkedListADT list);
int getLength(LinkedListADT list);

#endif