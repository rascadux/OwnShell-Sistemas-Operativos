
// AUTOR: Daniel Rodriguez Sanchez
// AUTOR: Lucia Docampo Rodriguez

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "list.h"
#include "struct_listas.h"
#include "p1.h"
#include "p2.h"
#include "p3.h"

#define ROJO   "\x1b[31m"
#define BRILLO "\x1b[1m"
#define RESET  "\x1b[0m"
#define MAX_TOKENS 100

int processCmd(char *tokens[], int ntokens, struct Listas *lista);

int parseString(char * cadena, char * trozos[]) {
    int i=1;
    if ((trozos[0]=strtok(cadena," \n\t"))==NULL)
        return 0;
    while ((trozos[i]=strtok(NULL," \n\t"))!=NULL)
        i++;

    return i;
}

int autores(char *tokens[], int ntokens, struct Listas *lista){
    if(tokens[1]==NULL){
        printf("Daniel Rodriguez Sanchez: daniel.rodriguez.sanchez1\n");
        printf("Lucia Docampo Rodriguez: lucia.docampo\n");
    }else{
        if (!strcmp(tokens[1],"-n")) {
            printf("Daniel Rodriguez Sanchez\n");
            printf("Lucia Docampo Rodriguez\n");
        }
        if(!strcmp(tokens[1],"-l")){
            printf("daniel.rodriguez.sanchez1\n");
            printf("lucia.docampo\n");
        }
    }
    return 0;
}

int pid(char *tokens[], int ntokens, struct Listas *lista){
    if (tokens[1]==NULL){
        printf("Pid de shell: %d\n", getpid());
    }else {
        if(!strcmp(tokens[1], "-p")){
            printf("Pid del padre del shell:  %d\n", getppid());
        }
    }
    return 0;
}

int carpeta(char *tokens[], int ntokens, struct Listas *lista){
    char direccion [100];
    if (tokens[1]==NULL){
        printf("%s\n", getcwd(direccion, 100));
    } else{
        if (chdir(tokens[1])!=0){
            printf("Imposible cambiar directorio: No such file or directory\n");
        }
    }
    return 0;
}

int fecha(char *tokens[], int ntokens, struct Listas *lista){
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char fecha[100];
    if ((tokens[1]==NULL)||!strcmp(tokens[1],"-h")) {
        strftime(fecha, 100, "%H:%M:%S",tm);
        printf("%s\n", fecha);
    }
    if ((tokens[1]==NULL)||!strcmp(tokens[1],"-d")){
        strftime(fecha, 100, "%d/%m/%Y",tm);
        printf("%s\n", fecha);
    }
    return 0;
}

int hist(char *tokens[], int ntokens, struct Listas *lista){

    if(tokens[1]==NULL){
        int i=0;

        for(pos p = first(lista->command_list); !end(lista->command_list, p); p = next(lista->command_list, p)) {
            char *command = get(lista->command_list, p);
            printf("%d-> %s\n", i, command);
            i++;
        }
    }else if(!strcmp(tokens[1], "-c")){
        clear(&lista->command_list); //Borra la lista
    }else if(atoi(tokens[1]) != 0){ // en caso de hist -N
        int n = -atoi(tokens[1]);
        pos p = first(lista->command_list);
        for(int i=0 ; i<=n; i++){
            char *command = get(lista->command_list, p);
            printf("%d-> %s\n", i, command);
            p = next(lista->command_list, p);
        }
    }
    return 0;
}

int comando(char *tokens[], int ntokens, struct Listas *lista){
    char *comp;
    strtol(tokens[1], &comp, 10);
    if((comp == tokens[1]) || (tokens[1]==NULL)){
        printf("Error, despues de 'comando' introduce un numero incluido en 'hist'\n");
        return 0;
    } else{
        int n = atoi(tokens[1]);
        pos p = first(lista->command_list);
        int i=0;
        while(!end(lista->command_list, p)){ // sumamos i++ hasta que llega al final de la lista
            p=next(lista->command_list, p);
            i++; //cuando salga del bucle i=cantidad de comandos en la lista
        }
        if(n>i){
            printf("No hay elemento %d en el historico\n", n);
        }else{

            int n = atoi(tokens[1]);
            pos p = first(lista->command_list); //creamos p que apunta al primer nodo de la lista
            for(int i=0; i<n; i++){
                p = next(lista->command_list, p); //avanzamos hasta la posicion donde esta el comando
            }
            char* command = get(lista->command_list, p); // command es el comando que queremos repetir
            printf("Ejecutando hist (%d): %s\n", n, command);
            char* command2 = strdup(command);
            int numtokens = parseString(command2, tokens);
            processCmd(tokens, numtokens, lista);
            free(command2);
        }
    }
    return 0;
}

int infosis(char *tokens[], int ntokens, struct Listas *lista){
    struct utsname utsData;
    uname(&utsData);
    printf("%s %s %s %s %s\n", utsData.sysname, utsData.nodename, utsData.release, utsData.version, utsData.machine);
    return 0;
}

