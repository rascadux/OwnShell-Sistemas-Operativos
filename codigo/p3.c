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
#include <sys/resource.h>
#include <pwd.h>
#include <sys/types.h>
#include "list.h"
#include "struct_listas.h"
#include "p2.h"
#include "p3.h"


#define MAXVAR 1024
#define MAX_TOKENS 100

struct background_info{
    pid_t pid;
    char user[100];
    char *command[100];
    char date[101];
    int ret_value;
    char status[100];
    int prio;
    int valor;
    int leido;
};

/******************************SENALES ******************************************/
struct SEN{
    char *nombre;
    int senal;
};
static struct SEN sigstrnum[]={
        "HUP", SIGHUP,
        "INT", SIGINT,
        "QUIT", SIGQUIT,
        "ILL", SIGILL,
        "TRAP", SIGTRAP,
        "ABRT", SIGABRT,
        "IOT", SIGIOT,
        "BUS", SIGBUS,
        "FPE", SIGFPE,
        "KILL", SIGKILL,
        "USR1", SIGUSR1,
        "SEGV", SIGSEGV,
        "USR2", SIGUSR2,
        "PIPE", SIGPIPE,
        "ALRM", SIGALRM,
        "TERM", SIGTERM,
        "CHLD", SIGCHLD,
        "CONT", SIGCONT,
        "STOP", SIGSTOP,
        "TSTP", SIGTSTP,
        "TTIN", SIGTTIN,
        "TTOU", SIGTTOU,
        "URG", SIGURG,
        "XCPU", SIGXCPU,
        "XFSZ", SIGXFSZ,
        "VTALRM", SIGVTALRM,
        "PROF", SIGPROF,
        "WINCH", SIGWINCH,
        "IO", SIGIO,
        "SYS", SIGSYS,
/*senales que no hay en todas partes*/
#ifdef SIGPOLL
        "POLL", SIGPOLL,
#endif
#ifdef SIGPWR
        "PWR", SIGPWR,
#endif
#ifdef SIGEMT
        "EMT", SIGEMT,
#endif
#ifdef SIGINFO
        "INFO", SIGINFO,
#endif
#ifdef SIGSTKFLT
        "STKFLT", SIGSTKFLT,
#endif
#ifdef SIGCLD
        "CLD", SIGCLD,
#endif
#ifdef SIGLOST
        "LOST", SIGLOST,
#endif
#ifdef SIGCANCEL
        "CANCEL", SIGCANCEL,
#endif
#ifdef SIGTHAW
        "THAW", SIGTHAW,
#endif
#ifdef SIGFREEZE
        "FREEZE", SIGFREEZE,
#endif
#ifdef SIGLWP
        "LWP", SIGLWP,
#endif
#ifdef SIGWAITING
        "WAITING", SIGWAITING,
#endif
        NULL,-1,
}; /*fin array sigstrnum */

int priority(char *tokens[], int ntokens, struct Listas *lista){
    int p;
    if(tokens[1] == NULL){
        //Mostrar prioridad del proceso que ejecuta el shell
        if((p = getpriority(PRIO_PROCESS, 0)) != -1){
            printf("Prioridad del proceso %d es %d\n", getpid(), p);
        }else perror("Error al obtener prioridad: ");
    }else if(tokens[2] == NULL){
        //Mostrar prioridad del proceso tokens[1]
        id_t numpid = (id_t) atoi(tokens[1]);
        if((p = getpriority(PRIO_PROCESS, numpid)) != -1){
            printf("Prioridad del proceso %d es %d\n", numpid, p);
        }else perror("Error al obtener prioridad: ");
    }else{
        //La prioridad de tokens[1] pasa a ser tokens[2]
        id_t numpid = (id_t) atoi(tokens[1]);
        int valor = atoi(tokens[2]);
        if((p = setpriority(PRIO_PROCESS, numpid, valor)) != -1) {
            printf("Prioridad del proceso %d cambiada a %d\n", numpid, valor);
        }else perror("Error al cambiar prioridad: ");
    }
    return 0;
}

