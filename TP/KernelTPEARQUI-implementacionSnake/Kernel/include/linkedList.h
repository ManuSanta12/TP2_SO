#ifndef LINKEDLIST_H
#define LINKEDLIST_H

typedef struct LinkedListCDT *LinkedList;

typedef struct listNode {
	void *data;
	struct listNode *prev;
	struct listNode *next;
} listNode;


LinkedList create_linked_list();
listNode *append_element(LinkedList list, void *data);
listNode *append_list_node(LinkedList list, listNode *node);
listNode *prepend_list_node(LinkedList list, listNode *node);
listNode *get_first(LinkedList list);
int is_empty(LinkedList list);
int get_length(LinkedList list);
void *remove_list_node(LinkedList list, listNode *node);
void begin(LinkedList list);
int has_next(LinkedList list);
void *next(LinkedList list);
void free_linked_list_deep(LinkedList list);
void free_linked_list(LinkedList list);

#endif