int ayuda (char *tokens[], int ntokens, struct Listas *lista){
    //no se le puede pasar un string a un switch asi que hay que encadenar if elses
    if(tokens[1]==NULL){
        printf("'ayuda cmd' donde cmd es uno de los siguientes comandos:\n");
        printf("autores "); printf("pid "); printf("carpeta "); printf("fecha ");
        printf("hist "); printf("comando "); printf("infosis "); printf("ayuda ");
        printf("fin "); printf("salir "); printf("bye\n");
    }else if(!strcmp(tokens[1], "autores")){
        printf("autores [-n|-l] Muestra los nombres y logins de los autores\n");
    }else if(!strcmp(tokens[1], "pid")){
        printf("pid [-p]	Muestra el pid del shell o de su proceso padre\n");
    } else if(!strcmp(tokens[1], "carpeta")){
        printf("carpeta [dir]	Cambia (o muestra) el directorio actual del shell\n");
    } else if(!strcmp(tokens[1], "fecha")){
        printf("fecha [-d|.h	Muestra la fecha y o la hora actual\n");
    } else if(!strcmp(tokens[1], "hist")){
        printf("hist [-c|-N]	Muestra el historico de comandos, con -c lo borra\n");
    } else if(!strcmp(tokens[1], "comando")){
        printf("comando [-N]	Repite el comando N (del historico)\n");
    } else if(!strcmp(tokens[1], "infosis")){
        printf("infosis 	Muestra informacion de la maquina donde corre el shell\n");
    } else if(!strcmp(tokens[1], "ayuda")){
        printf("ayuda [cmd]	     Muestra ayuda sobre los comandos\n");
    } else if(!strcmp(tokens[1], "fin")){
        printf("fin    Termina la ejecucion del shell\n");
    } else if(!strcmp(tokens[1], "salir")){
        printf("salir  Termina la ejecucion del shell\n");
    } else if(!strcmp(tokens[1], "bye")){
        printf("bye    Termina la ejecucion del shell\n");
    } else printf("%s no encontrado\n", tokens[1]);

    return 0;
}

int fin(char *tokens[], int ntokens, struct Listas *lista) {
    return 1;
}

struct cmd {
    char *cmd_name;
    int (*cmd_fun)(char *tokens[], int ntokens, struct Listas *lista);
};

struct cmd cmds[] = {
        //Practica 0
        {"autores", autores},
        {"pid", pid},
        {"carpeta", carpeta},
        {"hist", hist},
        {"comando", comando},
        {"infosis", infosis},
        {"ayuda", ayuda},
        {"fecha", fecha},
        {"fin", fin},
        {"salir", fin},
        {"bye", fin},
        //Practica 1
        {"crear", crear},
        {"borrar", borrar},
        {"borrarrec", borrarrec},
        {"listfich", listfich},
        {"listdir", listdir},
        //Practica 2
        {"malloc", malloc_fun},
        {"mmap", mmap_fun},
        {"shared", shared_fun},
        {"dealloc", dealloc_fun},
        {"memoria", memoria},
        {"volcarmem", volcarmem},
        {"llenarmem", llenarmem},
        {"recursiva", recursiva},
        {"e-s read", readfich},
        {"e-s write", writefich},
        //Practica 3
        {"priority", priority},
        {"rederr", rederr},
        {"entorno", entorno},
        {"mostrarvar", mostrarvar},
        {"cambiarvar", cambiarvar},
        {"uid", uid},
        {"fork", fork_fun},
        {"ejec", ejec},
        {"ejecpri", ejecpri},
        {"fg", fg},
        {"fgpri", fgpri},
        {"back", back},
        {"backpri", backpri},
        {"ejecas", ejecas},
        {"fgas", fgas},
        {"bgas", bgas},
        {"listjobs", listjobs},
        {"job", job},
        {"borrarjobs", borrarjobs},
        {NULL, NULL}
};

int processCmd(char *tokens[], int ntokens, struct Listas *lista) {
    int i;
    for(i=0; cmds[i].cmd_name != NULL; i++) {
        if(tokens[0] == NULL){//si no se mete nada
            return 0;
        }
        if(strcmp(tokens[0], cmds[i].cmd_name) == 0)//si son iguales
            return cmds[i].cmd_fun(tokens, ntokens, lista);
    }

    return cualquier_comando(tokens, ntokens, lista);
}

int main(int argc, char *argv[], char *env[]) {
    char *line;
    char *tokens[MAX_TOKENS];
    int ntokens;
    int end = 0;

    struct Listas *lista = malloc(sizeof(struct Listas));
    //Inicializo lista para comandos
    init_list(&lista->command_list);
    //Inicializo lista para malloc
    init_list(&lista->malloc_list);
    //Inicializo lista para mmap
    init_list(&lista->mmap_list);
    //Inicializo lista para Shared
    init_list(&lista->shared_list);
    //Inicializo lista para Background Process
    init_list(&lista->background_list);
    //Array env[] para la funcion entorno
    lista->otros.env_main = &env;

    while(!end) {
        line = readline(ROJO BRILLO ">> " RESET);
        char* linecpy = strdup(line);
        insert(&lista->command_list, linecpy);
        ntokens = parseString(line, tokens);
        end = processCmd(tokens, ntokens, lista);
        free(line);
    }
}
