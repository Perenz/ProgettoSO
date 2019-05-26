#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include "../strutture/comandiH.h"

int hand_control(char **, int *, char *); //Dovra ritornare il pid del dispositivo specificato come argomento
int hand_help(char **, int *, char *); //Indica i diversi comandi disponibili
int hand_exit(char **, int *, char *); //Per eseguire un'uscita sicura dal sistema
int hand_exit1(char **, int *, int, char); //Richiama hand_exit per eseguire uscita sicura
int hand_release(char **, int *, int, char); //Rilascia il dispositivo controllato 
int hand_switch(char **, int *, int, char); //Esegue switch sul comando controllato 
int hand_set(char **, int *, int, char); //Esegue set sul comando controllato

void inizializzaFifo(int);

//fifo per il collegamento DIRETTO con il dispositivo che si sta controllando
int fdManual;
char fifoManDisp[30];

char *noControl_builtin_cmd[] = {
    "control", //Prende in input un parametro, il nome/id del dispositivo su cuiu vogliamo agire
    "help",
    "exit"
};

//Pointers list to a Function associated to each command
int (*noControl_builtin_func[])(char **, int *, char *) = {
    &hand_control,
    &hand_help,
    &hand_exit};

char *control_builtin_cmd[] = {
    "exit",
    "release",
    "switch",
    "set" //Per settare: delay, perc e temp del frigorifero
};

int (*control_builtin_func[])(char **, int *, int, char) = {
    &hand_exit1,
    &hand_release,
    &hand_switch,
    &hand_set};

void inizializzaFifo(int pidCont)
{
    //file gia creato dal dispositivo
    int fdManDisp;
    sprintf(fifoManDisp, "/tmp/fifoManComp%d", pidCont);
}

void stampaOutput(char **args, int *cont, int idCont, char tipoCont, char* msg){
    if(tipoCont=='h' || tipoCont=='t'){
        //Se sto controllando un HUB devo controllare quale su quale interruttore sto lavorando per decidere l'output
        if((strcmp(args[1], "apertura")==0 || strcmp(args[1], "chiusura")==0)){
            printf("L'hub/timer (pid %d, id %d, tipo %c), i frigoriferi e le finestre controllati risultano ora %s\n", *cont, idCont, tipoCont, (strcmp(msg, "on")==0 ? "aperti" : "chiusi"));       
        }
        else if((strcmp(args[1], "aperturaW")==0 || strcmp(args[1], "chiusuraW")==0)){
            printf("L'hub/timer (pid %d, id %d, tipo %c) e le finestre controllate risultano ora %s\n", *cont, idCont, tipoCont, (strcmp(msg, "on")==0 ? "aperte" : "chiuse"));       

        }
        else if((strcmp(args[1], "aperturaF")==0 || strcmp(args[1], "chiusuraF")==0)){
            printf("L'hub/timer (pid %d, id %d, tipo %c) e i frigoriferi controllati risultano ora %s\n", *cont, idCont, tipoCont, (strcmp(msg, "on")==0 ? "aperti" : "chiusi"));
        }
        else if((strcmp(args[1], "accensione")==0)){
            printf("L'hub/timer (pid %d, id %d, tipo %c) e le lampadine controllate risultano ora %s\n", *cont, idCont, tipoCont, (strcmp(msg, "on")==0 ? "accese" : "spente"));
        }
        else if((strcmp(args[1], "termostato")==0)){
            printf("L'hub/timer (pid %d, id %d, tipo %c) e i frigoriferi controllati hanno ora temperatura interna pari a %s\n", *cont, idCont, tipoCont, msg);
        }
        return;
    }

    if(strcmp(args[1],"termostato")!=0){
        if (strcmp(msg, "on")==0 || strcmp(msg, "aperto")==0)
        {
            printf("Il dispositivo (pid %d, id %d, tipo %c) risulta ora %s\n", *cont, idCont, tipoCont, (tipoCont=='b' ? "acceso" : "aperto"));
        }
        else
        {
            printf("Il dispositivo (pid %d, id %d, tipo %c) risulta ora %s\n", *cont, idCont, tipoCont, (tipoCont=='b' ? "spento" : "chiuso"));
        }
    }
    else
    {
        printf("Il frigorifero (pid %d, id %d, tipo %c) ha ora una temperatura di %s° C\n", *cont, idCont, tipoCont, msg);

    }   
    return;
}