int rederr(char *tokens[], int ntokens, struct Listas *actual){
    
    if(tokens[1] == NULL){
        //Shows where the standard error is currently going to
        //char buff[1024];
        //ssize_t a = readlink("/proc/self/fd/2", buff, sizeof(buff));
        //printf("Error estandar en fichero %s\n", buff); //Imprimir nombre del fichero
        if(actual->otros.name[0] == 0){
            printf("error estandar en fichero configuracion original\n");
        }else printf("error estandar en fichero %s\n", actual->otros.name);
        return 0;
    }

    if(strcmp(tokens[1], "-reset") == 0){
        //Restores the standard error to what it was originally
        dup2(actual->otros.copia_stderr, STDERR_FILENO);
        memset(&actual->otros.name[0], 0, sizeof(actual->otros.name));
        return 0;
    }

    //Redirects the standard error of the shell to file fich

    if ((actual->otros.copia_stderr = dup(STDERR_FILENO)) == -1){
        perror("Error: ");
        return  0;
    }

    if(close(STDERR_FILENO) == -1){
        perror("Error: ");
        return 0;
    }

    if(fopen(tokens[1], "ab+") == NULL){
        perror("Error: ");
        return 0;
    }
    strcpy(actual->otros.name, tokens[1]);
    return 0;
}

void MostrarEntorno (char **entorno, char * nombre_entorno)
{
    int i=0;
    while (entorno[i]!=NULL) {
        printf ("%p->%s[%d]=(%p) %s\n", &entorno[i],
                nombre_entorno, i, entorno[i], entorno[i]);
        i++;
    }
}

int entorno(char *tokens[], int ntokens, struct Listas *variables) {
    printf("environ->%p\n", environ);
    printf("env->%p\n", (*variables->otros.env_main));
    printf("\n --------------------------------------- \n");
    if (tokens[1] == NULL) {
        MostrarEntorno((*variables->otros.env_main), "main");
        return 0;
    }

    if(strcmp(tokens[1], "-environ") == 0){
        //Shows all the environment variables of the shell process.
        //Access will be through the external variable environ
        MostrarEntorno(environ, "environ");
        /*int i=0;
        while(environ[i] != NULL){
            printf("%p->environ[%d]=(%p) %s\n", environ[i], i, &environ[i], environ[i]);
            i++;
        }*/
        return 0;
    }

    if(strcmp(tokens[1], "-addr") == 0){
        //Shows the value (as pointers) of environ and the third argument of main
        //Shows also the addresses at with they are stored
        printf("environ: %p (almacenado en %p)\n", environ[0], &environ);
        printf("main arg3: %p (almacenado en %p)\n", &(*variables->otros.env_main)[0], &(*variables->otros.env_main));
        return 0;
    }
    printf("%s no es una opcion valida\n", tokens[1]);
    return 0;
}

int BuscarVariable (char * var, char *e[])
{
    int pos=0;
    char aux[MAXVAR];
    strcpy (aux,var);
    strcat (aux,"=");
    while (e[pos]!=NULL)
        if (!strncmp(e[pos],aux,strlen(aux)))
            return (pos);
        else
            pos++;
    errno = ENOENT; /*no hay tal variable*/
    return(-1);
}


int mostrarvar(char *tokens[], int ntokens, struct Listas *variables){

    if(tokens[1] == NULL){
        MostrarEntorno((*variables->otros.env_main), "main");
        return 0;
    }

    char *env_num;
    int main, envi;
    main = BuscarVariable(tokens[1], (*variables->otros.env_main));
    envi = BuscarVariable(tokens[1], environ);
    env_num = getenv(tokens[1]);
    if(main == -1){ //si no esta en la del main
        if((envi  != -1) && (env_num  != NULL )){   //pero esta en environ
            printf("  Con environ %s=:%s(%p) @%p\n", tokens[1], env_num, environ[envi], &environ[envi]);
            printf("    Con getenv %s(%p)\n", env_num, &env_num);
        }else perror("Error"); //Si tampoco esta en los otros 2
        return 0;
    }else{//Si esta en el main, esta en los otros 2
        printf("Con arg3 main %s=:%s(%p) @%p\n", tokens[1], env_num, (*variables->otros.env_main)[main], &(*variables->otros.env_main)[main]);
        printf("  Con environ %s=:%s(%p) @%p\n", tokens[1], env_num, environ[envi], &environ[envi]);
        printf("    Con getenv %s(%p)\n", env_num, &env_num);
    }
    return 0;
}

