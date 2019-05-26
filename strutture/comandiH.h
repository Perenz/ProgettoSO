#ifndef __COMANDI_H__
#define __COMANDI_H__
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct{
    char nome[15];
    char stato[10];
}interruttore;

typedef struct{
    interruttore accensione;
    double maxTime;
    char override_hub;
}bulb;
typedef struct{
    interruttore apertura;
    interruttore chiusura;
    double maxTime;
    char override_hub;
}window;

typedef struct{
    interruttore apertura;
    interruttore termostato;
    interruttore delayI;
    interruttore percI;
    double delay;
    int percentuale;
    int temperatura;
    double maxTime;
    char override_hub;
}fridge;



typedef struct{
    int id;
    int pid;
    int pid_padre;
    char tipo[10];
    char nome[15];
    char stato[10];
    double time;
    int def; //default, nuovo dispositivo

    bulb lampadina;
    window finestra;
    fridge frigo;
}info;

typedef struct{
    char tipo_comando;
    int id;//se == 0 --> list
    int forzato;//utile per delete --all e per far sì che i figli rispondano anche se l'id non coincide
    int info_forzate;
    int profondita;
    int manuale; //se 1 significa che il comando arriva tramite controllo manuale
    int id2; //caso link
    int id_padre;
    interruttore cmdInterruttore;//interruttore utilizzato nello switch
    info info_disp;//info dispositivo utilizzate nel link
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
    //potrei usare errore e settare gli errori: 
        //1 per dispostivio interazione in fase di linking
        //2 aggiunta dispositivo a timer che possiede già un dispositivo
        //3 override su hub
    int dispositivo_interazione;
    int errore;
    info info_disp;
}risp;


void print_risp(risp);
void print_cmd(cmd);

/*
void printComando(cmd c);
void printRisposta(risposta r);
*/
#endif