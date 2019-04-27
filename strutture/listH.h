#ifndef __LIST_H__
#define __LIST_H__
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct Nodo{
    int data;
    struct Nodo* next;
}Nodo;


typedef Nodo* NodoPtr;

NodoPtr listInit(int);
void printList(NodoPtr);
NodoPtr insertLast(NodoPtr, int);

#endif