int CambiarVariable(char * var, char * valor, char *e[])
{
    int pos;
    char *aux;
    if ((pos=BuscarVariable(var,e))==-1)
        return -1;
    if ((aux=(char *)malloc(strlen(var)+strlen(valor)+2))==NULL)
        return -1;
    strcpy(aux,var);
    strcat(aux,"=");
    strcat(aux,valor);
    e[pos]=aux;
    return (pos);
}

int cambiarvar(char *tokens[], int ntokens, struct Listas *entorno){

    if(tokens[1] == NULL || tokens[2] == NULL || tokens[3] == NULL){
        printf("Uso: cambiarvar [-a|-e|-p] var valor\n");
        return 0;
    }

    if(strcmp(tokens[1], "-a") == 0){
        //access through main’s third argument
        //tokens[2] = VAR || tokens[3] = value
        if(CambiarVariable(tokens[2], tokens[3], (*entorno->otros.env_main)) == -1){
            printf("Uso: cambiarvar [-a|-e|-p] var valor\n");
        }
        return 0;
    }

    if(strcmp(tokens[1], "-e") == 0){
        //through environ
        //tokens[2] = VAR || tokens[3] = value
        if(CambiarVariable(tokens[2], tokens[3], environ) == -1){
            printf("Uso: cambiarvar [-a|-e|-p] var valor\n");
        }
        return 0;
    }

    if(strcmp(tokens[1], "-p") == 0){
        //means access through the library function putenv
        //tokens[2] = VAR || tokens[3] = value
        char *cadena = NULL;
        if ((cadena = (char *)malloc(strlen(tokens[2])+strlen(tokens[3])+2)) == NULL){
            perror("Error malloc");
            return 0;
        }
        strcpy(cadena, tokens[2]);
        strcat(cadena, "=");
        strcat(cadena, tokens[3]);
        putenv(cadena);
        return 0;
    }

    printf("Uso: cambiarvar [-a|-e|-p] var valor\n");
    return 0;
}

//U I D --------------------------------------

char * NombreUsuario (uid_t uid)
{
    struct passwd *p;
    if ((p= getpwuid(uid))==NULL)
        return (" ??????");
    return p->pw_name;
}
uid_t UidUsuario (char * nombre)
{
    struct passwd *p;
    if ((p= getpwnam (nombre))==NULL)
        return (uid_t) -1;
    return p->pw_uid;
}
void MostrarUidsProceso (void)
{
    uid_t real=getuid(), efec=geteuid();
    printf ("Credencial real: %d, (%s)\n", real, NombreUsuario (real));
    printf ("Credencial efectiva: %d, (%s)\n", efec, NombreUsuario (efec));
}
void CambiarUidLogin (char * login)
{
    uid_t uid;
    if ((uid=UidUsuario(login))==(uid_t) -1){
        printf("login no valido: %s\n", login);
        return;
    }
    if (setuid(uid)==.1)
        printf ("Imposible cambiar credencial: %s\n", strerror(errno));
}

void Cmd_getuid(char *tr[]) {
    uid_t real = getuid(), efec = geteuid();

    printf("Credencial real: %d, (%s)\n", real, NombreUsuario(real));
    printf("Credencial efectiva: %d, (%s)\n", efec, NombreUsuario(efec));
}


