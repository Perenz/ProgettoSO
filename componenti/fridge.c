#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <sched.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "../strutture/listH.h"
#include "../strutture/comandiH.h"


//ho spostato i metodi getLine e splitLine in una nuova libreria 
//TODO tale verrà linkata nel gestore generale dei processi di interazione
//void get_info_string(info*);
int device_handle_command(cmd);


int dev_getinfo(cmd);
int dev_delete(cmd);
int dev_changestate(cmd);
int dev_manualControl(cmd);
int dev_switch(cmd);
int dev_list(cmd);
int dev_info(cmd);
int dev_set(cmd);


void set_time();

void sign_cont_handler(int);

#include "../Include/funzioniDispositiviInterazione.c"


time_t tempoUltimaMisurazione;
//TODO : potrei usare un unico node
//File descriptor in cui il figlio legge e il padre scrive
int fd_read;
//File descriptor in cui il figlio scrive e il padre legge
int fd_write;

int fifoCreata=0;
info informazioni;




char *builtin_command[]={
    "l",//list
    "s",//switch
    "i",//getInfo
    "d", //delete
    "m",//Manual
    "p"//set

};
int (*builtin_func[]) (cmd comando) = { //int man: 0 allora il comando arriva da centralina, 1 il comando arriva da manuale
    &dev_list,
    &dev_switch,
    &dev_info,
    &dev_delete,
    &dev_manualControl,
    &dev_set
};
int dev_numCommands(){
    return (sizeof(builtin_command)/ sizeof(char*));
}
int device_handle_command(cmd comando){
    //da fare come in functionDeclarations in file dispositivi
    //NON FUNZICA
    int i;
    for(i=0; i<dev_numCommands(); i++){
        char tmp = *builtin_command[i];
        if(comando.tipo_comando == tmp){
            set_time();
            return builtin_func[i](comando);
        }
    }
    //comando non riconosciuto
    return -1;
}
void signhandle_quit(int sig){
    char fifo[30];
    if(sig==SIGQUIT){
        if(fifoCreata!=0){
            sprintf(fifo, "/tmp/fifoManComp%d", informazioni.pid);
            remove(fifo);
        }
        _exit(0);
    }
}
void sighandle_alarm(int sig){
    if(sig == SIGALRM){
        set_time();
        strcpy(informazioni.stato, "chiuso");
    }
}
void sighandle_usr1(int sig){
    sighandle1(sig, fd_read, fd_write);
}
void sighandle_usr2(int sig){
    sighandle2(sig);
}
void sign_cont_handler(int sig){
    return;
}

void signint_handler(int sig){
    //Segnale int inviato da comando power
    //Vado in pausa
    //Per il bulb è inutile andare in pause perchè tanto c'è while(1) pause;
}
//COMANDO   l
/*restituisce in pipe
    se comando è l: <informazioni>
*/
int dev_list(cmd comando){
    int err = dev_list_gen(comando, informazioni.pid_padre, fd_write, informazioni);
    return err;
}

//COMANDO switch <id> <label> <stato:on/off>
/*restituisco in pipe:
    0 se NON sono il dispositivo in cui ho modificato lo stato
    1 se sono il dispositivo in cui ho modificato lo stato
*/
//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
int dev_switch(cmd comando){
    risp answer;
    if(comando.id == informazioni.id || comando.forzato == 1){
        if(strcmp(comando.cmdInterruttore.nome , "apertura")==0){
            //get_info_string(&(answer.info_disp));
            if(strcmp(informazioni.stato,"chiuso")== 0 && strcmp(comando.cmdInterruttore.stato , "on")==0){
                strcpy(informazioni.stato, "aperto"); 
                alarm(informazioni.frigo.delay); 
            }else if(strcmp(informazioni.stato,"aperto")== 0 && strcmp(comando.cmdInterruttore.stato , "off")==0){
                strcpy(informazioni.stato, "chiuso");  
            }
            //get_info_string(&(answer.info_disp));
        }else if(strcmp(comando.cmdInterruttore.nome , "termostato")==0){
            informazioni.frigo.temperatura = atoi(comando.cmdInterruttore.stato);
        }
        if(comando.manuale==1){
            //Devo rispondere al manuale
            //fd_manuale
            //devo aprire la fifo prima di rispondere
            char fifoManComp[30], msg[10];
            
            sprintf(fifoManComp, "/tmp/fifoManComp%d", getpid());
            //Apro Fifo in scrittura
            int fd_manuale = open(fifoManComp, O_WRONLY);

            //////////////////////////////////////////////////////////
            (strcmp(comando.cmdInterruttore.nome , "apertura")==0) ? sprintf(msg, "%s", informazioni.stato) : sprintf(msg, "%d", informazioni.frigo.temperatura);//Rispondo solamente con lo status attuale del dispositivo
            int esito=write(fd_manuale, msg, 10);
            /////////////////////////////////////////////////////////

            //Chiudo in scrittura
            close(fd_manuale);
            return 1;
        }
        answer.considera = 1;
    }else
    {
        answer.considera = 0;
    }
    answer.info_disp = informazioni;
    rispondi(answer, comando, fd_write);
    return 1;
}

