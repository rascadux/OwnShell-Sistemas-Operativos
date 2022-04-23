// AUTOR: Daniel Rodriguez Sanchez
// AUTOR: Lucia Docampo Rodriguez

//Las funciones walk_dir, delete_entry y cmd_delete fueron dadas por el profesor
//y modificadas luego por nosotros, de ahi que alomejor se puedan parecer a las
//de otros compañeros del grupo

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <libgen.h>
#include "struct_listas.h"

#define MAX_PATH 1000

struct delete_options{ //rec se vuelve 1 cuando hay que hacerlo recursivamente
    int rec;
};

struct listing_options{ //struct para comprobar que comandos se estan pidiendo ejecutar
    int longo;
    int acc;
    int link;
    int hid;
    int reca;
    int recb;
};

int crear(char *tokens[], int ntokens, struct Listas *lista){
    if(tokens[1] == NULL){
        char direccion[100];
        printf("%s\n", getcwd(direccion, 100));
        return 0; //para que no siga
    }
    int i, fd;
    char *name;
    for(i=1; i<ntokens; i++){ //va leyendo los nombres de los ficheros
        if(!strcmp(tokens[1], "-f")){
            if(i==1) i=2;//en caso de que se introducjera el -f los nombres empiezan en el tokens[2]
            name = tokens[i];
            fd = open(name, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR); //mira si existe, si no existe,
            if (fd < 0) { //En caso de que ya haya un fichero con ese nombre o otro error
                printf("Imposible a crear: %s\n", strerror(errno));
            }
        }else { //en caso de que se pida un directorio no un fichero
            name = tokens[i];
            if(mkdir(name, S_IRWXU) != 0){
                printf("Imposible crear: %s\n", strerror(errno));
            }
        }
    }
    return 0;
}

void walk_dir(char *entry, void (*action)(char *entry, void *), void *options, int hid, int hid2) {
    struct dirent *d;
    DIR *dir;
    if((dir = opendir(entry)) == NULL) {
        printf("Could not open %s: %s\n", entry, strerror(errno));
        return;
    }
    while((d = readdir(dir)) != NULL) {

        char *name = strdup(entry);
        char *base = basename(name);
        if(hid==0 && d->d_name[0] == '.'){
            continue;
        }
        if(hid2==1 && strcmp(d->d_name, ".")==0 || strcmp(d->d_name, "..")==0){
            continue;
        }
        char new_entry[MAX_PATH];
        strcpy(new_entry, entry);
        strcat(new_entry, "/");
        strcat(new_entry, d->d_name);
        action(new_entry, options);
    }
    closedir(dir);
}

void delete_entry(char *entry, struct delete_options *opts) { //Borra recursivamente el contenido
    struct stat st;

    if(lstat(entry, &st) == -1) {
        printf("Could not delete %s: %s\n", entry, strerror(errno));
        return;
    }
    if(S_ISDIR(st.st_mode)) {
        char *name = strdup(entry);
        char *base = basename(name);
        if(opts->rec == 1 && (strcmp(base, ".")==0 || strcmp(base, "..")==0)) {
            walk_dir(entry, (void (*)(char *, void *)) delete_entry, opts, 0, 1);
        }
        free(name);

        if(rmdir(entry) == -1) {
            printf("Could not delete %s: %s\n", entry, strerror(errno));
            return;
        }
    } else {
        if(unlink(entry) == -1) {
            printf("Could not delete %s: %s\n", entry, strerror(errno));
            return;
        }
    }
}

void cmd_delete(char *tokens[], int ntokens, struct delete_options opts) { //Para listdir hacer esto pero con la funcion listar

    for(int i=0; tokens[i]!=NULL; i++){
        delete_entry(tokens[i], &opts);
    }
}

int borrar(char *tokens[], int ntokens, struct Listas *lista){
    char direccion [100];
    if (tokens[1]==NULL){
        printf("%s\n", getcwd(direccion, 100));
        return 0; //para que no siga
    }
    struct delete_options opts;
    opts.rec=0;
    cmd_delete(tokens+1, ntokens-1, opts);
}