void Cmd_setuid(char *tr[]){
    uid_t uid;
    int u;

    if(tr[0]==NULL || (!strcmp(tr[0],"-l") && tr[1]==NULL)){
        Cmd_getuid(tr);
        return;
    }
    if(!strcmp(tr[0],"-l")){
        if((uid = UidUsuario(tr[1])) == (uid_t) -1){
            printf("Usuario no existente %s\n",tr[1]);
            return;
        }
    }
    else if((uid=(uid_t) ((u=atoi(tr[0]))<0)? -1: u)==(uid_t) -1){
        printf("Valor no valido de la credencial %s\n",tr[0]);
        return;
    }
    if(setuid(uid)==-1){
        printf("Imposible cambiar credencial: %s\n", strerror(errno));
    }
}

int uid(char *tokens[], int ntokens, struct Listas *entorno){

    if(tokens[1] == NULL || strcmp(tokens[1],"-get") == 0){
        Cmd_getuid(NULL);
        return 0;
    }
    if(strcmp(tokens[1],"-set") == 0){
        if(tokens[2] == NULL){
            Cmd_getuid(NULL);
            return 0;
        }
        char *aux1[100];

        aux1[0]=tokens[2]; // [-l]
        aux1[1]=tokens[3]; // id

        Cmd_setuid(aux1);
    }
    return 0;
}

int fork_fun(char *tokens[], int ntokens, struct Listas *lista){
    pid_t pid;

    if((pid = fork()) == -1){
        perror("Error fork");
    }else{
        waitpid(pid, NULL, 0);
        printf("Ejecutando proceso %d\n", getpid());
    }
    return 0;
}

int ejec(char *tokens[], int ntokens, struct Listas *lista){
    char *aux1[100];
    int a=0;

    if(tokens[1] == NULL){
        printf("Faltan argumentos\n");
        return 0;
    }
    while(tokens[a] != NULL){
        aux1[a] = tokens[a+1];
        a++;
    }
    execvp(tokens[1], aux1);
    perror("No ejecutado");
    return 0;
}

int ejecpri(char *tokens[], int ntokens, struct Listas *lista) {

    // ejecpri prio prog arg1 arg2 ...
    char *aux1[100];
    int a=0;
    int prio;
    pid_t pid;

    if(tokens[1] == NULL || tokens[2] == NULL){
        printf("Faltan argumentos\n");
        return 0;
    }
    while(tokens[a+1] != NULL){
        aux1[a] = tokens[a+2];
        a++;
    }

    prio = atoi(tokens[1]);
    pid = getpid();
    if(setpriority(PRIO_PROCESS, pid, prio) == -1){
        perror("Error prioridad");
        return 0;
    }

    execvp(tokens[2], aux1);
    perror("No ejecutado");
    return 0;
}

int fg(char *tokens[], int ntokens, struct Listas *lista){
    char *aux1[100];
    pid_t pid;
    int a=0;

    if(tokens[1] == NULL){
        printf("Faltan argumentos\n");
        return 0;
    }
    while(tokens[a] != NULL){
        aux1[a] = tokens[a+1];
        a++;
    }
    if((pid = fork()) == 0){
        execvp(tokens[1], aux1);
        perror("No ejecutado");
        return 0;
    }
    waitpid(pid, NULL, 0);

    return 0;
}

int fgpri(char *tokens[], int ntokens, struct Listas *lista) {
    char *aux1[100];
    pid_t pid;
    int a=0;
    int prio;

    if(tokens[1] == NULL || tokens[2] == NULL){
        printf("Faltan argumentos\n");
        return 0;
    }
    while(tokens[a+1] != NULL){
        aux1[a] = tokens[a+2];
        a++;
    }

    prio = atoi(tokens[1]);

    if((pid = fork()) == 0){
        pid = getpid();
        if(setpriority(PRIO_PROCESS, pid, prio) == -1){
            perror("Error prioridad");
            return 0;
        }
        execvp(tokens[2], aux1);
        perror("No ejecutado");
        return 0;
    }else{
        waitpid(pid, NULL, 0);
    }
    return 0;
}

