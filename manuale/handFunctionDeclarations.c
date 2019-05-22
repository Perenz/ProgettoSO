#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "../strutture/comandiH.h"

int hand_control(char **, int *, char *); //Dovra ritornare il pid del dispositivo specificato come argomento
int hand_help(char **, int *, char *);
int hand_exit(char **, int *, char *);
int hand_exit1(char **, int *, int, char);
int hand_release(char **, int *, int, char);
int hand_switch(char **, int *, int, char);
int hand_set(char **, int *, int, char);

void inizializzaFifo(int);

//fifo per il collegamento DIRETTO con il dispositivo che si sta controllando
int fdManual;
char fifoManDisp[30];

char *noControl_builtin_cmd[] = {
    "control", //Prende in input un parametro, il nome/id del dispositivo su cuiu vogliamo agire
    "help",
    "exit"};

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

int cen_numCommands(int type)
{
    if (type == 0)
        return (sizeof(noControl_builtin_cmd) / sizeof(char *));
    else
    {
        return (sizeof(control_builtin_cmd) / sizeof(char *));
    }
}

int hand_control(char **args, int *cenPid, char *tipoDisp)
{
    int fd;
    if (args[1] == NULL || args[2] != NULL)
    {
        printf("Errore nei parametri\n");
        printf("Usage: control <pid/nome>\n");

        return -1;
    }
    else
    {
        //Uso una fifo tra manuale e centralina
        char *manCenFifo = "/tmp/manCenFifo";
        if (mkfifo(manCenFifo, 0666) < 0)
        {
            fprintf(stderr, "Errore nella creazione della fifo %s %s\n", manCenFifo, strerror(errno));
        }
        //Mando signal a centralina per aprire la fifo in READ_ONLY
        kill(*cenPid, SIGUSR2);
        //Apro la fifo in WRITEONLY
        fd = open(manCenFifo, O_WRONLY);
        if (fd < 0)
        {
            fprintf(stderr, "Errore nell'apertura in WRITE ONLY della fifo %s %s\n", manCenFifo, strerror(errno));
        }
        char msg[20];
        strcat(msg, "contpid ");
        strcat(msg, args[1]); //args1 conterrà l'id del dispositivo che si vuole controllare

        if (write(fd, msg, strlen(msg) + 1) < 0)
        {
            fprintf(stderr, "Errore nella scrittura sulla fifo %s %s\n", manCenFifo, strerror(errno));
        }
        close(fd); //Chiudo in scrittura

        //Ascolto per la risposta
        //Apro pipe in lettura
        fd = open(manCenFifo, O_RDONLY);
        if (fd < 0)
        {
            fprintf(stderr, "Errore nell'apertura in READ ONLY della fifo %s %s\n", manCenFifo, strerror(errno));
        }

        risp dispCercato;

        if (read(fd, &dispCercato, sizeof(risp)) < 0)
        {
            fprintf(stderr, "Errore nella lettura dalla fifo %s %s\n", manCenFifo, strerror(errno));
        }
        close(fd); //Chiudo in lettura
        int pidCerc = dispCercato.info_disp.pid;
        //Passo dal tipo scritto in stringa ad il tipo scritto con singolo carattere
        char c = dispCercato.info_disp.tipo[0];
        *tipoDisp = ((c <= 'Z' && c >= 'A') ? c + 32 : c);

        //Ritorno il pid corrispondente all'id indicato come argomento
        if (pidCerc == -1)
        {
            printf("ID indicato non riconosciuto\nL'ID inserito deve essere un dispositivo valido\n");
        }

        remove(manCenFifo);
        return pidCerc;
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
    return 0;
}

int hand_release(char **args, int *cont, int idCont, char tipoCont)
{
    *cont = 0;
    //TODO da fare la chiusura e la rimozione della fifo
    //Va fatta lato dispositivo
    return 1;
}

void esegui_switch(char **args, int *cont, int idCont, char tipoCont)
{
    //Qui comando
    cmd comando;
    comando.tipo_comando = 's';
    comando.manuale = 1;
    comando.id = idCont;
    strcpy(comando.info_disp.interruttore[0].nome, args[1]);
    strcpy(comando.info_disp.interruttore[0].stato, args[2]);
    //Scrivo sulla fifo e mando sig2
    //Apro la fifo in scrittura

    kill(*cont, SIGUSR2);
    fdManual = open(fifoManDisp, O_WRONLY);
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
    //TODO

    //Apro in lettura
    fdManual = open(fifoManDisp, O_RDONLY);

    char msg[10];
    //leggo la risposta
    int esito = read(fdManual, msg, 10);

    if (atoi(msg) == 1)
    {
        printf("Il dispositivo (pid %d, id %d, tipo %c) risulta ora %s\n", *cont, idCont, tipoCont, (tipoCont=='b' ? "acceso" : "aperto"));
    }
    else
    {
        printf("Il dispositivo (pid %d, id %d, tipo %c) risulta ora %s\n", *cont, idCont, tipoCont, (tipoCont=='b' ? "spento" : "chiuso"));
    }
}


int hand_switch(char **args, int *cont, int idCont, char tipoCont)
{
    if (args[2] == NULL || args[3] != NULL)
    {
        printf("Errore nei parametri\n");
        printf("Usage: switch <label> <nuovostato>\n");

        return -1;
    }
    else if (tipoCont == 'b' && strcmp(args[1], "accensione") == 0)
    {
        //Controlli per dispositivo BULB (o hub di bulb)
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
    else if (tipoCont == 'w' && (strcmp(args[1], "apertura") == 0 || strcmp(args[1], "chiusura") == 0))
    {
        //Controlli per dispositivo WINDOW (o hub di window)
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
    else if (tipoCont == 'f' && (strcmp(args[1], "apertura") == 0 || strcmp(args[1], "chiusura") == 0))
    {
        //Controlli per dispositivo WINDOW (o hub di window)
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
    if (tipoCont != 'f' || args[2] == NULL || args[3] != NULL)
    {
        printf("Comando set valido solo con frigorifero come tipo di dispositivo controllato\n");
        printf("Indicare una delle 3 proprietà: delay, perc, temp\n");
        printf("Usage: set <proprietà> <valore>\n");
        printf("Indicare i valori nelle seguenti modalità:\n-Secondi per delay\n-Percentuale 0-100 per perc\n-Gradi Celsius per temp\n");

        return -1;
    }
    else
    {
        //Prima faccio i controlli sui valori
        if (strcmp(args[1], "delay") == 0)
        {
            if (atoi(args[2]) < 0)
            {
                printf("Valore negativo per tempo di delay non ammesso\n");
                return -1;
            }
        }
        else if (strcmp(args[1], "temp") == 0)
        {
        }
        else if (strcmp(args[1], "perc") == 0)
        {
            int p = atoi(args[2]);
            if (p < 0 || p > 100)
            {
                printf("Valore per percentuale non ammesso, inserire percentuale compresa tra 0 e 100\n");
                return -1;
            }
        }
        else
        {
            printf("Proprietà non riconosciuta\n");
            printf("Indicare una delle 3 proprietà: delay, perc, temp\n");
            printf("Usage: set <proprietà> <valore>\n");
            printf("Indicare i valori nelle seguenti modalità:\n-Secondi per delay\n-Percentuale 0-100 per perc\n-Gradi Celsius per temp\n");

            return -1;
        }

        //Ora mando il comando
    }
}
