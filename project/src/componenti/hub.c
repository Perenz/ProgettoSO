#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include "../strutture/listH.h"
#include "../strutture/list.c"

#include "../include/gestioneComandi.c"
#include "../include/addDevice.c"
//#include "../includefunzioniDispositiviControllo.c"

//voglio usare le funzioni definite in functionDeclaration urca
#define CEN_BUFSIZE 128

int add = 0;
info info_device_to_add;


NodoPtr dispList; //lista dei dispositivi collegati all'hub
int fifoCreata=0;
//File descriptor in cui il figlio legge e il padre scrive
int fd_read;
//File descriptor in cui il figlio scrive e il padre legge
int fd_write;
info informazioni;
int sigEntrata=0;





int dev_list(cmd);
int dev_switch(cmd);
int dev_info(cmd);
int dev_delete(cmd);
int dev_link(cmd);
int dev_manualControl(cmd);
int dev_set(cmd);
int dev_depth_info(cmd comando, risp* risposta);
//int dev_link(char** args);
int device_handle_command(cmd);
void h_sigstop_handler ( int sig ) ;
//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
int dev_add(char*, char*);



void signhandle_quit(int sig){
    char fifo[30];
    if(sig==SIGQUIT){
        if(fifoCreata!=0){
            sprintf(fifo, "/tmp/fifoManComp%d", informazioni.pid);
            remove(fifo);
        }
        //Devo mandare il SIGQUIT a tutti i suoi figli
        NodoPtr nodo = dispList;
        while(nodo!=NULL){
            kill(SIGQUIT, nodo->data);
            nodo=nodo->next;
        }

        _exit(0);
    }
}

char *builtin_command[]={
    "l",//list
    "s",//switch
    "i",//getInfo
    "d", //delete
    "a", //addDevice / link   da cambiare scegliendo lettera corrispondente
    "m", //manualControl
    "p" //set
};
int (*builtin_func_hub[]) (cmd comando) = {
        &dev_list,
        &dev_switch,
        &dev_info,
        &dev_delete,
        &dev_link,
        &dev_manualControl,
        &dev_set
};
int cont_numCommands(){
    return (sizeof(builtin_command)/ sizeof(char*));
}

void sign_cont_handler_hub(int sig){
    signal(SIGCONT, sign_cont_handler_hub);
    return;

}

void sigint_handler(int sig){
    char fifo[30];
    if(fifoCreata!=0){
            sprintf(fifo, "/tmp/fifoManComp%d", informazioni.pid);
            remove(fifo);
    }
    //Devo mandare il SIGINT a tutti i suoi figli
    NodoPtr nodo = dispList;
    while(nodo!=NULL){
        kill(SIGINT, nodo->data);
        nodo=nodo->next;
    }
    return;
    //Come per bulb non serve andare in pausa
}

void sighandle2(int sig){
    if(sig == SIGUSR2){
        char fifoManComp[30];
        sprintf(fifoManComp, "/tmp/fifoManComp%d", getpid());
        int fd_manuale = open(fifoManComp, O_RDONLY);
        cmd comando;
        read(fd_manuale, &comando, sizeof(cmd));//uso 10 per intanto, vedi sopra poi
        close(fd_manuale);
        comando.manuale=1;
        int errnum = device_handle_command(comando);
    }
}

void sighandle1(int sig, int fd_read, int fd_write){
    if(sig == SIGUSR1){
        cmd comando;
        int err_signal;
        read(fd_read, &comando, sizeof(cmd));
        int errnum = device_handle_command(comando);
        err_signal = kill(informazioni.pid_padre, SIGCONT);
        if(err_signal != 0)
            perror("errore in invio segnale");

        //printf("Termina in modo adeguato.\n");
        return;
    }

}

//SIGUSR1 usato per l'implementazione della lettura della pipe con il padre
void sighandle_usr1_hub(int sig){
    sighandle1(sig, fd_read, fd_write);
}