int borrarrec(char *tokens[], int ntokens, struct Listas *lista){
    char direccion [100];
    if (tokens[1]==NULL){
        printf("%s\n", getcwd(direccion, 100));
        return 0; //para que no siga
    }
    struct delete_options opts;
    opts.rec=1;
    cmd_delete(tokens+1, ntokens-1, opts);
    return 0;
}

void listing_long(char *tokens, struct listing_options *opts){ //printea informacion -long, -link, -acc
    struct stat info;
    if(lstat(tokens, &info) == -1){
        printf("%s", strerror(errno));
        return;
    }
    struct passwd *pw = getpwuid(info.st_uid);
    struct group *gr = getgrgid(info.st_gid);
    char time[100];
    if(opts->acc == 1){ // si se pide el -acc
        strftime(time, 50, "%Y/%m/%d-%H:%M", localtime(&(info.st_atime))); // time = Ultimo acceso
    }else{
        strftime(time, 50, "%Y/%m/%d-%H:%M", localtime(&(info.st_mtime))); // time = Ultima modificacion
    }
    printf("%s   %ld (%lu)  %s  %s ", time, info.st_nlink, info.st_ino, pw->pw_name, gr->gr_name);
    printf( (S_ISDIR(info.st_mode)) ? "d" : "-");
    printf( (info.st_mode & S_IRUSR) ? "r" : "-");
    printf( (info.st_mode & S_IWUSR) ? "w" : "-");
    printf( (info.st_mode & S_IXUSR) ? "x" : "-");
    printf( (info.st_mode & S_IRGRP) ? "r" : "-");
    printf( (info.st_mode & S_IWGRP) ? "w" : "-");
    printf( (info.st_mode & S_IXGRP) ? "x" : "-");
    printf( (info.st_mode & S_IROTH) ? "r" : "-");
    printf( (info.st_mode & S_IWOTH) ? "w" : "-");
    printf( (info.st_mode & S_IXOTH) ? "x" : "-");
    if(opts->link == 0){ //Si no se pide con -link
        printf("%10ld %s\n", info.st_size, basename(tokens));
    } else{ // Si se pide con -link
        if(S_ISLNK(info.st_mode)){
            char buffer[MAX_PATH];
            ssize_t link_length = readlink(tokens, buffer, MAX_PATH);
            buffer[link_length] = '\0'; //verificar que este inicializado a 0
            printf("%9ld %s -> %s\n", info.st_size, basename(tokens), buffer);
        }else {
            printf("%9ld %s\n", info.st_size, basename(tokens));
        }

    }
}

void list_one(char *tokens, struct listing_options *opts){
    char *name = strdup(tokens);
    char *base = basename(name);
    if(opts->hid == 1){
        if(opts->longo == 1){
            listing_long(tokens, opts); //funcion que imprime la informacion
        } else{
            struct stat info;
            if(lstat(tokens, &info) == -1) {
                printf("%s\n", strerror(errno));
            }else{
                printf("%8ld  %s\n", info.st_size, base);
            }
        }
    } else{ //Si no metes el comando -hid
        if (base[0]!='.'){
            if(opts->longo == 1){
                listing_long(tokens, opts); //funcion que imprime la informacion
            } else{
                struct stat info;
                if(lstat(tokens, &info) == -1) {
                    printf("%s\n", strerror(errno));
                }else{
                    printf("%8ld  %s\n", info.st_size, base);
                }
            }
        }
    }
}

int check_command_fich(char *tokens[], int ntokens, struct listing_options *opts){
    int i=0, nombre=0; //posicion del string en la que estamos
    if((tokens[1]) == NULL){
        char direccion[100];
        printf("%s\n", getcwd(direccion, 100));
        return 0;
    }
    while(nombre != 1){//mientras que no llega al nombre comprueba que comandos se introducen
        i++; //empieza en tokens[1] y cuando no encuentra mas comandos, tokens[i] es el primer fichero
        if(tokens[i] == NULL){ //Si no se introduce ningun nombre, devuelve directorio actual
            char direccion[100];
            printf("%s\n", getcwd(direccion, 100));
            return 0;
        }
        if(!strcmp(tokens[i], "-long")){
            opts->longo=1;
        }else if(!strcmp(tokens[i], "-link")){
            opts->link=1;
        }else if(!strcmp(tokens[i], "-acc")){
            opts->acc=1;
        }else nombre=1;//cuando ya no encuentra mas comandos, esta en el nombre
    }

    while(i<ntokens){//mientras que no llegue al final de los nombres
        list_one(tokens[i], opts);
        i++;
    }
    return 0;
}

