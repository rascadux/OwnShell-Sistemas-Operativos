// AUTOR: Daniel Rodriguez Sanchez
// AUTOR: Lucia Docampo Rodriguez

#ifndef __P2_H__
#define __P2_H__
#include "struct_listas.h"

struct shared_info{
    void *address;
    key_t key;
    ssize_t size;
    char date[101];
};

void tiempo_actual(char []);

int malloc_fun(char *[], int, struct Listas *);
int mmap_fun(char *[], int, struct Listas *);
int shared_fun(char *[], int, struct Listas *);
int dealloc_fun(char *[], int, struct Listas *);
int memoria(char *[], int, struct Listas *);
int volcarmem(char *[], int, struct Listas *);
int llenarmem(char *[], int, struct Listas *);
int recursiva(char *[], int, struct Listas *);
int readfich(char *[], int, struct Listas *);
int writefich(char *[], int, struct Listas *);

#endif
