#include <linkedList.h>
#include <defs.h>
#include <memoryManager.h>
#include <stdlib.h>

typedef struct LinkedListCDT {
	listNode *first;
	listNode *last;
	listNode *current;
	int len;
} LinkedListCDT;



LinkedList create_linked_list() {
	LinkedList list = (LinkedList) memory_manager_malloc(sizeof(LinkedListCDT));
	list->len = 0;
	list->first = NULL;
	list->last = NULL;
	list->current = NULL;
	return list;
}

listNode *append_element(LinkedList list, void *data) {
	if (list == NULL)
		return NULL;
	listNode *newlistNode = (listNode *) memory_manager_malloc(sizeof(listNode));
	newlistNode->data = data;
	return append_list_node(list, newlistNode);
}

listNode *append_list_node(LinkedList list, listNode *listNode) {
	if (list == NULL)
		return NULL;
	listNode->next = NULL;
	if (list->len > 0)
		list->last->next = listNode;
	else
		list->first = listNode;
	listNode->prev = list->last;
	list->last = listNode;
	list->len++;
	return listNode;
}

listNode *prepend_list_node(LinkedList list, listNode *listNode) {
	if (list == NULL)
		return NULL;
	listNode->prev = NULL;
	if (list->len > 0)
		list->first->prev = listNode;
	else
		list->last = listNode;
	listNode->next = list->first;
	list->first = listNode;
	list->len++;
	return listNode;
}

listNode *get_first(LinkedList list) {
	if (list == NULL)
		return NULL;
	return list->first;
}

int is_empty(LinkedList list) {
	if (list == NULL)
		return -1;
	return !list->len;
}

int get_length(LinkedList list) {
	if (list == NULL)
		return -1;
	return list->len;
}

void *remove_list_node(LinkedList list, listNode *listNode) {
	if (list == NULL || listNode == NULL)
		return NULL;

	if (list->first == listNode)
		list->first = listNode->next;
	else
		listNode->prev->next = listNode->next;

	if (list->last == listNode)
		list->last = listNode->prev;
	else
		listNode->next->prev = listNode->prev;

	list->len--;
	void *data = listNode->data;
	listNode->next = NULL;
	listNode->prev = NULL;
	return data;
}

void begin(LinkedList list) {
	if (list == NULL)
		return;
	list->current = list->first;
}

int has_next(LinkedList list) {
	if (list == NULL)
		return -1;
	return list->current != NULL;
}

void *next(LinkedList list) {
	if (!has_next(list))
		return NULL;
	void *data = list->current->data;
	list->current = list->current->next;
	return data;
}

void free_linked_list_deep(LinkedList list) {
	listNode *current = list->first;
	listNode *next;
	while (current != NULL) {
		next = current->next;
		free_memory_manager(current);
		current = next;
	}
	free_linked_list(list);
}

void free_linked_list(LinkedList list) {
	free_memory_manager(list);
}