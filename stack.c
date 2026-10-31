#include "stack.h"
#include <stdlib.h>

void stack_init(TokenStack *s) {
    s->tope = NULL;
    s->tamano = 0;
}

void stack_push(TokenStack *s, Token tok) {
    StackNode *nodo = (StackNode *)malloc(sizeof(StackNode));
    nodo->token = tok;
    
    // El nuevo nodo apunta al que antes era el tope
    nodo->siguiente = s->tope;
    
    // El nuevo tope ahora es este nodo
    s->tope = nodo;
    s->tamano++;
}

bool stack_pop(TokenStack *s, Token *out_tok) {
    if (stack_is_empty(s)) return false;

    // Tomamos el nodo que está en la cima
    StackNode *temp = s->tope;
    *out_tok = temp->token;

    // El nuevo tope pasa a ser el elemento de abajo
    s->tope = temp->siguiente;
    
    free(temp);
    s->tamano--;
    return true;
}

bool stack_is_empty(const TokenStack *s) {
    return s->tope == NULL;
}

void stack_free(TokenStack *s) {
    Token dummy;
    while (stack_pop(s, &dummy)); // Vacía la memoria iterativamente
}