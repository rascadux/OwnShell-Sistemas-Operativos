// AUTOR: Daniel Rodriguez Sanchez
// AUTOR: Lucia Docampo Rodriguez

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <ctype.h>
#include "list.h"
#include "struct_listas.h"

#define TAMANO 4096
#define LEERCOMPLETO ((ssize_t)-1)

/*
* Acabar mmap_fun
*
*/

int ej1, ej2, ej3;

struct malloc_info{ //Lista para memoria
    void *address;
    ssize_t size;
    char date[101];
};

struct mmap_info{
    void *address;
    ssize_t size;
    char date[101];
    char fname[100];
    int descriptor;
};

struct shared_info{
    void *address;
    key_t key;
    ssize_t size;
    char date[101];
};

void tiempo_actual(char tiempo[]){
    time_t t = time(NULL);
    struct tm *tx = localtime(&t);
    char time[100];
    strftime(time, 100, "%b %d %H:%M", tx);
    strcpy(tiempo, time);
}

//  M A L L O C ------------------------------------------------------------------------------------------------------

pos search_size(list *malloc_list, ssize_t size){
    for(pos p = first(*malloc_list); !end(*malloc_list, p); p = next(*malloc_list, p)) {
        struct malloc_info *nodo = get(*malloc_list, p);
        if(nodo->size == size){
            return p;
        }
    }
    return NULL;
}

void print_malloc(struct Listas *lista){

    for(pos p = first(lista->malloc_list); !end(lista->malloc_list, p); p = next(lista->malloc_list, p)) {//Recorre la lista imprimiendo
        struct malloc_info *nodo = get(lista->malloc_list, p);
        printf("%20p %16ld %12s %25s\n", nodo->address, nodo->size, nodo->date, "malloc");
    }
}

void malloc_free(char *tokens, struct Listas *lista){
    if(tokens == NULL){
        printf("******Lista de bloques asignados malloc para el proceso %d\n", getpid());
        print_malloc(lista);
        return;
    }

    ssize_t size = (ssize_t) atoi(tokens);

    if(size<=0){
        printf("No se asignan bloques de 0 bytes \n");
        return;
    }
    pos nodo_mem;
    if((nodo_mem = search_size(&lista->malloc_list, size)) == NULL){
        printf("No hay bloque de ese tamaño asignado con malloc\n");
    }else{
        free_node(&lista->malloc_list, nodo_mem);
    }
}


int malloc_fun(char *tokens[], int ntokens, struct Listas *lista){

    struct malloc_info *nodo = malloc(sizeof(struct malloc_info));

    if(tokens[1]==NULL){
        printf("******Lista de bloques asignados malloc para el proceso %d\n", getpid());
        print_malloc(lista);
        return 0;
    }

    if (strcmp(tokens[1], "-free") == 0){

        malloc_free(tokens[2], lista);
        return 0;

    }else{

        ssize_t size = (ssize_t) atoi(tokens[1]); //si se mete una letra es -1
        char *address; // donde se guarda la direccion de memoria a imprimir

        if(size <= 0){ //Si se metio una letra o un numero negativo
            printf("No se asignan bloques de 0 bytes\n");
            return 0;
        }
        if((address = malloc(sizeof(size))) == NULL){//si malloc da algun fallo
            printf("Imposible obtener memoria con malloc: %s\n", strerror(errno));
        }else{
            printf("Asignados %zu bytes en %p\n", size, address);
            // address
            nodo->address = address;
            // date
            char time[100];
            tiempo_actual(time);
            strcpy(nodo->date, time);
            // size
            nodo->size = size;

            insert(&lista->malloc_list, nodo); //guardo en la lista
        }
    }
    return 0;
}

//  M M A P------------------------------------------------------------------------------------------------------

