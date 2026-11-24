#ifndef STACK_H
#define STACK_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TOKEN_PALABRA_RESERVADA,
    TOKEN_OPERADOR_LOGICO,
    TOKEN_OPERADOR_RELACIONAL,
    TOKEN_OPERADOR_ARITMETICO,
    TOKEN_ASIGNACION,
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

// Nodo individual de la pila
typedef struct StackNode {
    Token token;
    struct StackNode *siguiente;
} StackNode;

// Estructura controladora de la pila (la "clase")
typedef struct {
    StackNode *tope;
    int tamano;
} TokenStack;

void stack_init(TokenStack *s);
void stack_push(TokenStack *s, Token tok);
bool stack_pop(TokenStack *s, Token *out_tok);
bool stack_is_empty(const TokenStack *s);
void stack_free(TokenStack *s);

#endif // STACK_H