//Ritorna il numero di comandi
//Comandi con dispositivo controllato se type!=0
//Comandi senza dispositivo controllato se type==0
int cen_numCommands(int type)
{
    if (type == 0)
        return (sizeof(noControl_builtin_cmd) / sizeof(char *));
    else
    {
        return (sizeof(control_builtin_cmd) / sizeof(char *));
    }
}

int controlla(int idDisp, int* cenPid, char* tipoDisp){
        int fd;
        //Uso una fifo tra manuale e centralina
        char *manCenFifo = "/tmp/manCenFifo";
        if (mkfifo(manCenFifo, 0666) < 0)
        {
            fprintf(stderr, "Errore nella creazione della fifo %s %s\n", manCenFifo, strerror(errno));
        }
        //Mando signal a centralina per aprire la fifo in READ_ONLY
        int esito=kill(*cenPid, SIGUSR2);
        //Apro la fifo in WRITEONLY
        fd = open(manCenFifo, O_WRONLY);
        if (fd < 0)
        {
            fprintf(stderr, "Errore nell'apertura in WRITE ONLY della fifo %s %s\n", manCenFifo, strerror(errno));
        }
        char msg[20];
        memset(msg, 0, strlen(msg));

        sprintf(msg, "contpid %d", idDisp);

        //Scrivo il messaggio 'contpid ID' sulla fifo con la centralina
        if (write(fd, msg, strlen(msg)) < 0)
        {
            fprintf(stderr, "Errore nella scrittura sulla fifo %s %s\n", manCenFifo, strerror(errno));
        }
        memset(msg, 0, strlen(msg));
        close(fd); //Chiudo in scrittura

        //Ascolto per la risposta
        //Apro pipe in lettura
        fd = open(manCenFifo, O_RDONLY);
        if (fd < 0)
        {
            fprintf(stderr, "Errore nell'apertura in READ ONLY della fifo %s %s\n", manCenFifo, strerror(errno));
        }

        //Utilizzo la struct risp per catturare la risposta
        //Conterra sia il PID del dispositivo cercato che il suo tipo
        risp dispCercato;

        if (read(fd, &dispCercato, sizeof(risp)) < 0)
        {
            fprintf(stderr, "Errore nella lettura dalla fifo %s %s\n", manCenFifo, strerror(errno));
        }
        close(fd); //Chiudo in lettura

        //Memorizzo il pid del dispositivo cercato
        int pidCerc = dispCercato.info_disp.pid;

        //Passo dal tipo scritto in stringa ad il tipo scritto con singolo carattere
        char c = dispCercato.info_disp.tipo[0];
        *tipoDisp = ((c <= 'Z' && c >= 'A') ? c + 32 : c);

        //Ritorno il pid corrispondente all'id indicato come argomento
        if (pidCerc == -1)
        {
            printf("ID indicato non riconosciuto\nL'ID inserito deve essere un dispositivo valido\n");
        }

        //Rimuovo la fifo per la connessione diretta alla centralina che aveva creato poco prima
        remove(manCenFifo);

        //Se ID non valido verrà tornato -1
        return pidCerc;
}

int hand_control(char **args, int *cenPid, char *tipoDisp)
{
    if (args[1] == NULL || args[2] != NULL )
    {
        printf("Errore nei parametri\n");
        printf("Usage: control <pid/nome>\n");

        return -1;
    }
    else
    {
        return controlla(atoi(args[1]), cenPid, tipoDisp);
    }
}