int dev_set(cmd comando){
    risp answer;
    char fifoManComp[30], msg[10];
    if(comando.id==informazioni.id){
        if(strcmp(comando.cmdInterruttore.nome, "delay")==0){
            informazioni.frigo.delay = atoi(comando.cmdInterruttore.stato);
        }
        else if(strcmp(comando.cmdInterruttore.nome, "perc")==0){
            informazioni.frigo.percentuale = atoi(comando.cmdInterruttore.stato);
        }
        answer.considera=1;

        if(comando.manuale==1){
            //Devo rispondere al manuale
            //fd_manuale
            //devo aprire la fifo prima di rispondere
            char fifoManComp[30], msg[10];
            
            sprintf(fifoManComp, "/tmp/fifoManComp%d", getpid());
            //Apro Fifo in scrittura
            int fd_manuale = open(fifoManComp, O_WRONLY);

            //////////////////////////////////////////////////////////
            sprintf(msg, "%s", comando.cmdInterruttore.stato);//Rispondo solamente con lo status attuale del dispositivo
            int esito=write(fd_manuale, msg, 10);
            /////////////////////////////////////////////////////////

            //Chiudo in scrittura
            close(fd_manuale);

            return 1;
        } 
    }else{
        answer.considera = 0;
    }
    
    rispondi(answer, comando, fd_write);

    return 1; 
}

//COMANDO   info <id>
/*restituisce in pipe
    <info> := <tipo> <pid???> <id> <status> <time>
*/
int dev_info(cmd comando){
    int err = dev_info_gen(comando, informazioni.id, informazioni.pid_padre, fd_write, informazioni.pid, informazioni);
    return err;
}
//COMANDO d <pid>
/*restituisco in pipe:
    0 se NON sono il dispositivo da eliminare
    pid se sono il dispositivo da eliminare
*/
int dev_delete(cmd comando){
    int err = dev_delete_gen(comando, informazioni.pid, informazioni.id, informazioni.pid_padre, fd_write, informazioni);
    return err;
}


void set_time(){
    if(strcmp(informazioni.stato,"aperto")== 0){
        time_t tmp;
        time(&tmp);
        informazioni.time += (difftime(tmp, tempoUltimaMisurazione));
        //la lampadina è accesa
        tempoUltimaMisurazione = tmp;
    }else{//utilizzato per gestire il caso di cambio di stato
        time(&tempoUltimaMisurazione);
    }
}

int dev_manualControl(cmd comando){
    fifoCreata=1;
    int err = dev_manual_info_gen(comando, informazioni.id, informazioni.pid_padre, fd_write, informazioni.pid, informazioni);
    return err;
}


int main(int argc, char *args[]){
    //leggo args per prendere gli argomenti passati(puntatore al lato di scrittura della pipe)
    fd_read = atoi(args[1]);
    fd_write = atoi(args[2]);  
    int err = read(fd_read, &informazioni,sizeof(info));
    if(err == -1)
        printf("eerore nella lettura delle info BULB\n");
    
    time(&tempoUltimaMisurazione);
    if(informazioni.def == 1){
        informazioni.frigo.delay = 10.0; //di default 5 secondi
        informazioni.frigo.temperatura = 3; //di default 3 gradi
        informazioni.frigo.percentuale = 0; //di default frigo vuoto
        strcpy(informazioni.tipo, "fridge");
        strcpy(informazioni.stato, "chiuso");
        informazioni.time = 0.0;
    }
    if(strcmp(informazioni.stato,"aperto")==0){
        alarm((informazioni.frigo.delay - ((int)informazioni.time % (int)informazioni.frigo.delay)));
    }
    informazioni.pid = getpid(); // chiedo il mio pid
    informazioni.pid_padre = getppid(); //chiedo il pid di mio padre
    

    signal(SIGALRM, sighandle_alarm);
    signal(SIGQUIT, signhandle_quit);
    signal(SIGUSR1, sighandle_usr1); //imposto un gestore custom che faccia scrivere sulla pipe i miei dati alla ricezione del segnale utente1
    signal(SIGUSR2, sighandle_usr2); //Alla ricezione di SIGUSR2 leggere il comanda sulla fifo direttamente connessa al manuale
    signal(SIGCONT, sign_cont_handler);//Segnale per riprendere il controllo 

    if(informazioni.def == 1){
        printf("\nFrigo posto in magazzino\n");
        printf("Id: %d\n", informazioni.id);
        printf("Nome: %s\n", informazioni.nome);
        printf("Pid: %d\nPid padre: %d\n\n", informazioni.pid, informazioni.pid_padre);
    }else{
        printf("\nFrigo collegata\n");
        printf("Id: %d\n", informazioni.id);
        printf("Nome: %s\n", informazioni.nome);
        printf("Pid: %d\nPid padre: %d\n\n", informazioni.pid, informazioni.pid_padre);
    }
    informazioni.def = 0; 

    //Invio segnale al padre
    int ris = kill(informazioni.pid_padre, SIGCONT); 

    //Child va in pausa
    
    
    while(1){
        pause();
    }

    //printf("Child ora termina\n");   
    exit(0);
}