//USATO PER SVEGLIARE IL PROCESSO
void sighandle_usr2(int sig){
    sigEntrata=2;
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
    //se il comando è diverso da i non restituisco i tempi massimi 
    if(comando.tipo_comando != 'i'){
        informazioni.lampadina.maxTime = -1.0;
        informazioni.frigo.maxTime = -1.0;
        informazioni.finestra.maxTime = -1.0;

        risposta_controllore.info_disp.lampadina.maxTime = -1.0;
        risposta_controllore.info_disp.frigo.maxTime = -1.0;
        risposta_controllore.info_disp.finestra.maxTime = -1.0;

    }
    if(comando.manuale != 1){
        risposta_controllore.id_padre = comando.id_padre;
        risposta_controllore.termina_comunicazione = 0;
        risposta_controllore.pid = informazioni.pid;
    }

    //vado io in controllo e mando le varie risposte al papi
    //attenz, buono che salto il primo
    broadcast_controllo(dispList, comando, informazioni, fd_write, risposta_controllore);
    return 1;
}

int dev_manual_info_gen(cmd comando, int id, int idPar, int fd_write, int pid, info informazioni){
  risp answer;
    if(id == comando.id){//comando forzato per avere le info di dispositivi situati nel sott'albero di un processo che ha id 

        answer.pid = pid;
        answer.considera = 1;
        answer.id = id;
        answer.dispositivo_interazione = 1;
        answer.info_disp.def = 0;
        answer.info_disp = informazioni;
        //get_info_string(&(answer.info_disp));

        //Devo creare la fifo per il collegamento diretto
        char fifoManComp[30];
        
        sprintf(fifoManComp, "/tmp/fifoManComp%d", getpid());
        mkfifo(fifoManComp, 0666);
    
    }else{
        answer.considera = 0;
        answer.dispositivo_interazione = 1;
    }
    rispondi(answer, comando);
    
    //famo ritornare l'errore poi
    return 1;
}

int dev_list(cmd comando){
    risp risposta_controllore;


    risposta_controllore.id = informazioni.id;
    risposta_controllore.pid = informazioni.pid;
    risposta_controllore.considera = 1;
    risposta_controllore.eliminato = 0;

    //Metto a -1 i max time così che non vengano stampati con il comando list, con il comando info verranno trovati e stampati
    
    //set_info
    risposta_controllore.info_disp = informazioni;
    rispondi(risposta_controllore, comando);
    return 1;
}
int dev_switch(cmd comando){//////DA MODIFICARE
    //puoi richiamare la funzione che c'è sopra bro, guarda, per il resto non dovrebbe variare nulla
    risp risposta_controllore;
    if(comando.id == informazioni.id || comando.forzato == 1){
        comando.forzato = 1;
        risposta_controllore.id = informazioni.id;
        risposta_controllore.considera = 1;
        
        risposta_controllore.pid = informazioni.pid;
        risposta_controllore.profondita = comando.profondita+1;

        if(strcmp(comando.cmdInterruttore.nome , "accensione")==0){//si tratta di un comando per una bulb 
            //get_info_string(&(answer.info_disp));
            if(strcmp(informazioni.lampadina.accensione.stato,"off")== 0 && strcmp(comando.cmdInterruttore.stato , "on")==0){
                strcpy(informazioni.lampadina.accensione.stato, "on");  
            }else if(strcmp(informazioni.lampadina.accensione.stato,"on")== 0 && strcmp(comando.cmdInterruttore.stato , "off")==0){
                strcpy(informazioni.lampadina.accensione.stato, "off");  
            }
        }
        
        
        else if(strcmp(comando.cmdInterruttore.nome , "apertura")==0 || strcmp(comando.cmdInterruttore.nome , "aperturaF")==0){//si tratta di un frigo
            //get_info_string(&(answer.info_disp));
            if(strcmp(informazioni.frigo.apertura.stato,"chiuso")== 0 && strcmp(comando.cmdInterruttore.stato , "on")==0){
                strcpy(informazioni.frigo.apertura.stato, "aperto"); 
                alarm(informazioni.frigo.delay); 
            }else if(strcmp(informazioni.frigo.apertura.stato,"aperto")== 0 && strcmp(comando.cmdInterruttore.stato , "off")==0){
                strcpy(informazioni.frigo.apertura.stato, "chiuso");  
            }
            //get_info_string(&(answer.info_disp));
        }

        if(strcmp(comando.cmdInterruttore.nome , "apertura")==0 || strcmp(comando.cmdInterruttore.nome , "aperturaW")==0){
            //get_info_string(&(answer.info_disp));
            if(strcmp(informazioni.finestra.apertura.stato,"chiusa")== 0 && strcmp(comando.cmdInterruttore.stato , "on")==0){
                strcpy(informazioni.finestra.apertura.stato, "aperta");  
               
            }
        }else if(strcmp(comando.cmdInterruttore.nome, "chiusura")==0 || strcmp(comando.cmdInterruttore.nome , "chiusuraW")==0){
            if(strcmp(informazioni.finestra.chiusura.stato,"aperta")== 0 && strcmp(comando.cmdInterruttore.stato , "on")==0){
                strcpy(informazioni.finestra.chiusura.stato, "chiusa");  
            }
            //get_info_string(&(answer.info_disp));
        }

        risposta_controllore.considera = 1;    
        
        risposta_controllore.info_disp = informazioni;
        if(comando.manuale==1){
            //comando.manuale=0;
            //rispondi(risposta_controllore, comando);

            //Devo rispondere al manuale
            //fd_manuale
            //devo aprire la fifo prima di rispondere
            char fifoManComp[30], msg[10];
            
            sprintf(fifoManComp, "/tmp/fifoManComp%d", getpid());
            //Apro Fifo in scrittura
            int fd_manuale = open(fifoManComp, O_WRONLY);

            sprintf(msg, "%s", comando.cmdInterruttore.stato);//Rispondo solamente con lo status attuale del dispositivo
            int esito=write(fd_manuale, msg, 10);

            //Chiudo in scrittura
            close(fd_manuale);
        }

    }else{
        risposta_controllore.considera = 0;
        risposta_controllore.id = informazioni.id;
    }
    
    rispondi(risposta_controllore, comando);  
    return 1;
}