int hand_help(char **args, int *cenPid, char *tipo)
{
    printf("Progetto SO realizzato da: Paolo Tasin, "
           "Stefano Perenzoni, Marcello Rigotti\n");
    printf("Centralina per controllo domotico\n");
    printf("Comandi accettati se non si controlla alcun dispositivo:\n");

    int i;
    for (i = 0; i < cen_numCommands(0); i++)
    {
        printf("> %s\n", noControl_builtin_cmd[i]);
    }

    printf("Comandi accettati mentre si controlla qualche dispositivo:\n");

    for (i = 0; i < cen_numCommands(1); i++)
    {
        printf("> %s\n", control_builtin_cmd[i]);
    }

    return -1;
}

int hand_exit(char **args, int *cenPid, char *tipo)
{
    //Ritorno 0 in modo che lo stato imponga l'uscita dal sistema
    return 0;
}

int hand_release(char **args, int *cont, int idCont, char tipoCont)
{
    *cont = 0;
    return 1;
}

void esegui_switch(char **args, int *cont, int idCont, char tipoCont)
{
    //Creo il comando e gli assegno le informazioni relative allo switch
    cmd comando;
    comando.tipo_comando = 's';
    comando.manuale = 1;
    comando.id = idCont;
    strcpy(comando.cmdInterruttore.nome, args[1]);
    strcpy(comando.cmdInterruttore.stato, args[2]);
    //Scrivo sulla fifo e mando sig2
    //Apro la fifo in scrittura

    //mando segnale al dispositivo che dovrà aprire fifo e mettersi in ascolto
    kill(*cont, SIGUSR2);

    fdManual =  open(fifoManDisp, O_WRONLY);
    if (fdManual < 0)
    {
        fprintf(stderr, "Errore in apertura WRITE ONLY della fifo %s %s", fifoManDisp, strerror(errno));
    }

    //Scrivo sulla fifo il messaggio ed invio SIGUSR2 al dispositivo per indicare che è stato scritto un nuovo comando nella fifo
    write(fdManual, &comando, sizeof(comando));
    if (fdManual < 0)
    {
        fprintf(stderr, "Errore in scrittura sulla fifo %s %s", fifoManDisp, strerror(errno));
    }
    //Chiudo la Fifo in scrittura
    close(fdManual);

    //Apro in lettura
    fdManual = open(fifoManDisp, O_RDONLY);

    char msg[10];
    //leggo la risposta
    read(fdManual, msg, 10);
    close(fdManual);
    
    //Funzione per la stampa dell'output a seconda dei diversi casi: dispositivo, label e valore
    stampaOutput(args, cont, idCont, tipoCont, msg);
}


void esegui_set(char **args, int *cont, int idCont, char tipoCont){
    //Creo il comando e gli assegno le informazioni relative al set
    cmd comando;
    comando.tipo_comando = 'p'; //Comando p per il set
    comando.manuale = 1;
    comando.id = idCont;
    strcpy(comando.cmdInterruttore.nome, args[1]);
    strcpy(comando.cmdInterruttore.stato, args[2]);
    //Scrivo sulla fifo e mando sig2
    //Apro la fifo in scrittura

    //mando segnale al dispositivo che dovrà aprire fifo e mettersi in ascolto
    kill(*cont, SIGUSR2);

    //apro fifo in write only con controllo di eventuale errore
    fdManual =  open(fifoManDisp, O_WRONLY);
    if (fdManual < 0)
    {
        fprintf(stderr, "Errore in apertura WRITE ONLY della fifo %s %s", fifoManDisp, strerror(errno));
    }

    //Scrivo sulla fifo il messaggio
    write(fdManual, &comando, sizeof(comando));
    if (fdManual < 0)
    {
        fprintf(stderr, "Errore in scrittura sulla fifo %s %s", fifoManDisp, strerror(errno));
    }

    //Chiudo la Fifo in scrittura
    close(fdManual);

    //Apro in lettura
    fdManual = open(fifoManDisp, O_RDONLY);

    char msg[10];
    //leggo la risposta
    read(fdManual, msg, 10);

    //Chiudo in readonly
    close(fdManual);

    //TODO Compattare il tutto e fare il caso degli hub
    

    if(tipoCont=='h' || tipoCont=='f'){
        if(strcmp(args[1], "delay")==0){
            printf("Il tempo di delay dell'hub (pid %d, id %d, tipo %c) e dei frigoriferi controllati è ora pari a %s sec\n", *cont, idCont, tipoCont, msg);
        }else if(strcmp(args[1], "perc")==0){
            printf("La percentuale di riempimento dell'hub (pid %d, id %d, tipo %c) e dei frigoriferi controllati è ora pari a %s%%\n", *cont, idCont, tipoCont, msg);
        }
    }else{
        if(strcmp(args[1], "delay")==0){
            printf("Il tempo di delay del frigorifero (pid %d, id %d, tipo %c) è ora pari a %s sec\n", *cont, idCont, tipoCont, msg);
        }else if(strcmp(args[1], "perc")==0){
            printf("La percentuale di riempimento del frigorifero (pid %d, id %d, tipo %c) è ora pari a %s%%\n", *cont, idCont, tipoCont, msg);
        }
    }
}

