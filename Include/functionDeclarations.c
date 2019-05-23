#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include "../strutture/listH.h"
#include "../strutture/comandiH.h"
#include "gestioneComandi.c"
#include "addDevice.c"
//Comandi centralina
int cen_prova(char **args, NodoPtr collegati_list, NodoPtr magazzino_list);
int cen_clear(char **args, NodoPtr collegati_list, NodoPtr magazzino_list);
int cen_exit(char **args, NodoPtr collegati_list, NodoPtr magazzino_list);
int cen_help(char **args, NodoPtr collegati_list, NodoPtr magazzino_list);
int cen_add(char **args, NodoPtr collegati_list, NodoPtr magazzino_list);
int cen_list(char **args, NodoPtr collegati_list, NodoPtr magazzino_list);
int cen_delete(char **args, NodoPtr collegati_list, NodoPtr magazzino_list);
int cen_switch(char **args, NodoPtr collegati_list, NodoPtr magazzino_list);
int cen_info(char **args, NodoPtr collegati_list, NodoPtr magazzino_list);
int cen_link(char **args, NodoPtr collegati_list, NodoPtr magazzino_list);
int cen_power(char **args, NodoPtr collegati_list, NodoPtr magazzino_list);
int cen_unlink(char **args, NodoPtr collegati_list, NodoPtr magazzino_list);
void getManualPid();

int cen_numCommands();
int checkPower();




//Variabile intera che memorizza lo stato di accensione della centralina
//1 accesa
//0 spenta
int powerOn=1;

//l'id della centralina è uguale a 2 --> potremo generalizzare e fare che in start.c creo il processo cmd che comunica con la centralina tramite pipe il
//comando che deve essere eseguito --> centralina ha id = 2 o 1, è più fico id == 1

//Lista dei comandi della centralina
char *builtin_cmd[]={
        "prova",
        "clear",
        "help",
        "exit",
        "add",
        "list",
        "delete",
        "switch",
        "info",
        "link",
        "unlink",
        "power"
        
        //CREA NUOVO COMANDO INFOMANUALE 
        //TODO
};

//Puntatore alle funzioni associati alla lista dei comandi builtin_cmd
int (*builtin_func[]) (char **, NodoPtr, NodoPtr) = {
        &cen_prova,
        &cen_clear,
        &cen_help,
        &cen_exit,
        &cen_add,
        &cen_list,
        &cen_delete,
        &cen_switch,
        &cen_info,
        &cen_link,
        &cen_unlink,
        &cen_power
};
int cen_numCommands(){
    return (sizeof(builtin_cmd)/ sizeof(char*));
}


