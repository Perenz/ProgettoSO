#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include "../strutture/listH.h"
#include "../strutture/list.c"
#include "../Include/gestioneComandi.c"
#include "../Include/addDevice.c"

//voglio usare le funzioni definite in functionDeclaration urca
#define CEN_BUFSIZE 128



NodoPtr dispList; //lista dei dispositivi collegati all'hub
pid_t idPar;
pid_t pid;
int id;
int status; //1 acceso, 0 spento
//File descriptor in cui il figlio legge e il padre scrive
int fd_read;
//File descriptor in cui il figlio scrive e il padre legge
int fd_write;
info informazioni;
char nome[20];
int broadcast_controllo(NodoPtr list, cmd comando, int pid_papi, int fd_papi, risp risposta_to_padre);
int dev_list(cmd);
int dev_switch(cmd);
int dev_info(cmd);
int dev_delete(cmd);
int dev_link(cmd);

void get_info_string(info*);//TODO aggiungere timer
void set_info(char*);

//int dev_link(char** args);
int device_handle_command(cmd);
void h_sigstop_handler ( int sig ) ;
//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
int dev_add(char*, char*);


void signhandle_quit(int sig){
    signal(SIGQUIT, signhandle_quit);
    _exit(0);
}




char *builtin_command[5]={
    "l",//list
    "s",//switch
    "i",//getInfo
    "d", //delete
    "a" //addDevice    da cambiare scegliendo lettera corrispondente
};
int (*builtin_func_hub[]) (cmd comando) = {
        &dev_list,
        &dev_switch,
        &dev_info,
        &dev_delete,
        &dev_link,
};
int cont_numCommands(){
    return (sizeof(builtin_command)/ sizeof(char*));
}

void sign_cont_handler_hub(int sig){
    signal(SIGCONT, sign_cont_handler_hub);
    return;

}

void sigint_handler(int sig){
    //Devo mandare il SIGINT a tutti i suoi figli
    NodoPtr nodo = dispList;
    while(nodo!=NULL){
        kill(SIGINT, nodo->data);
        nodo=nodo->next;
    }
    //Come per bulb non serve andare in pausa
}

//SIGUSR1 usato per l'implementazione della lettura della pipe con il padre
void sighandle_usr1(int sig){
    signal(SIGCONT, sign_cont_handler);
    signal(SIGUSR1, sighandle_usr1);
    if(sig == SIGUSR1){
        cmd comando;
        int err_signal;
        read(fd_read, &comando, sizeof(cmd));
        int errnum = device_handle_command(comando);
        //printf("Termino %d\n", id);
        err_signal = kill(idPar, SIGCONT);
        if(err_signal != 0)
            perror("errore in invio segnale");

        //printf("Termina in modo adeguato.\n");
        return;
    }
}

//USATO PER SVEGLIARE IL PROCESSO
void sighandle_usr2(int sig){
    signal(SIGUSR2, sighandle_usr2);
    if(sig == SIGUSR2){
        return;
    }
}

int device_handle_command(cmd comando){
    //da fare come in functionDeclarations in file dispositivi
    int i;
    for(i=0; i<cont_numCommands(); i++){
        char tmp = *builtin_command[i];
        if(comando.tipo_comando==tmp)
            return builtin_func_hub[i](comando);
    }
    return 1;
}
int rispondi(risp risposta_controllore, cmd comando){
    risposta_controllore.termina_comunicazione = 0;
    risposta_controllore.pid = pid;

    //vado io in controllo e mando le varie risposte al papi
    //attenz, buono che salto il primo
    broadcast_controllo(dispList, comando, idPar, fd_write, risposta_controllore);
    return 1;
}