//Funzioni che passata una stringa ritorna 1 se è un numero intero, 0 se non lo è
int isNum(char* str){
    int length = strlen (str);
    int i;
    for (i=0;i<length; i++){
        if (!isdigit(str[i]) && str[i]!='-')
        {
            //Ritorno 0 se non è numero
            return 0;
        }
    }
    //Ritorno 1 se è numero
    return 1;
}

int checkHubArgs(char **args, char tipoCont){
    //Per hub e timer le label permesse sono diverse, controllo che quella inserita sia uguale ad una delle tante
    return (tipoCont == 'h' || tipoCont=='t' )&& (strcmp(args[1], "apertura") == 0 || strcmp(args[1], "chiusura")==0 || strcmp(args[1], "aperturaF")==0 ||
        strcmp(args[1], "chiusuraF")==0 || strcmp(args[1], "aperturaW")==0 || strcmp(args[1], "chiusuraW")==0 || strcmp(args[1], "termostato")==0);
}

//Si occupa dei controlli degli argomenti per l'esecuzione dello switch
int hand_switch(char **args, int *cont, int idCont, char tipoCont)
{
    if (args[2] == NULL || args[3] != NULL) //Controlla che il numero di argomenti inseriti sia corretto
    {
        printf("Errore nei parametri\n");
        printf("Usage: switch <label> <nuovostato>\n");

        return -1;
    }
    else if ((tipoCont == 'b' || tipoCont =='h' || tipoCont=='t') && strcmp(args[1], "accensione") == 0)
    {
        //Controlli per dispositivo BULB
        if (strcmp(args[2], "on") != 0 && strcmp(args[2], "off") != 0)
        {
            printf("Errore nei parametri\n");
            printf("Usage: switch <label> <nuovostato>\n");
            printf("Label ammesse pr tipo bulb: accensione\n");
            printf("Stati ammessi per label accensione: on/off\n");
            return -1;
        }

        esegui_switch(args, cont, idCont, 'b');
        return -1;
    }
    else if (tipoCont == 'w' && (strcmp(args[1], "apertura") == 0 || strcmp(args[1], "chiusura") == 0 || strcmp(args[1], "aperturaW") == 0 || strcmp(args[1], "chiusuraW") == 0))
    {
        //Controlli per dispositivo WINDOW
        if (strcmp(args[2], "on") != 0 && strcmp(args[2], "off") != 0)
        {
            printf("Errore nei parametri\n");
            printf("Usage: switch <label> <nuovostato>\n");
            printf("Label ammesse pr tipo window: apertura/chiusura\n");
            printf("Stati ammessi per label apertura/chiusura: on/off\n");
            return -1;
        }

        esegui_switch(args, cont, idCont, 'w');
        return -1;
    }
    else if (tipoCont == 'f' && (strcmp(args[1], "apertura") == 0 || strcmp(args[1], "chiusura") == 0 || strcmp(args[1], "aperturaF") == 0 || strcmp(args[1], "chiusuraF") == 0))
    {
        //Controlli per dispositivo FRIDGE 
        if (strcmp(args[2], "on") != 0 && strcmp(args[2], "off") != 0)
        {
            printf("Errore nei parametri\n");
            printf("Usage: switch <label> <nuovostato>\n");
            printf("Label ammesse pr tipo fridge: apertura/chiusura\n");
            printf("Stati ammessi per label apertura/chiusura: on/off\n");
            return -1;
        }

        esegui_switch(args, cont, idCont, 'f');
        return -1;
    }
    else if(tipoCont == 'f' && (strcmp(args[1],"termostato") ==0 )){
        //Controllo per dispositivo FRIDGE con termostato come label

        //argomento rappresentante la temperatura deve essere un numero intero
        if(isNum(args[2])==0){
            printf("Errore nei parametri\n");
            printf("Usage: switch termostato <nuovaTemp>\n");
            printf("Temperatura deve essere di tipo intero\n");
            return -1;
        }

        esegui_switch(args,cont,idCont, 'f');
        return -1;
    }
    else if (checkHubArgs(args, tipoCont))
    {
        //Controlli per dispositivo hub
        if ((strcmp(args[2], "on") != 0 && strcmp(args[2], "off") != 0) && (strcmp(args[1], "termostato")==0 && isNum(args[2])==0))
        {
            printf("Errore nei parametri\n");
            printf("Usage: switch <label> <nuovostato>\n");
            printf("Label ammesse pr tipo bulb: accensione\n");
            printf("Stati ammessi per label accensione: on/off\n");
            return -1;
        }

        esegui_switch(args, cont, idCont, 'h');
        return -1;
    }
    else
    {
        printf("Combinazione tra label e tipo dispositivo controllato non riconosciuta\n");
        printf("Usage: switch <label> <nuovostato>\n");
        printf("Label ammesse pr tipo bulb: accensione\n");
        printf("Label ammesse pr tipo window: apertura/chiusura\n");
        printf("Label ammesse pr tipo fridge: apertura/chiusura\n");
        return -1;
    }
}