int cen_prova(char **args, NodoPtr collegati_list, NodoPtr magazzino_list){
    printf("Hai inserito il comando %s\n", args[0]);
    return 1;
}
int cen_clear(char **args, NodoPtr collegati_list, NodoPtr magazzino_list){
    if(powerOn==0){
        printf("La centralina risulta spento tramite interrutore generale\n");
        printf("Accendere la centralina tramite 'power' prima di impartire nuovi comandi\n");

        return 1;
    }
    system("clear");
    return 1;
}
int cen_exit(char **args, NodoPtr collegati_list, NodoPtr magazzino_list){
    if(powerOn==0){
        printf("La centralina risulta spento tramite interrutore generale\n");
        printf("Accendere la centralina tramite 'power' prima di impartire nuovi comandi\n");

        return 1;
    }
    signal(SIGQUIT, SIG_IGN);
    while(collegati_list!=NULL){
            kill(collegati_list->data, SIGQUIT);
            collegati_list=collegati_list->next;
    }
    while(magazzino_list!=NULL){
            kill(magazzino_list->data, SIGQUIT);
            magazzino_list=magazzino_list->next;
    }
    //Dealloco le liste

    freeList(collegati_list);
    freeList(magazzino_list);

    //Rimuovo la fifo usata dal manuale per ottenere il pid dei dispositivi
    remove("/tmp/manCenFifo");

    return 0;
}
int cen_help(char **args, NodoPtr collegati_list, NodoPtr magazzino_list){
    printf("Progetto SO realizzato da: Paolo Tasin, "
           "Stefano Perenzoni, Marcello Rigotti\n");
    printf("Centralina per controllo domotico\n");
    printf("Digitare i seguenti comandi:\n");

    int i;
    for(i=0; i<cen_numCommands();i++){
        printf("> %s\n", builtin_cmd[i]);
    }
    return 1;
}
/*
    Funzione: elenca tutti i dispositivi con <nome>, quelli attivi con <nome> <id> 
    Sintassi lato utente: list
    Sintassi comunicata dalla centralina ai figli: l
*/
int cen_list(char **args, NodoPtr collegati_list, NodoPtr magazzino_list){ 
    if(powerOn==0){
        printf("La centralina risulta spento tramite interrutore generale\n");
        printf("Accendere la centralina tramite 'power' prima di impartire nuovi comandi\n");

        return 1;
    }
    signal(SIGCONT, sign_cont_handler);

    //RICEZIONE RISPOSTE
    risp* array_risposte_collegati_list;
    malloc_array(&array_risposte_collegati_list, N_MAX_DISP);

    risp* array_risposte_magazzino_list;
    malloc_array(&array_risposte_magazzino_list, N_MAX_DISP);

    cmd comando;
    comando.tipo_comando = 'l';
    comando.profondita = 0;
    int n = 0;
    printf("\n\tStampo la lista dei dispositivi COLLEGATI:\n");
    printf("\nCENTRALINA VAGINA\n");
    n = broadcast_centralina(collegati_list, comando, array_risposte_collegati_list);  
    printRisp(array_risposte_collegati_list, n, 1);

    
    printf("\n\tStampo la lista dei dispositivi DISPONIBILI:\n");
    //gestione eerr
    n = broadcast_centralina(magazzino_list, comando, array_risposte_magazzino_list);
    printRisp(array_risposte_magazzino_list, n, 1);
    printf("Numero dispositivi: %d\n", n);



    free(array_risposte_collegati_list);
    free(array_risposte_magazzino_list);
    return 1;
}
/*
    Funzione: rimuove il dispositivo <id>, se di controllo elimina anche i dispositivi sottostanti
    Sintassi lato utente:                          delete <id>
    Sintassi comunicata dalla centralina ai figli: d <id>
*/
//delete funziona su magazzino_list per ora
int cen_delete(char **args, NodoPtr collegati_list, NodoPtr magazzino_list){
    if(powerOn==0){
        printf("La centralina risulta spento tramite interrutore generale\n");
        printf("Accendere la centralina tramite 'power' prima di impartire nuovi comandi\n");

        return 1;
    }
    //TODO da modificare, pensavo che l'eliminazione avvenisse anche per tipo.
    if(args[1]==NULL /*|| atoi(args[1]) < 2*/){
        printf("Argomenti non validi\n");
        printf("Utilizzo: delete <id>\n");
        return 1;
    }else{
        //RICEZIONE RISPOSTE
        risp* array_risposte_collegati_list;
        malloc_array(&array_risposte_collegati_list, N_MAX_DISP);

        risp* array_risposte_magazzino_list;
        malloc_array(&array_risposte_magazzino_list, N_MAX_DISP);
        int n;
        signal(SIGCONT, sign_cont_handler);
        cmd comando;
        comando.tipo_comando = 'd';
        comando.forzato = 0;

        if(strcmp(args[1], "--all")==0){
            comando.forzato = 1;
        }else{
            comando.forzato = 0;
            comando.id = atoi(args[1]);
        }

        n = broadcast_centralina(magazzino_list, comando, array_risposte_magazzino_list);
        printRisp(array_risposte_magazzino_list, n, 1);
        //gestione err
        n = broadcast_centralina(collegati_list, comando, array_risposte_collegati_list);
        printRisp(array_risposte_collegati_list, n, 1);
        free(array_risposte_collegati_list);
        free(array_risposte_magazzino_list);
    }
    /*GESTIONE ID non esistente
    printf("Device indicato non riconosciuto\n");
    printf("Utilizzo: delete <id>\n");
    */
    
    return 1;
}

