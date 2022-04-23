// AUTOR: Daniel Rodriguez Sanchez
// AUTOR: Lucia Docampo Rodriguez

#ifndef __LIST_H__
#define __LIST_H__

typedef struct node *list;
typedef struct node *pos;

void init_list(list *);
int insert(list *, void *);
pos first(list);
pos next(list, pos);
int end(list, pos);
void *get(list ,pos);
void clear(list *);
void free_node(list *l, pos);
void duplicar_nodo(list *l, pos);

#endif