void *mmapFichero(char *fichero, int protection, int *descr){ //descr es el int descriptor que quiero obtener para cuando
    int map=MAP_PRIVATE,modo=O_RDONLY;                       //haga el close y para guardar en mem->descriptor
    struct stat s;
    void *p;

    if(protection&PROT_WRITE){
        modo = O_RDWR;
    }
    if(stat(fichero,&s) == -1 || (*descr = open(fichero, modo)) == -1){
        return NULL;
    }
    if ((p = mmap(NULL, s.st_size, protection, map, *descr,0)) == MAP_FAILED){
        printf("Error al mapear: %s\n", strerror(errno));
        return NULL;
    }
    //Guardar Direccion de Mmap (p, s.st_size,fichero,descr......);
    return p;
}

void print_mmap(struct Listas *lista){
    for(pos p_node = first(lista->mmap_list); !end(lista->mmap_list, p_node); p_node = next(lista->mmap_list, p_node)){
        struct mmap_info *mem = get(lista->mmap_list, p_node);
        printf("%20p %16ld %12s %11s %4d %8s\n", mem->address, mem->size, mem->date, mem->fname, mem->descriptor, "mmap");
    }
}

pos search_mmap(list *l, char *name){
    for(pos p = first(*l); !end(*l, p); p = next(*l, p)){
        struct mmap_info *nodo = get(*l, p);
        if(strcmp(nodo->fname, name) == 0){
            return p;
        }
    }
    return NULL;
}

void mmap_free(char *tokens, struct Listas *lista){

    if(tokens == NULL){
        printf("******Lista de bloques asignados mmap para el proceso %d\n", getpid());
        print_mmap(lista);
        return;
    }

    pos nodo_mem = search_mmap(&lista->mmap_list, tokens); //Guardo en nodo_mem el nodo a eliminar
    if (nodo_mem == NULL){ //COMPRUEBA SI ESTA EN LA LISTA
        printf("Fichero %s no mapeado\n", tokens);
        return;
    }

    struct mmap_info *nodo = get(lista->mmap_list, nodo_mem);

    if(munmap(nodo->address, nodo->size) != 0){ //DESMAPEA EL FICHERO
        printf("Error al desmapear fichero: %s\n", strerror(errno));
    }
    if(close(nodo->descriptor) != 0){
        printf("Error al cerrar fichero: %s\n", strerror(errno));
    }

    free_node(&lista->mmap_list, nodo_mem);
}

int mmap_fun(char *tokens[], int ntokens, struct Listas *lista){
    void *p;
    int protection=0, descr;

    struct mmap_info *mem = malloc(sizeof(struct mmap_info));

    if(tokens[1] == NULL){
        printf("******Lista de bloques asignados mmap para el proceso %d\n", getpid());
        print_mmap(lista);
        //IMPRIME LA LISTA MMAP_LIST
        return 0;
    }

    if (strcmp(tokens[1], "-free") == 0){// Si se pide con -free
        mmap_free(tokens[2], lista);
        return 0;
    }else{// Si no se pide el -free

        if(tokens[2] != NULL && strlen(tokens[2])<4){
            if (strchr(tokens[2],'r')!=NULL) protection|=PROT_READ;
            if (strchr(tokens[2],'w')!=NULL) protection|=PROT_WRITE;
            if (strchr(tokens[2],'x')!=NULL) protection|=PROT_EXEC;
        }

        if((p = mmapFichero(tokens[1], protection, &descr)) == NULL){
            printf("Imposible mapear fichero\n");
        }else {
            struct stat s;
            if(stat(tokens[1],&s) == -1){
                printf("Error al obtener datos del fichero\n");
            }
            printf ("fichero %s mapeado en %p\n", tokens[1], p);
            //address
            mem->address = p;
            //date
            char time[100];
            tiempo_actual(time);;
            strcpy(mem->date, time);
            //size
            mem->size = s.st_size;
            //name
            strcpy(mem->fname, tokens[1]);
            //descriptor
            mem->descriptor = descr;

            insert(&lista->mmap_list, mem);
        }
    }
    return 0;
}