//Unlink funziona solo sui dispositivi collegati
int cen_unlink(char **args, NodoPtr collegati_list, NodoPtr magazzino_list){
    if(powerOn==0){
        printf("La centralina risulta spento tramite interrutore generale\n");
        printf("Accendere la centralina tramite 'power' prima di impartire nuovi comandi\n");

        return 1;
    }
    //TODO da modificare, pensavo che l'eliminazione avvenisse anche per tipo.
    if(args[1]==NULL /*|| atoi(args[1]) < 2*/){
        printf("Argomenti non validi\n");
        printf("Utilizzo: delete <id>\n");
        return 1;
    }else{
        //RICEZIONE RISPOSTE
        risp* array_risposte_collegati_list;
        malloc_array(&array_risposte_collegati_list, N_MAX_DISP);
        int n;
        signal(SIGCONT, sign_cont_handler);
        cmd comando;
        //Elimino tutti i dispositivi e poi li riaggiungo al magazzino_list 
        comando.tipo_comando = 'd';
        if(strcmp(args[1], "--all")==0){
            comando.forzato = 1;
        }else{
            comando.forzato = 0;
            comando.id = atoi(args[1]);
        }
        n = broadcast_centralina(collegati_list, comando, array_risposte_collegati_list);
        printRisp(array_risposte_collegati_list, n, 0);

        int j, i;

        //C'È LA FUNZIONE STAMPA RISPOSTE
        for(i=0; i < n; i++){
            for(j=0; j<device_number(); j++){
                if(strcmp(array_risposte_collegati_list[i].info_disp.tipo, builtin_device[j])==0){
                    add_device_generale(builtin_dev_path[j], magazzino_list, array_risposte_collegati_list[i].info_disp, NULL);
                }
            }
        }

        free(array_risposte_collegati_list);

    }
    /*GESTIONE ID non esistente
    printf("Device indicato non riconosciuto\n");
    printf("Utilizzo: delete <id>\n");
    */
    
    return 1;
}
/*
    Funzione: aggiunge un device al sistema e ne mostra i dettagli
    Sintassi lato utente:                          add <tipo>
    TODO Sintassi comunicata dalla centralina :    add <tipo> (centralina comunica a processo specifico)
*/
int cen_add(char **args, NodoPtr collegati_list, NodoPtr magazzino_list){
    char nome[20];
    if(powerOn==0){
        printf("La centralina risulta spento tramite interrutore generale\n");
        printf("Accendere la centralina tramite 'power' prima di impartire nuovi comandi\n");

        return 1;
    }
    if(args[1]==NULL){
            printf("Argomenti non validi\n");
            printf("Utilizzo: add <device>\n");
            printf("Comando 'device' per vedere la lista di quelli disponibili\n");
            return 1;
    }
    //3 device disponibili: bulb, window, fridge
    else{
        if(args[2]==NULL){
            //Devo concatenare nome del device piu id 
            //Es: bulb3
            
            sprintf(nome, "%s%d", args[1], id_gen);
        }
        else{
            //strcpy(nome,args[2]);
            //strcpy(nome, "Prova");
            strcpy(nome, args[2]); 
        }
        int i;
        for(i=0; i<device_number(); i++){
            if(strcmp(args[1], builtin_device[i])==0){
                    id_gen+=1;
                    return add_device(builtin_dev_path[i], magazzino_list, nome);//viene sempre aggiunto prima al magazzino
            }
        }
    }

    printf("Device indicato non riconosciuto\n");
    printf("Utilizzo: add <device>\n");
    printf("Digitare il comando 'device' per la lista di quelli disponibili\n");

    return 1;        
}

/*
    Funzione: modifica <stato> di <label> del dispositivo con <id>
        (label sta per interruttore o termostato) //per ora realizzo solo interruttore
    Sintassi lato utente:                          switch <id> <label> <pos>
    Sintassi comunicata dalla centralina :         s <id> <label> <stato>
*/
//TODO
    //Cambiare sintassi <label> <stato> //da generalizzare AAAAAAèAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