int back(char *tokens[], int ntokens, struct Listas *lista){
    char *aux1[100];
    int a=0;
    pid_t pid;

    if(tokens[1] == NULL){
        printf("Faltan argumentos\n");
        return 0;
    }
    while(tokens[a+1] != NULL){
        aux1[a] = strdup(tokens[a+1]);
        a++;
    }
    aux1[a] = NULL;
    if((pid = fork()) == 0){
        execvp(tokens[1], aux1);
        perror("No ejecutado");
        return 0;
    }
    struct background_info *nodo = malloc(sizeof(struct background_info));
    //--------------------------------------------------------------------------
    // COMMAND
    int x=0;
    while(aux1[x] != NULL){
        nodo->command[x] = aux1[x];
        x++;
    } //Guardo en nodo->command el comando ejecutado
    nodo->command[x] = NULL;// para que al recorrerlo en la funcion print, pare al llegar a NULL
    // DATE
    char time[100];
    tiempo_actual(time);
    strcpy(nodo->date, time);
    // PID
    nodo->pid = pid;
    // USER
    uid_t uid = getuid();
    strcpy(nodo->user, NombreUsuario(uid));
    // STATUS
    nodo->valor = 0;
    //--------------------------------------------------------------------------
    insert(&lista->background_list, nodo); //Guardo en la lista background
    return 0;
}

int backpri(char *tokens[], int ntokens, struct Listas *lista){
    char *aux1[100];
    int a=0;
    pid_t pid;
    int prio;

    if(tokens[1] == NULL || tokens[2] == NULL){
        printf("Faltan argumentos\n");
        return 0;
    }
    while(tokens[a+2] != NULL){
        aux1[a] = strdup(tokens[a+2]);
        a++;
    }
    aux1[a] = NULL;
    pid = getpid();
    prio = atoi(tokens[1]);
    if(setpriority(PRIO_PROCESS, pid, prio) == -1){
        perror("Error prioridad");
        return 0;
    }
    if((pid = fork()) == 0){
        execvp(tokens[2], aux1);
        perror("No ejecutado");
        return 0;
    }
    struct background_info *nodo = malloc(sizeof(struct background_info));
    //--------------------------------------------------------------------------
    int x=0;
    while(aux1[x] != NULL){
        nodo->command[x] = aux1[x];
        x++;
    } //Guardo en nodo->command el comando ejecutado
    nodo->command[x] = NULL;// para que al recorrerlo en la funcion print, pare al llegar a NULL
    // DATE
    char time[100];
    tiempo_actual(time);
    strcpy(nodo->date, time);
    // PID
    nodo->pid = pid;
    // USER
    uid_t uid = getuid();
    strcpy(nodo->user, NombreUsuario(uid));
    // STATUS
    nodo->valor = 0;
    //--------------------------------------------------------------------------
    insert(&lista->background_list, nodo); //Guardo en la lista background
    return 0;
}

int ejecas(char *tokens[], int ntokens, struct Listas *lista){
    //tokens[1] = login // tokens[2] = prog // tokens[3] = argumentos
    char *aux1[100];
    char *aux2[100];
    int a=0;
    uid_t uid;
    if(tokens[1] == NULL || tokens[2] == NULL){
        printf("Faltan argumentos\n");
        return 0;
    }
    while(tokens[a+2] != NULL){
        aux1[a] = tokens[a+2];
        a++;
    }
    aux1[a] = NULL;

    uid = UidUsuario(tokens[1]);
    int x = (int) uid;
    char texto[100];
    sprintf(texto ,"%d", x);
    aux2[0] = texto;
    if(setuid(uid) == -1){
        Cmd_setuid(aux2);
        return 0;
    }
    if(execvp(tokens[2], aux1) == -1){
        perror("Error al ejecutar");
        return 0;
    }
    return 0;
}