//  S H A R E D ------------------------------------------------------------------------------------------------------

void print_shared(struct Listas *lista){

    for(pos p_node = first(lista->shared_list); !end(lista->shared_list, p_node); p_node = next(lista->shared_list, p_node)){
        struct shared_info *mem = get(lista->shared_list, p_node);
        printf("%20p %16ld %12s       (key %2d) %10s\n", mem->address, mem->size, mem->date,  mem->key, "shared");
    }
}

void * ObtenerMemoriaShmget (key_t clave, size_t tam){ //Obtienen un puntero a una zaona de memoria compartida
//si tam >0 intenta crearla y si tam==0 asume que existe
    void * p;
    int aux,id,flags=0777;
    struct shmid_ds s;


    if (tam) //si tam no es 0 la crea en modo exclusivo esta funcion vale para shared y shared -create
        flags=flags | IPC_CREAT | IPC_EXCL;
    //si tam es 0 intenta acceder a una ya creada

    if (clave == IPC_PRIVATE) {
        errno = EINVAL;
        return NULL;
    };

    if ((id=shmget(clave, tam, flags))==-1) {
        return (NULL);
    }

    if ((p=shmat(id,NULL,0))==(void*) -1){
        aux=errno; //si se ha creado y no se puede mapear
        if (tam) //*se borra
            shmctl(id,IPC_RMID,NULL);
        errno=aux;
        return (NULL);
    }
    shmctl (id,IPC_STAT,&s);


   //Guardar En Direcciones de Memoria Shared (p, s.shm_segsz, clave.....);
    return (p);
}

void SharedCreate (char *tokens[], struct Listas *lista){ //tokens[2] is the key and tokens[3] is the size
     key_t k;
     ssize_t tam=0;
     void *p;
     if (tokens[2]==NULL || tokens[3]==NULL){//Listar Direcciones de Memoria Shared
           printf("******Lista de bloques asignados shared para el proceso %d\n", getpid());
           print_shared(lista);
           return;
     }

     k=(key_t) atoi(tokens[2]);
     if (tokens[3] != NULL){
          tam = (ssize_t) atoll(tokens[3]);
          if(tam <= 0){
              printf("No se asignan bloques de 0 bytes\n");
              return;
          }
     }
     if ((p = ObtenerMemoriaShmget(k,tam)) == NULL){
          perror ("Imposible obtener memoria shmget\n");
     } else {
          printf ("Memoria de shmget de clave %d asignada en %p\n", k, p);
          struct shared_info *shared_mem = malloc(sizeof(struct shared_info));

          shared_mem->address = p;
          shared_mem->key = k;
          shared_mem->size = tam;
          char time[100];
          tiempo_actual(time);
          strcpy(shared_mem->date, time);

          insert(&lista->shared_list, shared_mem);
     }

}

pos search_shared(list *shared_list, key_t key){

    for(pos p = first(*shared_list); !end(*shared_list, p); p = next(*shared_list, p)) {
        struct shared_info *nodo = get(*shared_list, p);
        if(nodo->key == key){
            return p;
        }
    }
    return NULL;
}


void SharedDelkey(char *token){//token es un puntero que apunta a un str que contiene la key
    key_t clave;
    int id;
    char *key = token;

    if(key == NULL || (clave =(key_t) strtoul(key, NULL, 10)) == IPC_PRIVATE){
        printf("   shared -delkey clave_valida\n");
        return;
    }
    if ((id=shmget(clave,0,0666))==-1){
        perror("shmget: imposible obtener memoria compartida");
        return;
    }
    if(shmctl(id, IPC_RMID, NULL) == -1){
        perror ("shmctl: imposible eliminar memoria compartida\n");
    }
}