int cen_switch(char **args, NodoPtr collegati_list, NodoPtr magazzino_list){
    if(powerOn==0){
        printf("La centralina risulta spento tramite interrutore generale\n");
        printf("Accendere la centralina tramite 'power' prima di impartire nuovi comandi\n");

        return 1;
    }
    //TODO cambiare controllo, potrei farlo nel dispositivo
    if(args[1]==NULL || args[2]==NULL || args[3]==NULL){         //primo argomento diverso da id (errori, bisogna verificar sia un numero)
        printf("Argomenti non validi\n");
        printf("Utilizzo: switch <id> <label> <pos>\n");
        printf("Comando 'device' per vedere la lista di quelli disponibili\n");
        return 1;
    }else{
        /*
        signal(SIGCONT, sign_cont_handler);
        char* comando = malloc(4 + strlen(args[1]) + strlen(args[2]) + strlen(args[3]));//7 per il comando, 4 per spazi di sep. e la terminazione
        //comando
        sprintf(comando, "s %s %s", args[1], args[2]);
        strcat(comando,(isdigit(args[3])?(args[3]):(args[3])));
        //printf("scrittura lato padre: %s\n", comando);
        char* answer = broadcast(collegati_list, NULL, comando);
        if(strcmp(answer, "0")!=0){//ha trovato il dispositivo
            printf("%s\n", answer);//stampiamo una qualche risposta daje
            free(comando);
            free(answer);
        }else{//non ho trovato nessun dispositivo con quell'id
            printf("Nessun elemento ha questo id o errore nel comando\n");    
        }          
        */
        cmd comando;
        comando.forzato=0;
        comando.tipo_comando = 's';
        comando.id = atoi(args[1]);
        strcpy(comando.info_disp.interruttore[0].nome,args[2]);
        strcpy(comando.info_disp.interruttore[0].stato,args[3]);

        risp* array_risposte_magazzino_list;
        malloc_array(&array_risposte_magazzino_list, N_MAX_DISP);
        risp* array_risposte_collegati_list;
        malloc_array(&array_risposte_collegati_list, N_MAX_DISP);

        int n = broadcast_centralina(magazzino_list, comando, array_risposte_magazzino_list);
        printRisp(array_risposte_magazzino_list, n, 1);

        

        n = broadcast_centralina(collegati_list, comando, array_risposte_collegati_list);
        printRisp(array_risposte_magazzino_list, n, 1);


        free(array_risposte_collegati_list);
        free(array_risposte_magazzino_list);
        return 1; //esci che sennò va avanti    
    }
    printf("Device indicato non riconosciuto\n");
    printf("Utilizzo: del <device> <num>\n");
    printf("Digitare il comando 'device' per la lista di quelli disponibili\n");
        
    return 1;
}
/*
    Funzione: mostra i dettagli del dispositivo 
    Sintassi lato utente:                          info <id>
    TODO Sintassi comunicata dalla centralina :    i <id> (centralina comunica a processo specifico)
*/  //da generalizzare 
int cen_info(char **args, NodoPtr collegati_list, NodoPtr magazzino_list){
    if(powerOn==0){
        printf("La centralina risulta spento tramite interrutore generale\n");
        printf("Accendere la centralina tramite 'power' prima di impartire nuovi comandi\n");

        return 1;
    }

    risp* array_risposte_magazzino_list;
    malloc_array(&array_risposte_magazzino_list, 1);
    risp* array_risposte_collegati_list;
    malloc_array(&array_risposte_collegati_list, 1);

    signal(SIGCONT, sign_cont_handler);
    cmd comando;
    int n;
    comando.tipo_comando = 'i';
    comando.info_forzate = 1;
    comando.id = atoi(args[1]);

    n = broadcast_centralina(collegati_list, comando, array_risposte_collegati_list);
    printRisp(array_risposte_collegati_list, n, 0);
    n = broadcast_centralina(magazzino_list, comando, array_risposte_magazzino_list);
    printRisp(array_risposte_magazzino_list, n, 0);

    free(array_risposte_collegati_list);
    free(array_risposte_magazzino_list);
    //gestione non c'è nessun dispositivo con questo id
    return 1;
}

