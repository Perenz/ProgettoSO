#include "listH.h"
NodoPtr listInit(int data){
    NodoPtr toRtn = malloc(sizeof(Nodo));
    toRtn->data=data;
    toRtn->next=NULL;

    return toRtn;
}


void printList(NodoPtr list){
    while(list != NULL){
        if(list->data != -1)
            printf("%d ", list->data);
            
        list=list->next;
    }
    printf("\n");
}

//Dealloco tutta ma meoria allocata dinamicamente per i nodi della lista
void freeList(NodoPtr list){
    NodoPtr node;
    while(list!=NULL){
        node=list;
        list=list->next;
        free(node);
    }
}

NodoPtr insertLast(NodoPtr list, int data, int fd_reader,int fd_writer){
    if(list==NULL){
        NodoPtr toRtn = listInit(data);
        toRtn->fd_reader=fd_reader;
        toRtn->fd_writer=fd_writer;

        return toRtn;
    }
    
    //Scorro la lista fino all'ultima posizione
    NodoPtr n = list;
    while (n->next != NULL)
    {
        n = n->next;
    }
    
    //Inserisco il nuovo nodo in ultima posizione
    NodoPtr tmp = listInit(data);
    tmp->fd_reader=fd_reader;
    tmp->fd_writer=fd_writer;
    n->next=tmp;




    return list;
}
void removeNode(NodoPtr list, int data){
    if(list != NULL){//controllo che la lista non sia vuota
        NodoPtr nodo = list;
        NodoPtr tmp = nodo->next;
        if(tmp == NULL){//La lista ha un elemento


        }else{//la lista ha più di un elemento
            while(tmp != NULL){
                if(tmp->data == data){
                    //printf("Trovato\n");
                    nodo->next = tmp->next;
                    free(tmp);
                    //printList(list);
                    return;
                }
                nodo = nodo->next;
                tmp = tmp->next;
            }
            
        }

    }

}