int listfich(char *tokens[], int ntokens, struct Listas *lista){
    struct listing_options opts; //struct para comprobar que comandos se estan pidiendo ejecutar
    check_command_fich(tokens, ntokens, &opts);
    return 0;
}

void list_entry(char *entry, struct listing_options *opts) { //Lista recursivamente el contenido
    struct stat info;

    if(lstat(entry, &info) == -1) {
        printf("Could not list %s: %s\n", entry, strerror(errno));
        return;
    }

    if(S_ISREG(info.st_mode) && opts->reca==0 && opts->recb==0){
        list_one(entry, opts);

    }else if(S_ISDIR(info.st_mode)) { //Si es un directorio
        char *name = strdup(entry);
        char *base = basename(name);
        printf("************%s\n", entry);
        if(opts->recb == 1){ // Subdirectorios antes
            walk_dir(entry, (void (*)(char *, void *)) list_entry, opts, opts->hid, 1);
            walk_dir(entry, (void (*)(char *, void *)) list_one, opts, opts->hid, 0);

        }else if(opts->reca == 1){ // Subdirectorios despues
            walk_dir(entry, (void (*)(char *, void *)) list_one, opts, opts->hid, 0);
            walk_dir(entry, (void (*)(char *, void *)) list_entry, opts, opts->hid, 1);//tiene que hacerlo recursivo
        } else{
            walk_dir(entry, (void (*)(char *, void *)) list_one, opts, opts->hid, 0);
        }
        free(name);
    }
}

int check_command_dir(char *tokens[], int ntokens, struct listing_options *opts){
    int i=0, nombre=0; //posicion del string en la que estamos
    if((tokens[1]) == NULL){ //si solo se introduce el comando
        char direccion[100];
        printf("%s\n", getcwd(direccion, 100));
        return 0;
    }
    while(nombre != 1){//mientras que no llega al nombre comprueba que comandos se introducen
        i++; //empieza en tokens[1] y cuando no encuentra mas comandos, tokens[i] es el primer fichero
        if(tokens[i] == NULL){ //Si no se introduce ningun nombre, devuelve directorio actual
            char direccion[100];
            printf("%s\n", getcwd(direccion, 100));
            return 0;
        }
        if(!strcmp(tokens[i], "-long")){
            opts->longo=1;
        }else if(!strcmp(tokens[i], "-link")){
            opts->link=1;
        }else if(!strcmp(tokens[i], "-acc")){
            opts->acc=1;
        }else if(!strcmp(tokens[i], "-hid")){
            opts->hid=1;
        }else if(!strcmp(tokens[i], "-reca")){
            opts->reca=1;
        }else if(!strcmp(tokens[i], "-recb")){
            opts->recb=1;
        }else nombre=1;//cuando ya no encuentra mas comandos, esta en el nombre
    }
    return i;
}

void cmd_list(char *tokens[], int ntokens, struct listing_options *opts) {

    int i = check_command_dir(tokens, ntokens, opts); // i es la posicion de tokens[] en la que empiezan los nombres
    if(i==0){// Si no se introdujo ningun comando o ningun nombre
        return;
    }
    while(i < ntokens){ //recorre todos los nombre
        list_entry(tokens[i], opts);
        i++;
    }
}

int listdir(char *tokens[], int ntokens, struct Listas *lista){
    struct listing_options opts;
    opts.longo=0;
    opts.acc=0;
    opts.link=0;
    opts.hid=0;
    opts.reca=0;
    opts.recb=0;
    cmd_list(tokens, ntokens, &opts);
    return 0;
}