/*
    Funzione: collega due dispositivi (uno deve essere di controllo) 
    Sintassi lato utente:                          link <id1> to <id2>
    TODO Sintassi comunicata dalla centralina : 
*///AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
int cen_link(char** args, NodoPtr collegati_list, NodoPtr magazzino_list){
    //L'ID DEL PADRE MANCA
    if(powerOn==0){
        printf("La centralina risulta spento tramite interrutore generale\n");
        printf("Accendere la centralina tramite 'power' prima di impartire nuovi comandi\n");

        return 1;
    }
    if(args[1]==NULL || args[2]==NULL || args[3]==NULL )//gestione errori alla Carlina
        return -1;

    //DA SISTEMARE DATO CHE PER ORA HO VARI RETURN E PERCIò NON FACCIO MAI I FREE
    //IL CODICE RISULTA DISORDINATO 
    
    //2 casi da gestire: 
        //caso 1--> l'id2 è uguale a 2
            //a1) chiedo le info di id1 per vedere se si trova in magazzino_list
                //aT) se si trova in magazzino_list sposto id1 in collegati_list
                //aF) else delete di id1 e add alla centralina in collegati_list
        //caso 2-->else
            //chiedo le info di id1 per vedere se è dispositivo di controllo
            //chiedo le info di id2 per vedere se è un dispositivo di interazione collegabile (stesso tipo) ad id1
            //delete id1
            //link id1 to id2
        int id1 = atoi(args[1]);
        int id2 = atoi(args[3]);
        risp* array_risposte_collegati_list = malloc(N_MAX_DISP * sizeof(risp));
        risp* array_risposte_magazzino_list = malloc(N_MAX_DISP * sizeof(risp));
        //ricorda di pulire gli array

        signal(SIGCONT, sign_cont_handler);
        cmd comando;
        int n_id_src;
        int n_id_dst;
        int id_src, id_dst;
        id_src = atoi(args[1]);
        id_dst = atoi(args[3]);
        int i;
        //LINKING CON DESTINAZIONE LA CENTRALINA FUNZIONA
        if(id2 == 2){//collegamento alla centralina, id_dst == centralina_id
            comando.tipo_comando = 'i';
            comando.info_forzate = 1;
            comando.id = id_src;
            //n_id1 rappresenta il numero di dispositivi del sott'albero di id_src
            n_id_src = broadcast_centralina(magazzino_list, comando, array_risposte_magazzino_list);
            if(n_id_src != 0){//il dispositivo da collegare si trova in magazzino_list

                risp dispostivo = array_risposte_magazzino_list[0];
                //MARCELLO TOGLI IL NULL QUANDO HAI SISTEMATO ADD_DEVICE
                //non serve che lo aggiungo a collegati_list dato che è già stato aggiunto in add_device_generale
                //lo rimuovo dal magazzino_list
                removeNode(magazzino_list,  dispostivo.pid);
                //SE USASSI SPOSTA NODE NON SERVE RICREARLO
                for(i=0; i<device_number(); i++)
                    if(strcmp(array_risposte_magazzino_list[0].info_disp.tipo, builtin_device[i])==0)
                        add_device_generale(builtin_dev_path[i], collegati_list, array_risposte_magazzino_list[0].info_disp, NULL); 
            }else{
                comando.tipo_comando = 'i';//info
                n_id_src = broadcast_centralina(collegati_list, comando, array_risposte_collegati_list);   
                if(n_id_src != 0){//il dispositivo da collegare si trova in collegati_list? 
                    //Elimino il dispositivo e i suoi figli
                    comando.tipo_comando = 'd';//delete
                    comando.forzato = 0;//elimina anche i figli
                    n_id_src = broadcast_centralina(collegati_list, comando, array_risposte_collegati_list);
                    //Aggiungo il primo elemento alla centralina
                    for(i=0; i<device_number(); i++)
                        if(strcmp(array_risposte_collegati_list[0].info_disp.tipo, builtin_device[i])==0)
                            add_device_generale(builtin_dev_path[i], collegati_list, array_risposte_collegati_list[0].info_disp, NULL);  
                    //aggiungo a cascata tutti gli altri elementi, sono sicuro che l'ordine sia giusto 
                    //poiché broadcast_centralina restituisce gli elementi in maniera dfs
                    
                    comando.tipo_comando = 'a';//link aggiungendo
                    //potrei aggiungere un controllo se l'array = NULL non aggiungere elementi
                    risp* array_tmp_esito_linking = malloc(1 * sizeof(risp));
                    //parto da 1 perché il primo è già collegato
                    for(i=1; i<n_id_src; i++){
                        comando.id = array_risposte_collegati_list[i].id_padre;
                        comando.id2 = array_risposte_collegati_list[i].id;
                        comando.info_disp = array_risposte_collegati_list[i].info_disp;
                        int tmp = broadcast_centralina(collegati_list, comando, array_tmp_esito_linking);
                    }
                }else{
                    printf("Dispositivo %d non esiste nè nel magazzino_list nè nei dispositivi collegati\n", id_src);
                }
            
            }
        
        }else{//la destinazione del linking non è la centralina
            //LINKING CON DESTINAZIONE GIÁ COLLEGATA E SORGENTE GIÀ COLLEGATA
            comando.tipo_comando = 'i';//info
            comando.id = id_src;
            
            n_id_src = broadcast_centralina(collegati_list, comando, array_risposte_collegati_list);  


            if(n_id_src != 0){//il dispositivo source esiste in collegati
                comando.id = id_dst;
                n_id_dst = broadcast_centralina(collegati_list, comando, array_risposte_collegati_list); 
                if(n_id_dst != 0){//il dispositivo destinazione esiste in collegati_list
                    if(array_risposte_collegati_list[0].dispositivo_interazione !=1){//Il dispositivo di destinazione è un dispositivo di controllo? 
                        comando.tipo_comando = 'd';
                        comando.id = id_src;
                        comando.forzato = 1;//elimina anche i figli
                        comando.forzato = 0;

                        //Elimino il dispositivo source e il suo sottoalbero
                        n_id_src = broadcast_centralina(collegati_list, comando, array_risposte_collegati_list);
                        comando.tipo_comando = 'a';//link aggiungendo
                        //potrei aggiungere un controllo se l'array = NULL non aggiunge elementi
                        risp* array_tmp_esito_linking = malloc(1 * sizeof(risp));

                        comando.id = id_dst; 
                        comando.id2 = id_src;
                        comando.info_disp = array_risposte_collegati_list[0].info_disp;
                        int tmp = broadcast_centralina(collegati_list, comando, array_tmp_esito_linking);
                        //se ha altri figli
                        for(i=1; i<n_id_src; i++){
                            comando.id = array_risposte_collegati_list[i].id_padre;
                            comando.id2 = array_risposte_collegati_list[i].id;
                            comando.info_disp = array_risposte_collegati_list[i].info_disp;
                            int tmp = broadcast_centralina(collegati_list, comando, array_tmp_esito_linking);
                        }
                    }else{//SE È UN DISPOSITIVO DI INTERAZIONE NON POSSO FARE IL LINKING
                        printf("Stai cercando di fare il linking ad un dispositivo di interazione, errore 1\n");
                    }
                    return 1;
                //LINKING CON DESTINAZIONE GIÁ COLLEGATA E SORGENTE NON COLLEGATA
                }/////////

            }else{//verifico se il dispositivo source esiste nel magazzino_list
                comando.tipo_comando = 'i';//info
                comando.id = id_src;
                
                n_id_src = broadcast_centralina(magazzino_list, comando, array_risposte_magazzino_list); 
                    if(n_id_src != 0){//Il dispositivo source esiste in magazzino_list
                        comando.id = id_dst;
                        n_id_dst = broadcast_centralina(collegati_list, comando, array_risposte_collegati_list); 
                        if(n_id_dst != 0){//il dispositivo destinazione esiste in collegati_list
                            if(array_risposte_collegati_list[0].dispositivo_interazione !=1){//Il dispositivo di destinazione è un dispositivo di controllo? 
                                //è come il delinking
                                comando.tipo_comando = 'd';
                                comando.forzato = 0;//elimina anche i figli
                                comando.id = id_src;
                                n_id_src = broadcast_centralina(magazzino_list, comando, array_risposte_magazzino_list);
                                
                                printRisp(array_risposte_magazzino_list, n_id_dst, 0);
                                comando.tipo_comando = 'a';//link aggiungendo

                                risp* array_tmp_esito_linking = malloc(1 * sizeof(risp));

                                comando.id = id_dst; 
                                comando.id2 = id_src;
                                comando.info_disp = array_risposte_magazzino_list[0].info_disp;
                                int tmp = broadcast_centralina(collegati_list, comando, array_tmp_esito_linking);
                                //se ha altri figli
                                for(i=1; i<n_id_src; i++){
                                    comando.id = array_risposte_magazzino_list[i].id_padre;
                                    comando.id2 = array_risposte_magazzino_list[i].id;
                                    comando.info_disp = array_risposte_magazzino_list[i].info_disp;
                                    int tmp = broadcast_centralina(collegati_list, comando, array_tmp_esito_linking);
                                }
                            }else{
                                printf("Stai cercando di fare il linking ad un dispositivo di interazione, errore 2\n");
                            }
                        }
                }
                return 1;
            }
            
            

        }


        
        free(array_risposte_collegati_list);
        free(array_risposte_magazzino_list);    



    return 1;
    
}