int dev_set(cmd comando){
    risp risposta_controllore;
    //Il set arriverà per forza dal manuale
    //Faccio comunque il controllo in caso di modifiche future
    if(comando.manuale==1){
        comando.forzato = 1;
        risposta_controllore.id = informazioni.id;
        risposta_controllore.considera = 1;
        risposta_controllore.info_disp = informazioni;

        rispondi(risposta_controllore, comando);  

        //Devo rispondere al manuale
        //fd_manuale
        //devo aprire la fifo prima di rispondere
        char fifoManComp[30], msg[10];
            
        sprintf(fifoManComp, "/tmp/fifoManComp%d", getpid());
        //Apro Fifo in scrittura
        int fd_manuale = open(fifoManComp, O_WRONLY);

        sprintf(msg, "%s", comando.cmdInterruttore.stato);//Rispondo solamente con lo status attuale del dispositivo
        int esito=write(fd_manuale, msg, 10);

        //Chiudo in scrittura
        close(fd_manuale);
      
        return 1;
    }
}


int dev_info(cmd comando){
    risp risposta_controllore;
    if(comando.id == informazioni.id || comando.forzato == 1){//comando --all , forzato forza l'invio delle info anche se l'id non è uguale
            //il parametro info_forzate è usato nel link: forza i dispositivi nell'invio delle proprie informazioni 
            //se figli di un hub con tali informazioni 
        if(comando.info_forzate == 1){
            comando.forzato = 1;
        }
        risposta_controllore.info_disp.def = 0;
        risposta_controllore.id = informazioni.id;
        risposta_controllore.considera = 1;
        risposta_controllore.pid = informazioni.pid;
        risposta_controllore.dispositivo_interazione = 0;
        dev_depth_info(comando, &risposta_controllore);
        risposta_controllore.info_disp = informazioni;
        //set_info

        //SE VOGLIAMO FARE CHE IL DISPOSITIVO MANDA UN MESSAGGIO E NON CERCA SE I SUOI FIGLI HANNO LO STESSO ID: 
        /* NON VA SE NON è STATO FATTO UN LIST PRIMA ED IN ALCUNI CASI SI BLOCCA
        risposta_controllore.termina_comunicazione = 0;
        write(fd_write, &risposta_controllore, sizeof(risp));
        risposta_controllore.termina_comunicazione = 1;
        write(fd_write, &risposta_controllore, sizeof(risposta_controllore));
        */
        //Se VOGLIAMO FARE CHE IL DISPOSITIVO CHIEDE AI SUOI FIGLI SE C'è QUALCUNO CON QUELL'ID ANCHE SE LUI HA GIà QUELL'ID
        rispondi(risposta_controllore, comando);
    }else{
        risposta_controllore.considera = 0;//non considerarmi
        rispondi(risposta_controllore, comando);
    }
    return 1;
}