int hand_exit1(char **args, int *contPid, int contId, char tipoCont)
{
    return hand_exit(args, contPid, &tipoCont);
}

int hand_set(char **args, int *contPid, int contId, char tipoCont)
{
    if ((tipoCont != 'f' && tipoCont != 'h' && tipoCont != 't') || args[2] == NULL || args[3] != NULL)
    {
        printf("Comando set valido solo con frigorifero, hub o timer come tipo di dispositivo controllato\n");
        printf("Indicare una delle 3 proprietà: delay, perc, termostato\n");
        printf("Usage: set <proprietà> <valore>\n");
        printf("Indicare i valori nelle seguenti modalità:\n-Secondi per delay\n-Percentuale 0-100 per perc\n-Gradi Celsius per temp\n");

        return -1;
    }
    else
    {
        if(isNum(args[2])==0){
            printf("Valore non numerico non ammesso, inserire valore intero\n");
            return -1;
        }
        //Prima faccio i controlli sui valori
        if (strcmp(args[1], "delay") == 0)
        {
            if (atoi(args[2]) < 0)
            {
                printf("Valore per delay non ammesso, inserire delay maggiore di 0\n");
                return -1;
            }
            esegui_set(args, contPid, contId, tipoCont);
            return -1;
        }
        else if (strcmp(args[1], "termostato") == 0)
        {
            //Uguale a comando mandato tramite switch
            esegui_switch(args, contPid, contId, tipoCont);
            return -1;
        }
        else if (strcmp(args[1], "perc") == 0)
        {
            //Controllo che il valore per la percentuale inserito sia tra 0 e 100
            int p = atoi(args[2]);
            if (p < 0 || p > 100)
            {
                printf("Valore per percentuale non ammesso, inserire percentuale compresa tra 0 e 100\n");
                return -1;
            }
            esegui_set(args, contPid, contId, tipoCont);
            return -1;

        }
        else
        {
            printf("Proprietà non riconosciuta\n");
            printf("Indicare una delle 3 proprietà: delay, perc, temp\n");
            printf("Usage: set <proprietà> <valore>\n");
            printf("Indicare i valori nelle seguenti modalità:\n-Secondi per delay\n-Percentuale 0-100 per perc\n-Gradi Celsius per temp\n");

            return -1;
        }

    }
}