int fgas(char *tokens[], int ntokens, struct Listas *lista){
    char *aux1[100];
    char *aux2[100];
    int a=0;
    uid_t uid;
    pid_t pid;

    if(tokens[1] == NULL || tokens[2] == NULL){
        printf("Faltan argumentos\n");
        return 0;
    }
    while(tokens[a+2] != NULL){
        aux1[a] = tokens[a+2];
        a++;
    }
    aux1[a] = NULL;

    uid = UidUsuario(tokens[1]);
    int x = (int) uid;
    char texto[100];
    sprintf(texto ,"%d", x);
    aux2[0] = texto;
    if(setuid(uid) == -1){
        Cmd_setuid(aux2);
        return 0;
    }
    if((pid = fork()) == 0){
        execvp(tokens[2], aux1);
        perror("No ejecutado");
        return 0;
    }
    waitpid(pid, NULL, 0);
    return 0;
}

int bgas(char *tokens[], int ntokens, struct Listas *lista){
    char *aux1[100];
    char *aux2[100];
    int a=0;
    uid_t uid;
    pid_t pid;

    if(tokens[1] == NULL || tokens[2] == NULL){
        printf("Faltan argumentos\n");
        return 0;
    }
    while(tokens[a+2] != NULL){
        aux1[a] = strdup(tokens[a+2]);
        a++;
    }
    aux1[a] = NULL;

    uid = UidUsuario(tokens[1]);
    int x = (int) uid;
    char texto[100];
    sprintf(texto ,"%d", x);
    aux2[0] = texto;
    if(setuid(uid) == -1){
        Cmd_setuid(aux2);
        return 0;
    }
    if((pid = fork()) == 0){
        execvp(tokens[2], aux1);
        perror("No ejecutado");
        return 0;
    }
    struct background_info *nodo = malloc(sizeof(struct background_info));
    //--------------------------------------------------------------------------
    int z=0;
    // COMMAND
    while(aux1[z] != NULL){
        nodo->command[z] = aux1[z];
        z++;
    } //Guardo en nodo->command el comando ejecutado
    nodo->command[z] = NULL; // para que al recorrerlo en la funcion print, pare al llegar a NULL
    // DATE
    char time[100];
    tiempo_actual(time);
    strcpy(nodo->date, time);
    // PID
    nodo->pid = pid;
    // USER
    strcpy(nodo->user, tokens[1]);
    // STATUS
    nodo->valor = 0;
    //--------------------------------------------------------------------------
    insert(&lista->background_list, nodo); //Guardo en la lista background
    return 0;
}

int Senal(char * sen) { /*devuel el numero de senial a partir del nombre*/
    int i;
    for (i=0; sigstrnum[i].nombre!=NULL; i++)
        if (!strcmp(sen, sigstrnum[i].nombre))
            return sigstrnum[i].senal;
    return -1;
}

char *NombreSenal(int sen) /*devuelve el nombre senal a partir de la senal*/
{ /* para sitios donde no hay sig2str*/
    int i;
    for (i=0; sigstrnum[i].nombre!=NULL; i++)
        if (sen == sigstrnum[i].senal)
            return sigstrnum[i].nombre;
    return ("SIGUNKNOWN");
}