int dev_depth_info(cmd comando, risp* risposta){
    risp* array_risposte_figli;
    malloc_array(&array_risposte_figli, N_MAX_DISP);
    comando.forzato = 1;
    int n = broadcast_centralina(dispList, comando, array_risposte_figli);
    int i;

    risposta->errore = 0;
    for(i=0; i<n; i++){
        if( strcmp ( array_risposte_figli[i].info_disp.tipo , "bulb" ) == 0){
            //C'è override 
            if(strcmp(array_risposte_figli[i].info_disp.stato , informazioni.lampadina.accensione.stato) != 0){
                //override errore = 3 bulb 
                informazioni.lampadina.override_hub = '1';
            }else{
                informazioni.lampadina.override_hub = '0';
            }
            /////VERIFICO CHE INTERRUTTORE NON SIA A 0, LI METTO A 0 NEL MAIN RICORDA
            /////variabile settata a 1 se esiste info o tempo messo a -1
            ////confronto con me, se diverso metto override = 1
            if(informazioni.lampadina.maxTime < array_risposte_figli[i].info_disp.time)
                informazioni.lampadina.maxTime = array_risposte_figli[i].info_disp.time;
            
        }else if( strcmp ( array_risposte_figli[i].info_disp.tipo , "fridge" ) == 0){
            if(strcmp(array_risposte_figli[i].info_disp.stato , informazioni.frigo.apertura.stato) != 0){
                //override errore = 4
               informazioni.frigo.override_hub = '1';
            }else{
                informazioni.frigo.override_hub = '0';
            }

            /////VERIFICO CHE INTERRUTTORE NON SIA A 0, LI METTO A 0 NEL MAIN RICORDA
            if(informazioni.frigo.maxTime < array_risposte_figli[i].info_disp.time)
                informazioni.frigo.maxTime = array_risposte_figli[i].info_disp.time;

        }else if( strcmp ( array_risposte_figli[i].info_disp.tipo , "window" ) == 0){
            if(strcmp(array_risposte_figli[i].info_disp.stato , informazioni.finestra.apertura.stato) != 0){
                //override errore = 3
                informazioni.finestra.override_hub = '1';
            }else{
                informazioni.finestra.override_hub = '0';
            }
            /////VERIFICO CHE INTERRUTTORE NON SIA A 0, LI METTO A 0 NEL MAIN RICORDA
            if(informazioni.finestra.maxTime < array_risposte_figli[i].info_disp.time)
                informazioni.finestra.maxTime = array_risposte_figli[i].info_disp.time;
        }
    }
    

    free(array_risposte_figli);
    return 1;
}



