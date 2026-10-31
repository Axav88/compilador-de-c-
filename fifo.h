#ifndef FIFO_H
#define FIFO_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TOKEN_PALABRA_RESERVADA,
    TOKEN_OPERADOR_LOGICO,
    TOKEN_DELIMITADOR,
    TOKEN_IDENTIFICADOR,
    TOKEN_NUMERO,
    TOKEN_EOF
} TipoToken;

typedef struct {
    TipoToken tipo;
    char lexema[128];
    uint32_t hash;
    unsigned int linea;
} Token;

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