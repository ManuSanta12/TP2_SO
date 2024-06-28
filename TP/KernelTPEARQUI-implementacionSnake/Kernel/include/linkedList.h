#ifndef _LINKED_LIST_H_
#define _LINKED_LIST_H_

typedef struct node_list_t *node_list_ptr;
typedef struct list_t *list_ptr;

list_ptr new_linked_list(int (*comp_funct)(void *, void *));

void add(list_ptr list, void *data);
int remove(list_ptr list, void *data);
void *find(list_ptr list, void *data, int (*comp_funct)(void *, void *));
void to_begin(list_ptr l);
int hasNext(list_ptr l);
void *next(list_ptr l);
void free_list(list_ptr list);
int size(list_ptr l);

#endif