void shared_free(char *token, struct Listas *lista){
    if(token == NULL){
        printf("******Lista de bloques asignados shared para el proceso %d\n", getpid());
        print_shared(lista);
        return;
    }
    key_t k=(key_t) atoi(token);
    pos nodo_mem = search_shared(&lista->shared_list, k); //Guardo en nodo_mem el nodo a eliminar
    if (nodo_mem == NULL){ //COMPRUEBA SI ESTA EN LA LISTA
        printf("No hay bloque de esa clave mapeado en el fichero\n");
        return;
    }
    free_node(&lista->shared_list, nodo_mem);
}

int shared_fun(char *tokens[], int ntokens, struct Listas *lista){

    if(tokens[1] == NULL){
        printf("******Lista de bloques asignados shared para el proceso %d\n", getpid());
        print_shared(lista);
        //IMPRIME LA LISTA SHARED_LIST
        return 0;
    }

    if(strcmp(tokens[1], "-create") == 0){
        SharedCreate(tokens, lista);
    }else if(strcmp(tokens[1], "-free") == 0){
        shared_free(tokens[2], lista);
    }else if(strcmp(tokens[1], "-delkey") == 0){
        SharedDelkey(tokens[2]);
    }else{
        pos nododup;
        key_t k = (key_t) atoi(tokens[1]);
        if((nododup = search_shared(&lista->shared_list, k)) == NULL){
            printf("Imposible asignar memoria compartida con clave %d, no esta en la lista\n", k);
            return 0;
        }
        duplicar_nodo(&lista->shared_list, nododup);
        return 0;
    }
    return 0;
}

//  D E A L L O C ------------------------------------------------------------------------------------------------------

pos search_addr(void *addr, struct Listas *lista, int *tipo_lista){
    for(pos p = first(lista->malloc_list); !end(lista->malloc_list, p); p = next(lista->malloc_list, p)) {
        struct malloc_info *nodo = get(lista->malloc_list, p);
        if(nodo->address == addr){
            *tipo_lista = 1;
            return p;
        }
    }
    for(pos p_mmap = first(lista->mmap_list); !end(lista->mmap_list, p_mmap); p_mmap = next(lista->mmap_list, p_mmap)) {
        struct malloc_info *nodo = get(lista->mmap_list, p_mmap);
        if(nodo->address == addr){
            *tipo_lista = 2;
            return p_mmap;
        }
    }
    for(pos p_shared = first(lista->shared_list); !end(lista->shared_list, p_shared); p_shared = next(lista->shared_list, p_shared)) {
        struct malloc_info *nodo = get(lista->shared_list, p_shared);
        if(nodo->address == addr){
            *tipo_lista = 3;
            return p_shared;
        }
    }
    return NULL;
}

int dealloc_fun(char *tokens[], int ntokens, struct Listas *lista){

     //struct mmap_info *mem = malloc(sizeof(struct mmap_info));

     if(tokens[1] == NULL){
          printf("******Lista de bloques asignados para el proceso %d\n", getpid());
          print_malloc(lista);
          print_mmap(lista);
          print_shared(lista);

          return 0;
     }

     if(strcmp(tokens[1], "-malloc") == 0){
          malloc_free(tokens[2], lista); //Elimina el nodo de ese tamaño de la lista
     }else if(strcmp(tokens[1], "-mmap") == 0){
          mmap_free(tokens[2], lista); //Elimina el nodo con ese nombre de la lista
     }else if(strcmp(tokens[1], "-shared") == 0){
         shared_free(tokens[2], lista); //Elimina el nodo con esa key de la lista
     }else{
         void *addr = (void *) strtoul(tokens[1], NULL, 16);
         pos nodo_addr;
         int tipo_lista = 0; //Asigno un numero segun la lista en la que se encuentre: MALLOC=1 ::: MMAP=2 ::: SHARED=3
         if((nodo_addr = search_addr(addr, lista, &tipo_lista)) == NULL){
             printf("Direccion %s no asignada con malloc, shared o mmap\n", tokens[1]);
             return 0;
         }else{
             if(tipo_lista == 1){
                 free_node(&lista->malloc_list, nodo_addr);
             }else if(tipo_lista == 2){
                 free_node(&lista->mmap_list, nodo_addr);
             }else if(tipo_lista == 3){
                 free_node(&lista->shared_list, nodo_addr);
             } //Si entra aqui es que esta en una de las 3 listas asi que no hace falta comprobar mas casos
         }
     }

     return 0;
}

