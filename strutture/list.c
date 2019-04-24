#include "listH.h"

NodoPtr listInit(int data){
    NodoPtr toRtn = malloc(sizeof(Nodo));
    toRtn->data=data;
    toRtn->next=NULL;

    return toRtn;
}

void printList(NodoPtr list){
    while(list != NULL){
        printf("%d ", list->data);
        list=list->next;
    }
    printf("\n");
}

NodoPtr insertLast(NodoPtr list, int data){
    if(list==NULL){
        return listInit(data);
    }
    
    //Scorro la lista fino all'ultima posizione
    NodoPtr n= list;
    while (n->next != NULL)
    {
        n = n->next;
    }
    
    //Inserisco il nuovo nodo in ultima posizione
    NodoPtr tmp = listInit(data);
    n->next=tmp;

    return list;
}