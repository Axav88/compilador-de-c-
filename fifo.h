#ifndef FIFO_H
#define FIFO_H

#include "stack.h"

typedef struct QueueNode {
    Token token;
    struct QueueNode *siguiente;
} QueueNode;

typedef struct {
    QueueNode *frente;
    QueueNode *final;
    int tamano;
} TokenFIFO;

void fifo_init(TokenFIFO *q);
void fifo_push(TokenFIFO *q, Token tok);
bool fifo_pop(TokenFIFO *q, Token *out_tok);
bool fifo_is_empty(const TokenFIFO *q);
void fifo_free(TokenFIFO *q);

#endif // FIFO_H