//  M E M O R I A ---------------------------------------------------------------------------------------------------------

void dopmap (void) /*no arguments necessary*/
{ pid_t pid; /*ejecuta el comando externo pmap para */
    char elpid[32]; /*pasandole el pid del proceso actual */
    char *argv[3]={"pmap",elpid,NULL};
    sprintf (elpid,"%d", (int) getpid());
    if ((pid=fork())==-1){
        perror ("Imposible crear proceso");
        return;
    }
    if (pid==0){
        if (execvp(argv[0],argv)==-1)
            perror("cannot execute pmap");
        exit(1);
    }
    waitpid (pid,NULL,0);
}

int memoria(char *tokens[], int ntokens, struct Listas *lista){
    int a, b, c;
    static int d, e, f;
    if(tokens[1] == NULL || strcmp(tokens[1], "-all") == 0){
        printf("Variables locales: %19p, %17p, %17p\n", &a, &b, &c);
        printf("Variables globales: %18p, %17p, %17p\n", &ej1, &ej2, &ej3);
        printf("Variables estaticas: %17p, %17p, %17p\n", &d, &e, &f);
        printf("Funciones programa: %18p, %17p, %17p\n", memoria, malloc_free, malloc_fun);
        printf("Funciones programa: %18p, %17p, %17p\n", getpid, strcmp, strcpy);printf("******Lista de bloques asignados para el proceso %d\n", getpid());
        print_malloc(lista);
        print_mmap(lista);
        print_shared(lista);
        return 0;
    }else if(strcmp(tokens[1], "-blocks") == 0){
        printf("******Lista de bloques asignados para el proceso %d\n", getpid());
        print_malloc(lista);
        print_mmap(lista);
        print_shared(lista);
    }else if(strcmp(tokens[1], "-vars") == 0){
        printf("Variables locales: %19p, %17p, %17p\n", &a, &b, &c);
        printf("Variables globales: %18p, %17p, %17p\n", &ej1, &ej2, &ej3);
        printf("Variables estaticas: %17p, %17p, %17p\n", &d, &e, &f);
        return 0;
    }else if(strcmp(tokens[1], "-funcs") == 0){
        printf("Funciones programa: %18p, %17p, %17p\n", memoria, malloc_free, malloc_fun);
        printf("Funciones programa: %18p, %17p, %17p\n", getpid, strcmp, strcpy);
        return 0;
    }else if(strcmp(tokens[1], "-pmap") == 0){
        dopmap();
        return 0;
    }else{
        printf("Opcion %s no contemplada\n", tokens[1]);
    }

    return 0;
}

// V O L C A R M E M ------------------------------------------------------------------------------------------------------

int volcarmem(char *tokens[], int ntokens, struct Listas *lista){
    char *address;
    int size;
    if(tokens[1] == NULL){
        return 0;
    }
    address = (char*) strtoul(tokens[1], NULL, 16);

    if(tokens[2] == NULL){
        size = 25;
    }else{
        size = atoi(tokens[2]);
    }
    int a = size / 25;
    int b = size % 25;
    int despl = 0;
    int cont = 0;

    if(size < 25){
        despl = size;
    }else{
        despl = 25;
    }
    for(int i=0; i < size; i = i + 25) {
        for (int j = 0; j < despl; j++) {
            if (isprint(address[j + i])) {
                printf("%3c", (address[j + i]));
            } else {
                printf("   ");
            }
        }
        printf("\n");
        for (int j = 0; j < despl; j++) {
            if (isprint(address[j + i])) {
                printf("%3X", (address[j + i]));
            } else{
                printf(" 0");
            }
        }
        printf("\n");
        ++cont;
        if (cont >= a) despl = b;
    }
    printf("\n");
    return 0;
}