int dev_delete(cmd comando){
    risp risposta_controllore;
    if(comando.forzato == 1 || comando.id == informazioni.id){//comando --all 
        risposta_controllore.id = informazioni.id;
        risposta_controllore.errore = 0;
        risposta_controllore.considera = 1;
        risposta_controllore.eliminato = 1;
        risposta_controllore.pid = informazioni.pid;
        comando.forzato = 1;//indico ai miei figli di eliminarsi
        //set_info 
        risposta_controllore.info_disp = informazioni;
        
        rispondi(risposta_controllore, comando);
    
        exit(0);
    }else{
        risposta_controllore.considera = 0;//non considerarmi, non sono stato eliminato
        risposta_controllore.eliminato = 0;
        risposta_controllore.info_disp = informazioni;
        rispondi(risposta_controllore, comando);
    }
    return 1;
}
int dev_link(cmd comando){
    risp risposta_controllore;
    if(comando.id == informazioni.id){    
        int i, err;
        risposta_controllore.considera = 0;
        risposta_controllore.eliminato = 0;
        info_device_to_add = comando.info_disp;
        add = 1;
        risposta_controllore.errore = 0;
        risposta_controllore.termina_comunicazione = 0;

        write(fd_write, &risposta_controllore, sizeof(risp));

        

        //La continuazione della risposta si trova nel main
    }else{
        risposta_controllore.considera = 0;
        risposta_controllore.eliminato = 0;
        rispondi(risposta_controllore, comando);
    }
    return 1;
}

int dev_manualControl(cmd comando){
    fifoCreata=1;
    int err = dev_manual_info_gen(comando, informazioni.id, informazioni.pid_padre, fd_write, informazioni.pid, informazioni);
    return err;
}



int main(int argc, char **args){
    dispList = listInit(getpid());

    //UGUALE A BULB 

    fd_read = atoi(args[1]);
    fd_write = atoi(args[2]);
    //MANCA IL SET_INFO, sbaglia l'id
    int err = read(fd_read, &informazioni,sizeof(info));
    
    informazioni.pid = getpid(); // chiedo il mio pid
    
    informazioni.pid_padre = getppid(); //chiedo il pid di mio padre
    if(err == -1)
        printf("Errore nella lettura delle info date dal padre\n");
    

    if(informazioni.def == 1){
        strcpy(informazioni.stato, "off");



        strcpy(informazioni.tipo, "hub");
        informazioni.time = 0.0;
        informazioni.lampadina.maxTime = -1.0;
        informazioni.finestra.maxTime = -1.0;
        informazioni.frigo.maxTime = -1.0;
        strcpy(informazioni.lampadina.accensione.stato , "off");
        strcpy(informazioni.frigo.apertura.stato , "chiuso");
        strcpy(informazioni.finestra.apertura.stato , "chiusa");
        strcpy(informazioni.finestra.chiusura.stato , "aperta");
    }
    sigEntrata=0;

    signal(SIGINT, sigint_handler);
    signal(SIGCONT, sign_cont_handler_hub);//Segnale per riprendere il controllo 
    signal(SIGQUIT, signhandle_quit);
    signal(SIGUSR1, sighandle_usr1_hub); //imposto un gestore custom che faccia scrivere sulla pipe i miei dati alla ricezione del segnale utente1
    signal(SIGUSR2, sighandle_usr2);

    if(informazioni.def == 1){
        printf("\nHub posto in magazzino \n");
    }else{
        printf("\nHub collegato\n");
    }
    printf("Id: %d\n", informazioni.id);
    printf("Nome: %s\n", informazioni.nome);
    printf("Pid: %d\nPid padre: %d\n\n", informazioni.pid, informazioni.pid_padre);
    informazioni.def = 0;

    //Invio segnale al padre
    int ris = kill(informazioni.pid_padre, SIGCONT);

    //Child va in pausa
    while(1){
        if(add == 1){
            int i;
            add = 0;
            risp risposta_controllore;
            for(i=0; i<device_number(); i++){
            if(strcmp(info_device_to_add.tipo, builtin_device[i])==0)
                add_device_generale(builtin_dev_path[i], dispList, info_device_to_add, NULL);//ho invertito proc e disp
            }
            risposta_controllore.termina_comunicazione = 1;
            write(fd_write, &risposta_controllore, sizeof(risp));
        }

        if(sigEntrata==2){
            sighandle2(SIGUSR2);
        }
        sigEntrata = 0;
        pause();
    }

    printf("Child ora termina\n");   
    exit(0);
}
