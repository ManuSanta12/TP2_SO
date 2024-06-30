#ifndef LINKEDLIST_H
#define LINKEDLIST_H

typedef struct LinkedListCDT *LinkedListADT;

typedef struct ListNode {
	void *data;
	struct ListNode *prev;
	struct ListNode *next;
} ListNode;

LinkedListADT create_list();
ListNode *append(LinkedListADT list, void *data);
ListNode *append_node(LinkedListADT list, ListNode *node);
ListNode *prepend_node(LinkedListADT list, ListNode *node);
void *remove_node(LinkedListADT list, ListNode *node);
ListNode *get_first(LinkedListADT list);
int is_empty(LinkedListADT list);
void begin(LinkedListADT list);
int hasNext(LinkedListADT list);
void *next(LinkedListADT list);
void free_linked_list(LinkedListADT list);
void free_list(LinkedListADT list);
int get_len(LinkedListADT list);

#endif