void print_background(struct background_info *nodo){
    char char_ret_value[100];
    char aux1[100];
    int a, x, sen;

    // PRIORITY
    nodo->prio = getpriority(PRIO_PROCESS, nodo->pid);
    // STATUS

    if(waitpid(nodo->pid, &nodo->valor, WNOHANG |WUNTRACED |WCONTINUED) == nodo->pid){
        if(WIFEXITED(nodo->valor)){ // si termino normal
            nodo->ret_value = WEXITSTATUS(nodo->valor);
            strcpy(nodo->status, "TERMINADO");
        }else if(WIFSIGNALED(nodo->valor)){ // si termino por señal
            sen = WTERMSIG(nodo->valor);
            nodo->ret_value = sen;
            strcpy(nodo->status, "SENALADO");
        }else if(WIFSTOPPED(nodo->valor)){ // si fue parado por señal
            sen = WSTOPSIG(nodo->valor);
            nodo->ret_value = sen;
            strcpy(nodo->status, "PARADO");
        }else if(WIFCONTINUED(nodo->valor)) { // si se continuo con SIGCONT
            nodo->ret_value = 0;
            strcpy(nodo->status, "ACTIVO");
        }
        nodo->leido = 1;
    }else{
        if(WIFEXITED(nodo->valor)){
            if(kill(nodo->pid, 0) == 0){
                nodo->ret_value = 0;
                strcpy(nodo->status, "ACTIVO");
            }else {
                nodo->ret_value = WEXITSTATUS((nodo->valor));
                strcpy(nodo->status, "TERMINADO");
            }
        }else if(WIFSIGNALED(nodo->valor)){
            if(kill(nodo->pid, 0) == 0){ //Si sigue activo
                nodo->ret_value = 0;
                strcpy(nodo->status, "ACTIVO");
            }else{
                sen = WTERMSIG(nodo->valor);
                nodo->ret_value = sen;
                strcpy(nodo->status, "SENALADO");
            }
        }else if(WIFSTOPPED(nodo->valor)){
            sen = WSTOPSIG(nodo->valor);
            nodo->ret_value = sen;
            strcpy(nodo->status, "PARADO");
        }else if(WIFCONTINUED(nodo->valor)) {
            nodo->ret_value = 0;
            strcpy(nodo->status, "ACTIVO");
        }
    }
    a=1;
    strcpy(aux1, nodo->command[0]);
    while(nodo->command[a] != NULL){
        strcat(aux1, " ");
        strcat(aux1, nodo->command[a]);
        a++;
    }
    if(nodo->ret_value == 0){
        strcpy(char_ret_value, "000");
    }else{
        strcpy(char_ret_value, NombreSenal(nodo->ret_value));
    }
    printf("%6d %8s p=%d %13s %10s (%s)  %s\n", nodo->pid, nodo->user, nodo->prio, nodo->date, nodo->status,
           char_ret_value, aux1);
}

int listjobs(char *tokens[], int ntokens, struct Listas *lista){
    for(pos p = first(lista->background_list); !end(lista->background_list, p); p = next(lista->background_list, p)) {
        struct background_info *nodo = get(lista->background_list, p);
        print_background(nodo);
    }
    return 0;
}

pos search_pid(list *background_list, pid_t pid){
    for(pos p = first(*background_list); !end(*background_list, p); p = next(*background_list, p)) {
        struct background_info *nodo = get(*background_list, p);
        if(nodo->pid == pid){
            return p;
        }
    }
    return NULL;
}

int job(char *tokens[], int ntokens, struct Listas *lista){
    pos p;
    if(tokens[1] == NULL || tokens[2] == NULL){
        listjobs(tokens, ntokens, lista); // Si no se dio el pid
        return 0;
    }
    if(strcmp(tokens[1], "-fg") == 0){
        if((p = search_pid(&lista->background_list, (pid_t)atoi(tokens[2]))) == NULL){ // si no se encuentra el pid
            listjobs(tokens, ntokens, lista);
        }else{
            struct background_info *nodo = get(lista->background_list, p);
            //PASAR DE BACKGROUND A FOREGROUND
            waitpid(nodo->pid, &nodo->valor, 0); // No me hace falta &status porque voy a eliminar de la lista ya
            if(WIFEXITED(nodo->valor)){
                nodo->ret_value = WEXITSTATUS(nodo->valor);
                printf("Proceso %d terminado normalmente. Valor devuelto %d\n", nodo->pid, nodo->ret_value);
            }else if(WIFSIGNALED(nodo->valor)) {
                int sen;
                sen = WTERMSIG(nodo->valor);
                nodo->ret_value = sen;
                printf("Proceso %d terminado con señal %s.\n", nodo->pid, NombreSenal(sen));
            }
            free_node(&lista->background_list, p);
        }
        return 0;
    }
    if((p = search_pid(&lista->background_list, (pid_t)atoi(tokens[1]))) == NULL){
        listjobs(tokens, ntokens, lista);
        return 0;
    }
    struct background_info *nodo = get(lista->background_list, p);
    print_background(nodo); // Imprime la info solo del pid indicado

    return 0;
}

