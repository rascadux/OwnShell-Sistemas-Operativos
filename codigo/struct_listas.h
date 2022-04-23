#ifndef __STRUCT_LISTAS_H__
#define __STRUCT_LISTAS_H__

#include "list.h"
#define MAXIMO 1024

struct Otros{
    char ***env_main;
    int copia_stderr;
    char name[101];
};

struct Listas{
     list command_list;
     list malloc_list;
     list mmap_list;
     list shared_list;
     list background_list;
     struct Otros otros;
};

#endif
