#include "fifo.h"
#include <stdlib.h>
#include <string.h>

void fifo_init(TokenFIFO *q) {
    q->frente = NULL;
    q->final = NULL;
    q->tamano = 0;
}

void fifo_push(TokenFIFO *q, Token tok) {
    QueueNode *nodo = (QueueNode *)malloc(sizeof(QueueNode));
    nodo->token = tok;
    nodo->siguiente = NULL;

    if (q->final == NULL) {
        q->frente = nodo;
        q->final = nodo;
    } else {
        q->final->siguiente = nodo;
        q->final = nodo;
    }
    q->tamano++;
}

bool fifo_pop(TokenFIFO *q, Token *out_tok) {
    if (fifo_is_empty(q)) return false;

    QueueNode *temp = q->frente;
    *out_tok = temp->token;

    q->frente = q->frente->siguiente;
    if (q->frente == NULL) {
        q->final = NULL;
    }
    free(temp);
    q->tamano--;
    return true;
}

bool fifo_is_empty(const TokenFIFO *q) {
    return q->frente == NULL;
}

void fifo_free(TokenFIFO *q) {
    Token dummy;
    while (fifo_pop(q, &dummy));
}