// L L E N A R M E M ------------------------------------------------------------------------------------------------------

int llenarmem(char *tokens[], int ntokens, struct Listas *lista){
    int cont=128,i;
    char* dir;
    char A = 'A';
    if (tokens[1] == NULL) {
        return 0;
    }
    dir = (void*)strtoul(tokens[1],NULL,16);
    if (tokens[2] != NULL) {
        cont = atoi(tokens[2]);
    }
    if (tokens[3] != NULL) {
        A = (char) strtoul(tokens[3],NULL,16);
    }
    for (i=0; i < cont; i++) {
        dir[i] = A;
    }
    return 0;
}

//  R E C U R S I V A -----------------------------------------------------------------------------------------------------

void doRecursiva(int n){
    char automatico[TAMANO];
    static char estatico[TAMANO];
    printf("parametro: %d(%p) array %p, arr estatico %p\n", n, &n, automatico, estatico);
    n--;
    if (n>=0)
        doRecursiva(n);
}

int recursiva(char *tokens[], int ntokens, struct Listas *lista){
    if(tokens[1]==NULL){
        printf("Error al recibir el parametro\n");
    }
    else {
        int n = atoi(tokens[1]);
        doRecursiva(n);
    }
    return 0;
}

// R E A D F I C H --------------------------------------------------------------------------------------------------------

ssize_t LeerFichero (char *fich, void *p, ssize_t n)
{ /* le n bytes del fichero fich en p */
    ssize_t nleidos,tam=n; /*si n==-1 lee el fichero completo*/
    int df, aux;
    struct stat s;
    if (stat (fich,&s)==-1 || (df=open(fich,O_RDONLY))==-1)
        return ((ssize_t)-1);
    if (n == LEERCOMPLETO)
        tam=(ssize_t) s.st_size;
    if ((nleidos=read(df,p, tam))==-1){
        aux=errno;
        close(df);
        errno=aux;
        return ((ssize_t)-1);
    }
    close (df);
    return (nleidos);
}


int readfich(char *tokens[], int ntokens, struct Listas *lista){
    ssize_t cont, x;
    char *dir;
    dir = (char*) strtoul(tokens[2],NULL,16);
    if (tokens[3] != NULL){
        cont = atoi(tokens[3]);
    }else{
        cont = -1;
    }
    x = LeerFichero(tokens[1], dir, cont);
    if (x ==-1) {
        perror("Error al abrir el archivo");
        return 0;
    } else {
        printf("read %ld bytes of %s in %s\n", x, tokens[1], tokens[2]);
    }

    return 0;
}

// W R I T E F I C H  ------------------------------------------------------------------------------------------------------

int writefich(char *tokens[], int ntokens, struct Listas *lista){
    int cont, flags, df;
    char *dir, *nombre;
    if(!strcmp(tokens[1],"-o") && tokens[2]!=NULL && tokens[3]!=NULL &&
       tokens[4]!=NULL){
        nombre = tokens[2];
        cont = atoi(tokens[4]);
        dir = (char*)strtoul(tokens[3],NULL, 16);
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    } else if (tokens[1]==NULL || tokens[2]==NULL || tokens[3]==NULL){
        printf("Faltan parametros\n");
        return 0;
    } else {
        nombre = tokens[1];
        cont = atoi(tokens[3]);
        dir = (char*)strtoul(tokens[2],NULL,16);
        flags = O_WRONLY | O_CREAT | O_EXCL;
    }
    if((df=open(nombre, flags, S_IRWXU | S_IRWXG | S_IRWXO))==-1){
        perror("Error al abrir el archivo");
        return 0;
    }
    write(df,dir,cont);
    close(df);
    return 0;
}