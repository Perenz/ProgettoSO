#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

int hand_control(char **, int*); //Dovra ritornare il pid del dispositivo specificato come argomento
int hand_help(char **, int*);
int hand_exit(char **, int*);
int hand_release(char **, int*);

char *noControl_builtin_cmd[]={
        "control", //Prende in input un parametro, il nome/id del dispositivo su cuiu vogliamo agire
        "help",
        "exit"
};

//Pointers list to a Function associated to each command
int (*noControl_builtin_func[]) (char **, int*) = {
        &hand_control,
        &hand_help,
        &hand_exit
};

char *control_builtin_cmd[]={
    "exit",
    "release"
};

int (*control_builtin_func[]) (char **, int*) = {
    &hand_exit,
    &hand_release
};



int cen_numCommands(int type){
    if(type==0)
        return (sizeof(noControl_builtin_cmd)/ sizeof(char*));
    else{
        return (sizeof(control_builtin_cmd)/ sizeof(char*));
    }
}

int hand_control(char **args, int *cenPid){
    int fd;
    if(args[1]==NULL || args[2]!=NULL){
            printf("Errore nei parametri\n");
            printf("Usage: control <pid/nome>\n");
            

            return -1;
    }
    else{
            //Tramite comando info????
            printf("Chiedo alla centralina il pid del dispositivo richiesto\n");

            //Mi serve una fifo tra manuale.c e la centralina
            char *manCenFifo = "/tmp/manCenFifo";
            mkfifo(manCenFifo, 0666);
            //Mando signal a centralina per aprire la fifo in READ_ONLY
            kill(*cenPid, SIGUSR2);
            //Apro la fifo in WRITEONLY
            fd = open(manCenFifo, O_WRONLY);
            char msg[20];
            strcat(msg, "contpid ");
            strcat(msg, args[1]); //args1 conterrà l'id del dispositivo che si vuole controllare
            printf("Invio il messaggio\n%s\n", msg);
            int esito = write(fd, msg,strlen(msg)+1);
            close(fd); //Chiudo in scrittura
            //TODO
            printf("Scritto con esito %d\n", esito);
            //Per ora ritorno -1;
            //Ascolto per la risposta
            //Apro pipe in lettura
            fd = open(manCenFifo, O_RDONLY);
            read(fd,msg,20);
            close(fd);
            int pidCerc=atoi(msg);
            printf("Pid ricevuto %d\n", pidCerc);

            //Ora mi metto un lettura
            return pidCerc;
    }
}

int hand_help(char **args, int *cenPid){     
    printf("Progetto SO realizzato da: Paolo Tasin, "
           "Stefano Perenzoni, Marcello Rigotti\n");
    printf("Centralina per controllo domotico\n");
    printf("Comandi accettati se non si controlla alcun dispositivo:\n");

    int i=0;
    for(i; i<cen_numCommands(0);i++){
        printf("> %s\n", noControl_builtin_cmd[i]);
    }

    printf("Comandi accettati mentre si controlla qualche dispositivo:\n");
    i=0;
    for(i; i<cen_numCommands(1);i++){
        printf("> %s\n", control_builtin_cmd[i]);
    }

    return -1;
}

int hand_exit(char **args, int *cenPid){
        return 0;
}

int hand_release(char **args, int *cont){
    *cont=0;
    return 1;
}