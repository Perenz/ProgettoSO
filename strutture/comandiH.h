#ifndef __COMANDI_H__
#define __COMANDI_H__
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct{
    char nome[10];
    char stato[10];
}interruttore;

typedef struct{
    int id;
    int pid;
    char tipo[10];
    char nome[15];
    char stato[10];
    interruttore interruttore[3];
    double time;
    double delay;
    int percentuale;
    int temperatura;
    int def; //default, nuovo dispositivo
}info;

typedef struct{
    char tipo_comando;
    int id;//se == 0 --> list
    int forzato;//utile per delete --all e per far sì che i figli rispondano con terminazione 
    int info_forzate;
    int profondita;
    info info_disp;
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
    info info_disp;
}risp;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
  risp *array;
  size_t used;
  size_t size;
}array_risposte;

void print_risp(risp);
void print_cmd(cmd);

/*
void printComando(cmd c);
void printRisposta(risposta r);
*/
#endif