// AUTOR: Daniel Rodriguez Sanchez
// AUTOR: Lucia Docampo Rodriguez
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "p2.h"



struct node {
    void *data;
    struct node *next;
};

void init_list(list *l) {
    *l = NULL;
}

int insert(list *l, void *datos) {

    if(*l == NULL) { // Lista vacia?
        *l = malloc(sizeof(struct node));
        (*l)->data = datos;
        (*l)->next = NULL;
    } else { // Lista no vacia?
        list aux = *l;
        while(aux->next != NULL) aux = aux->next;

        aux->next = malloc(sizeof(struct node));
        aux->next->data = datos;
        aux->next->next = NULL;
    }
    return 0;
}

pos first(list l) {
    return l;
}

pos next(list l, pos p) {
    if(p==NULL) return NULL;
    return p->next;
}

int end(list l, pos p) { // estamos al final?
    return p==NULL;
}

void *get(list l, pos p){
    if(p == NULL){
        return NULL;
    }
    return p->data;
}

void clear(list *l){
    list aux = *l;
    list aux_2;
    while(aux != NULL){
        aux_2 = aux->next;
        free(aux->data);
        free(aux);
        aux = aux_2;
    }
    *l = NULL;
}

void free_node(list *l, pos nodo_mem) {

    list aux = *l;
    if(aux == nodo_mem){
        *l = (*l)->next;
        free(nodo_mem->data);
        free(nodo_mem);
    }else {
        while (aux->next != nodo_mem) { //aux pasa a ser el nodo anterior al que queremos eliminar
            aux = aux->next;
        }
        aux->next = nodo_mem->next;
        free(nodo_mem->data);
        free(nodo_mem);
    }
}

void duplicar_nodo(list *l, pos nodo_mem){

    struct shared_info *nodo = get(*l, nodo_mem);
    struct shared_info *nodo2 = malloc(sizeof(struct shared_info));

    nodo2->address = nodo->address;
    strcpy(nodo2->date, nodo->date);
    nodo2->key = nodo->key;
    nodo2->size = nodo->size;

    insert(l, nodo2);
}
