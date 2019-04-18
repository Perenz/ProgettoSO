int dev_bulb();
int dev_window();
int dev_fridge();
int dev_timer();
int dev_hub();

//device list
char *builtin_device[]={
        "bulb",
        "window",
        "fridge",
        "timer",
        "hub"
};

int (*builtin_addDevice[]) () = {
        &dev_bulb,
        &dev_window,
        &dev_fridge,
        &dev_timer,
        &dev_hub
};

int device_number(){
    return sizeof(builtin_device)/sizeof(char*);
}

int dev_bulb(){
    printf("Implementare l'aggiunta della bulb\n");

    return 1;
}

int dev_window(){
    printf("Implementare l'aggiunta della window\n");

    return 1;
}

int dev_fridge(){
    printf("Implementare l'aggiunta del fridge\n");

    return 1;
}

int dev_timer(){
    printf("Implementare l'aggiunta del timer\n");

    return 1;
}

int dev_hub(){
    printf("Implementare l'aggiunta dell'hub\n");

    return 1;
}

int cen_add(char **args){
        //Implementazione del comando spostabile in un altro file.c
        if(args[1]==NULL){
                printf("Argomenti non validi\n");
                printf("Utilizzo: add <device>\n");
                printf("Comando 'device' per vedere la lista di quelli disponibili\n");
        }

        //3 devide disponibili: bulb, window, fridge
        else{
            int i=0;
            for(i=0; i<device_number(); i++){
                if(strcmp(args[1], builtin_device[i])==0)
                    return builtin_addDevice[i]();
            }
        }

        printf("Device indicato non riconosciuto\n");
        printf("Utilizzo: add <device>\n");
        printf("Digitare il comando 'device' per la lista di quelli disponibili\n");

        return 1;        
}

