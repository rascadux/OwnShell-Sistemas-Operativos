#ifndef __P3_H__
#define __P3_H__

extern char **environ;

int priority(char *[], int, struct Listas *);
int rederr(char *[], int, struct Listas *);
int entorno(char *[], int, struct Listas *);
int mostrarvar(char *[], int, struct Listas *);
int BuscarVariable (char *, char *[]);
int cambiarvar(char *[], int, struct Listas *);
int uid(char *[], int, struct Listas *);
int fork_fun(char *[], int, struct Listas *);
int ejec(char*[], int, struct Listas *);
int ejecpri(char*[], int, struct Listas *);
int fg(char*[], int, struct Listas *);
int fgpri(char*[], int, struct Listas *);
int back(char*[], int, struct Listas *);
int backpri(char*[], int, struct Listas *);
int ejecas(char*[], int, struct Listas *);
int fgas(char*[], int, struct Listas *);
int bgas(char*[], int, struct Listas *);
int listjobs(char*[], int, struct Listas *);
int job(char*[], int, struct Listas *);
int borrarjobs(char*[], int, struct Listas *);
int cualquier_comando(char*[], int, struct Listas *);

#endif
