#ifndef __COMANDI_H__
#define __COMANDI_H__
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


typedef struct{
    char tipo_comando;
    int id;//se == 0 --> list
    int forzato;//utile per delete --all e per far sì che i figli rispondano con terminazione 
    int profondita;
    //aggiungere vari campi per comandi più complessi come link
}cmd;




//PER ORA IMPLEMENTO SOLO QUESTO, AL POSTO CHE INFO ANDRà MESSO DISPOSITIVO CHE SARà UN'ALTRA STRUCT 
typedef struct{
    int considera;
    int id;
    int pid;
    int id_padre;
    int profondita;
    int termina_comunicazione;
    int eliminato;
    int dispositivo_interazione;
    char info[64];
}risp;

typedef struct {
  risp *array;
  size_t used;
  size_t size;
}array_risposte;

void print_risp(risp);
void print_cmd(cmd);


void initArray(array_risposte *a, size_t initialSize);
void insertArray(array_risposte *a, risp element);
void freeArray(array_risposte *a);


/*
void printComando(cmd c);
void printRisposta(risposta r);
*/
#endif