// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <defs.h>
#include <linkedList.h>
#include <memoryManager.h>
#include <lib.h>

typedef struct LinkedListCDT {
	ListNode *first;
	ListNode *last;
	ListNode *current;
	int len;
} LinkedListCDT;

LinkedListADT createLinkedListADT() {
	LinkedListADT list = (LinkedListADT) memory_manager_malloc(sizeof(LinkedListCDT));
	list->len = 0;
	list->first = NULL;
	list->last = NULL;
	list->current = NULL;
	return list;
}

ListNode *appendElement(LinkedListADT list, void *data) {
	if (list == NULL)
		return NULL;
	ListNode *newNode = (ListNode *) memory_manager_malloc(sizeof(ListNode));
	newNode->data = data;
	return appendNode(list, newNode);
}

ListNode *appendNode(LinkedListADT list, ListNode *node) {
	if (list == NULL)
		return NULL;
	node->next = NULL;
	if (list->len > 0)
		list->last->next = node;
	else
		list->first = node;
	node->prev = list->last;
	list->last = node;
	list->len++;
	return node;
}

ListNode *prependNode(LinkedListADT list, ListNode *node) {
	if (list == NULL)
		return NULL;
	node->prev = NULL;
	if (list->len > 0)
		list->first->prev = node;
	else
		list->last = node;
	node->next = list->first;
	list->first = node;
	list->len++;
	return node;
}

ListNode *getFirst(LinkedListADT list) {
	if (list == NULL)
		return NULL;
	return list->first;
}

int isEmpty(LinkedListADT list) {
	if (list == NULL)
		return -1;
	return !list->len;
}

int getLength(LinkedListADT list) {
	if (list == NULL)
		return -1;
	return list->len;
}

void *removeNode(LinkedListADT list, ListNode *node) {
	if (list == NULL || node == NULL)
		return NULL;

	if (list->first == node)
		list->first = node->next;
	else
		node->prev->next = node->next;

	if (list->last == node)
		list->last = node->prev;
	else
		node->next->prev = node->prev;

	list->len--;
	void *data = node->data;
	node->next = NULL;
	node->prev = NULL;
	// free(node);
	return data;
}

// Atención: Usar funciones de agregado/borrado cuando se itera sobre la lista
// puede causar comportamiento indefinido.
void begin(LinkedListADT list) {
	if (list == NULL)
		return;
	list->current = list->first;
}

int hasNext(LinkedListADT list) {
	if (list == NULL)
		return -1;
	return list->current != NULL;
}

void *next(LinkedListADT list) {
	if (!hasNext(list))
		return NULL;
	void *data = list->current->data;
	list->current = list->current->next;
	return data;
}

void freeLinkedListADTDeep(LinkedListADT list) {
	ListNode *current = list->first;
	ListNode *next;
	while (current != NULL) {
		next = current->next;
		free_memory_manager(current);
		current = next;
	}
	freeLinkedListADT(list);
}

void freeLinkedListADT(LinkedListADT list) {
	free_memory_manager(list);
}