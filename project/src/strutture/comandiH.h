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




typedef struct{
    int considera;// se = 1 stampo la risposta
    int id;
    int pid;
    int id_padre;//usato per impartire il comando di linking 
    int profondita;//usato per l'indentazione
    int termina_comunicazione;//parametro che indica il termine della comunicazione
    int eliminato;//il dispositivo è stato eliminato se = 1 così che il padre possa rimuoverlo dalla lista dei figli
    int dispositivo_interazione;//se = 1 segnala che il linking non può essere fatto
    int errore;
    info info_disp;//informazioni relative al dispositivo che da la risposta
}risp;

#endif