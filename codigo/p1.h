// AUTOR: Daniel Rodriguez Sanchez
// AUTOR: Lucia Docampo Rodriguez

#ifndef __P1_H__
#define __P1_H__

struct delete_options;
struct listing_options;

int crear(char *[], int, struct Listas *);
int borrar(char *[], int, struct Listas *);
int borrarrec(char *[], int, struct Listas *);
int listfich(char *[], int, struct Listas *);
int listdir(char *[], int, struct Listas *);

#endif