int dev_list(cmd comando){
    risp risposta_controllore;
    risposta_controllore.profondita = comando.profondita+1;
    comando.profondita++;
    risposta_controllore.id = id;
    risposta_controllore.considera = 1;
    info infoD;
    get_info_string(&(risposta_controllore.info_disp));
    

    rispondi(risposta_controllore, comando);


    return 1;
}
int dev_switch(cmd comando){
    risp risposta_controllore;
    if(comando.id == id || comando.forzato){
        comando.forzato = 1;
        risposta_controllore.id = id;
        risposta_controllore.considera = 1;
        risposta_controllore.pid = pid;
        risposta_controllore.profondita = comando.profondita+1;
        
    }else{
        risposta_controllore.considera = 0;
    }
    rispondi(risposta_controllore, comando);
    return 1;
}
int dev_info(cmd comando){
    risp risposta_controllore;
    if(comando.id == id || comando.forzato){//comando --all , forzato forza l'invio delle info anche se l'id non è uguale
            //il parametro info_forzate è usato nel link: forza i dispositivi nell'invio delle proprie informazioni 
            //se figli di un hub con tali informazioni 
        if(comando.info_forzate == 1){
            comando.forzato = 1;
        }
        risposta_controllore.id = id;
        risposta_controllore.considera = 1;
        risposta_controllore.pid = pid;
        //char* info = malloc(ANSWER);
        //get_info_string(info);
        
        risposta_controllore.profondita = comando.profondita+1;
        //SE VOGLIAMO FARE CHE IL DISPOSITIVO MANDA UN MESSAGGIO E NON CERCA SE I SUOI FIGLI HANNO LO STESSO ID: 
        risposta_controllore.termina_comunicazione = 0;
        write(fd_write, &risposta_controllore, sizeof(risp));
        risposta_controllore.termina_comunicazione = 1;
        write(fd_write, &risposta_controllore, sizeof(risposta_controllore));
        
        //Se VOGLIAMO FARE CHE IL DISPOSITIVO CHIEDE AI SUOI FIGLI SE C'è QUALCUNO CON QUELL'ID ANCHE SE LUI HA GIà QUELL'ID
        //rispondi(risposta_controllore, comando);
    }else{
        risposta_controllore.considera = 0;//non considerarmi, non sono stato eliminato
       
        rispondi(risposta_controllore, comando);
    }
    return 1;
}
int dev_delete(cmd comando){
    risp risposta_controllore;
    if(comando.forzato == 1 || comando.id == id){//comando --all 
        risposta_controllore.id = id;
        risposta_controllore.considera = 1;
        risposta_controllore.eliminato = 1;
        risposta_controllore.pid = pid;
        comando.forzato = 1;//indico ai miei figli di eliminarsi
        
        get_info_string(&(risposta_controllore.info_disp));
        
        rispondi(risposta_controllore, comando);
    
        exit(0);
    }else{
        risposta_controllore.considera = 0;//non considerarmi, non sono stato eliminato
        risposta_controllore.eliminato = 0;
        get_info_string(&(risposta_controllore.info_disp));
        rispondi(risposta_controllore, comando);
    }
    return 1;
}
int dev_link(cmd comando){
    risp risposta_controllore;
    if(comando.id == id){//comando --all
        int i, err;
        risposta_controllore.considera = 0;
        for(i=0; i<device_number(); i++){
            if(strcmp(comando.info_disp.tipo, builtin_device[i])==0)
                    err = add_device_generale(builtin_dev_path[i], dispList, comando.info_disp, NULL);
        }
        risposta_controllore.termina_comunicazione = 0;
        write(fd_write, &risposta_controllore, sizeof(risp));
        risposta_controllore.termina_comunicazione = 1;
        write(fd_write, &risposta_controllore, sizeof(risposta_controllore));
    }else{
        risposta_controllore.considera = 0;
        rispondi(risposta_controllore, comando);
    }
    return 1;
}
void set_info(char* info){
    char** info_split = splitLine(info);
    //<infoDefault> := default <id>
    if(strcmp(info_split[0], "default")==0){
        id = atoi(info_split[1]);
    }else{
        //<info> := <id>
        id = atoi(info_split[0]);
    }
}

void get_info_string(info* ans){//TODO aggiungere timer
    //TODO 
    strcpy(ans->tipo, "Hub");
    ans->id = id;
    ans->pid = pid;
    strcpy(ans->nome, nome);
    strcpy(ans->stato, status==0?"off":"on");
}


