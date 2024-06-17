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



LinkedList createLinkedList() {
	LinkedList list = (LinkedList) allocMemory(sizeof(LinkedListCDT));
	list->len = 0;
	list->first = NULL;
	list->last = NULL;
	list->current = NULL;
	return list;
}

listNode *appendElement(LinkedList list, void *data) {
	if (list == NULL)
		return NULL;
	listNode *newlistNode = (listNode *) allocMemory(sizeof(listNode));
	newlistNode->data = data;
	return appendlistNode(list, newlistNode);
}

listNode *appendlistNode(LinkedList list, listNode *listNode) {
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

listNode *prependlistNode(LinkedList list, listNode *listNode) {
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

listNode *getFirst(LinkedList list) {
	if (list == NULL)
		return NULL;
	return list->first;
}

int isEmpty(LinkedList list) {
	if (list == NULL)
		return -1;
	return !list->len;
}

int getLength(LinkedList list) {
	if (list == NULL)
		return -1;
	return list->len;
}

void *removelistNode(LinkedList list, listNode *listNode) {
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
	// free(listNode);
	return data;
}

// Atención: Usar funciones de agregado/borrado cuando se itera sobre la lista
// puede causar comportamiento indefinido.
void begin(LinkedList list) {
	if (list == NULL)
		return;
	list->current = list->first;
}

int hasNext(LinkedList list) {
	if (list == NULL)
		return -1;
	return list->current != NULL;
}

void *next(LinkedList list) {
	if (!hasNext(list))
		return NULL;
	void *data = list->current->data;
	list->current = list->current->next;
	return data;
}

void freeLinkedListDeep(LinkedList list) {
	listNode *current = list->first;
	listNode *next;
	while (current != NULL) {
		next = current->next;
		free(current);
		current = next;
	}
	freeLinkedList(list);
}

void freeLinkedList(LinkedList list) {
	free(list);
}