risp manualCen_info(char *arg, NodoPtr collegati_list, NodoPtr magazzino_list){
    risp* array_risposte_collegati_list;
    risp* array_risposte_magazzino_list;
    malloc_array(&array_risposte_collegati_list, 1);
    malloc_array(&array_risposte_magazzino_list, 1);

    signal(SIGCONT, sign_cont_handler);
    cmd comando;
    risp dispCercato;
    dispCercato.info_disp.pid=-1;
    comando.tipo_comando = 'm';
    comando.id = atoi(arg);
    if(broadcast_centralina(collegati_list, comando, array_risposte_collegati_list)>0){
        dispCercato=array_risposte_collegati_list[0];
    }
    else if(broadcast_centralina(magazzino_list, comando, array_risposte_magazzino_list)>0){
        dispCercato=array_risposte_magazzino_list[0];
    }


    free(array_risposte_collegati_list);
    free(array_risposte_magazzino_list);
    //gestione non c'è nessun dispositivo con questo id 

    //Se non trova ritorna -1
    return dispCercato;
}

//Per gestire l'accensione/spegnimento generale della centralina
int cen_power(char **args, NodoPtr collegati_list, NodoPtr magazzino_list){
    NodoPtr nodo;
    if(powerOn==1){ //è acessa allora la spengo
        //TODO
        //Devo mandare in pausa tutti i dispositivi
        //Su entrambe le liste
    
        nodo=collegati_list;
        while(nodo!=NULL){
            kill(SIGINT, nodo->data);
            nodo=nodo->next;
        }
        nodo=magazzino_list;
        while(nodo!=NULL){
            kill(SIGINT, nodo->data);
            nodo=nodo->next;
        }
        powerOn=0;
        printf("Centralina spenta\n");
    }
    else if(powerOn==0){ //è spenta allora la accendo
        //TODO
        //Slocco dalla puasa tutti i dispositivi 
        //Su entrambe le liste
        nodo=collegati_list;
        while(nodo!=NULL){
            kill(SIGCONT, nodo->data);
            nodo=nodo->next;
        }
        nodo=magazzino_list;
        while(nodo!=NULL){
            kill(SIGCONT, nodo->data);
            nodo=nodo->next;
        }
        powerOn=1;
        printf("Centralina accesa\n");
    }
}