int main(int argc, char **args){
    dispList = listInit(getpid());

    //UGUALE A BULB 
    pid = getpid(); // chiedo il mio pid
    idPar = getppid(); //chiedo il pid di mio padre

    fd_read = atoi(args[1]);
    fd_write = atoi(args[2]);
    //MANCA IL SET_INFO, sbaglia l'id
    int err = read(fd_read,&informazioni,sizeof(info));
    if(err == -1)
        printf("Errore nella lettura delle info BULB");
    id = informazioni.id;
    if(informazioni.def == 1){
        status = 0; 
        informazioni.pid = pid;
    }else{
        
        informazioni.pid = pid;
        strcpy(nome, informazioni.nome);
    }

    signal(SIGINT, sigint_handler);
    signal(SIGCONT, sign_cont_handler_hub);//Segnale per riprendere il controllo 
    signal(SIGQUIT, signhandle_quit);
    signal(SIGUSR1, sighandle_usr1); //imposto un gestore custom che faccia scrivere sulla pipe i miei dati alla ricezione del segnale utente1

    printf("\nHub creato: id: %d\n", id);
    printf("Id: %d\n", id);
    printf("Pid: %d\nPid padre: %d\n", pid, idPar);
    printf("Nome: %s\n", nome);
    int i=0;

    //AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

    //PROVA

   //char* info = malloc(ANSWER);

   if(id < 9){
       info infoD;
       infoD.def = 1;
       infoD.id = informazioni.id+1;
       //sprintf(info, "%d", id+1);
       add_device_generale("./binaries/HUB", dispList, infoD, "ProvaName");
       //memset(info,0,strlen(info));
       //sprintf(info, "default %d", id+1);
       add_device_generale("./binaries/BULB", dispList, infoD, "ProvaName");
       //memset(info,0,strlen(info));


       //add_device_generale("./binaries/WINDOW", dispList, info, "ProvaName");
       //add_device_generale("./binaries/FRIDGE", dispList, info, "ProvaName");
       
       /*
       sprintf(info, "%default ", id+1);
       add_device_generale("./componenti/BULB", dispList, info, "perenzoni gay");
       sprintf(info, "%default ", id+1);
       add_device_generale("./componenti/BULB", dispList, info, "perenzoni gay");
       */
    }
    
    //Invio segnale al padre
    int ris = kill(idPar, SIGCONT);

    //Child va in pausa
    while(1){
        pause();
    }

    printf("Child ora termina\n");   
    exit(0);
}
//i vari parametri potrebbero essere levati se messo in hub TODO, serve anche al timer però
int broadcast_controllo(NodoPtr list, cmd comando, int pid_papi, int fd_papi, risp risposta_to_padre){
    signal(SIGCONT, sign_cont_handler_hub);//Segnale per riprendere il controllo 
    signal(SIGUSR1, sighandle_usr1);
    
    //nodo rappresenta il figlio, nell'hub passo il successivo dato che il primo nodo 
    //è sè stesso
    NodoPtr nodo = list->next;
    //risposta che verrà mandata al padre (se ho figli)
    risp answer;
    int err_signal;//errore kill
    //scrivo al padre la risposta del dispositivo di controllo contenente le sue info
    //il padre potrebbe essere un dispositivo diverso dalla centralina ma comunque sarà in ascolto
    write(fd_papi, &risposta_to_padre, sizeof(risp));
    //finchè ho figli
    while(nodo != NULL){
        //Mando il comando a mio figlio che lo gestirà
        write(nodo->fd_writer, &comando, sizeof(comando));
        err_signal = kill(nodo->data, SIGUSR1); 
        int pid_figlio = nodo->data;
        //Mando un segnale per comunicare a mio figlio di gestire il comando
        if(err_signal != 0)
            perror("errore in invio segnale");
        while(1){
            //Leggo la risposta --> viene letta dopo che mi è arrivato un segnale SIGCONT
            //dato che è bloccante
            read(nodo->fd_reader, &answer, sizeof(risp));
                //debug printf("Leggo la risposta in %d, %s\n", id, answer.info);
            //se è un messaggio di terminazione devo uscire dal ciclo di ascolto e andare 
            //al nodo successivo
            if(answer.termina_comunicazione == 1){
                break;
            }else{
                
                //Il delete non funziona se fatto non al primo nodo o con delete --all OIBO
                //se non è un messaggio di terminazione significa che il figlio ha ancora risp da comunicare
                //nel caso dei dispositivi di interazione (o controllo senza figli) verrà mandato
                //1 messaggio contenente le informazioni e un successivo messaggio di terminazione
                if(answer.eliminato == 1){//questo vale quando risalgo, se il dispositivo è da eliminare lo tolgo dalla lista dei processi
                    //printList(list);
                    removeNode(list, answer.pid);
                    answer.eliminato = 0;//setto a 0 sennò lo toglie anche il padre che non lo ha nella lista 
                }
                //finchè tutti i figli non avranno mandato il messaggio di terminazione
                //continuerà a mandare risposte in su nell'albero verso la centralina, quando tutti avranno mandato il messaggio di terminazione
                //egli manderà 1 messaggio di terminazione al padre
                //scrivo a mio padre la risposta che ho appena letto
                write(fd_papi, &answer, sizeof(risp));
                
                //mando un segnale al figlio per comunicare di continuare la comunicazione                
                err_signal = kill(pid_figlio, SIGCONT);

                if(err_signal != 0)
                    perror("errore in invio segnale");
                //messaggio di terminazione o
                //ulteriore risposta con terminazione = 0 
                
            }
        }
        nodo = nodo->next;
    }
    //comunico al padre di aver finito di comunicare mettendo il parametro termina_comunicazione = 1
    risposta_to_padre.termina_comunicazione = 1;
    write(fd_papi, &risposta_to_padre,sizeof(risp));
    return 1;
}