pos search_status(list *background_list, char status[100]){
    for(pos p = first(*background_list); !end(*background_list, p); p = next(*background_list, p)) {
        struct background_info *nodo = get(*background_list, p);
        if(strcmp(nodo->status, status) == 0){
            return p;
        }
    }
    return NULL;
}

pos search_signal(list *background_list){
    for(pos p = first(*background_list); !end(*background_list, p); p = next(*background_list, p)) {
        struct background_info *nodo = get(*background_list, p);
        if(nodo->ret_value != 0){
            return p;
        }
    }
    return NULL;
}

pos search_normal(list *background_list){
    for(pos p = first(*background_list); !end(*background_list, p); p = next(*background_list, p)) {
        struct background_info *nodo = get(*background_list, p);
        if(nodo->ret_value == 0){
            return p;
        }
    }
    return NULL;
}

int borrarjobs(char *tokens[], int ntokens, struct Listas *lista){
    if(tokens[1] == NULL){
        listjobs(tokens, ntokens, lista);
        return 0;
    }

    pos p;
    if(strcmp(tokens[1], "-term") == 0){ // Terminados normal
        p = search_normal(&lista->background_list); // Busca en la lista acabados con señal
        while(p != NULL){
            free_node(&lista->background_list, p); // elimino el nodo
            p = search_normal(&lista->background_list);// Busco el siguiente
        }
        return 0;
    }
    if(strcmp(tokens[1], "-sig") == 0){ // Terminados por señal
        p = search_signal(&lista->background_list); // Busca en la lista acabados con señal
        while(p != NULL){
            free_node(&lista->background_list, p); // elimino el nodo
            p = search_signal(&lista->background_list);// Busco el siguiente
        }
        return 0;
    }
    if(strcmp(tokens[1], "-all") == 0){ // Terminados
        p = search_status(&lista->background_list, "TERMINADO");
        while(p != NULL){
            free_node(&lista->background_list, p); // elimino el nodo
            p = search_status(&lista->background_list, "TERMINADO");// Busco el siguiente
        }
        return 0;
    }
    if(strcmp(tokens[1], "-clear") == 0){ // Todos
        clear(&lista->background_list); //Limpia toda la lista
        return 0;
    }
    listjobs(tokens, ntokens, lista);
    return 0;
}

void cualquier_comando_back(char *tokens[], struct Listas *lista){
    char *aux1[100];
    int a=0;
    pid_t pid;

    if(strcmp(tokens[0], "&") == 0){
        return;
    }
    while(tokens[a] != NULL && strcmp(tokens[a], "&") != 0){
        aux1[a] = strdup(tokens[a]);
        a++;
    }
    aux1[a] = NULL;

    if((pid = fork()) == 0){
        execvp(tokens[0], aux1);
        perror("No ejecutado");
        return;
    }
    struct background_info *nodo = malloc(sizeof(struct background_info));
    //--------------------------------------------------------------------------
    int x=0;
    while(aux1[x] != NULL){
        nodo->command[x] = aux1[x];
        x++;
    } //Guardo en nodo->command el comando ejecutado
    nodo->command[x] = NULL;

    // DATE
    char time[100];
    tiempo_actual(time);
    strcpy(nodo->date, time);
    // PID
    nodo->pid = pid;
    // USER
    uid_t uid = getuid();
    strcpy(nodo->user, NombreUsuario(uid));
    // STATUS
    nodo->valor = 0;
    //--------------------------------------------------------------------------
    insert(&lista->background_list, nodo); //Guardo en la lista background
}

int cualquier_comando(char *tokens[], int ntokens, struct Listas *lista){
    pid_t pid;
    int a=0;

    while(tokens[a] != NULL){
        if(strcmp(tokens[a], "&") == 0){
            cualquier_comando_back(tokens, lista);
            return 0;
        }
        a++;
    }
    if((pid = fork()) == 0){
        execvp(tokens[0], tokens);
        perror("No ejecutado");
        return 0;
    }
    waitpid(pid, NULL, 0);

    return 0;
}