//Funzione per ottenre l'ID del dispositivo che è richiesto dal manuale 
//sul quale poi tramite info() si ottiene il PID che viene poi passato al manuale
void getManualPid(NodoPtr collegati_list, NodoPtr magazzino_list){
    //Se ci sono entrato significa che è arrivato sigusr2 da manuale
    char msg[20];
    char** args;
    risp dispCercato;
    int fd;
    char *manCenFifo = "/tmp/manCenFifo";
    //Apro la fifo in lettura
    fd=open(manCenFifo, O_RDONLY);
    read(fd, msg, 20);
    //Chiudo lettura 
    close(fd);

    args = splitLine(msg);
    if(strcmp(args[0], "contpid")==0){
        //printf("Letto msg correto 'contpid'\n");
        //printf("Ho ricevuto il pid %s\n", args[1]);    

        //Tramite info(modificato) ricavo il pid corrispondente all'id passato

        //CAMBIA SOLO QUI
        //CON il nuovo info devo prendere pidCercato
        dispCercato = manualCen_info(args[1], collegati_list, magazzino_list); 
        //printf("Questo è il pid cercato %d\n", pidCercato); 
    }
    //Comunico il pid cercato al manuale
    //Apro la pipe in scrittura
    fd=open(manCenFifo, O_WRONLY);
    //sprintf(msg, "%d %s", dispCercato.info_disp.pid);
    int esito= write(fd, &dispCercato, sizeof(dispCercato));
    //printf("Cen ha scritto a manuale con esito %d", esito);


    //printf("Sono nel getManualPid della centralina\n");
    //Chiudo la fifo in lettura
    free(args);
  
    return;
}
