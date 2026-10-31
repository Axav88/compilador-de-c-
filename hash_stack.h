#ifndef HASH_STACK_H
#define HASH_STACK_H

#include <stdint.h>
#include <stdbool.h>
#include "stack.h" // Asume que aquí está definida tu estructura Token

// Una tabla de 10 bits tiene exactamente 1024 posiciones (2^10)
#define HASH_TABLE_SIZE 1024 

// Máscara de 10 bits en hexadecimal (0x03FF es 0000 0011 1111 1111 en binario, es decir, 1023)
#define HASH_MASK 0x03FF     

// Nodo de la tabla hash (guarda el token entero)
typedef struct HashNode {
    Token token;
    struct HashNode *siguiente;
} HashNode;

// La "Clase" Tabla Hash de 10 bits
typedef struct {
    HashNode *tabla[HASH_TABLE_SIZE];
} HashStackTable;

// Funciones core
uint16_t xxhash16(const void *input, size_t len, uint32_t seed);
uint16_t aplicar_mascara(uint16_t hash);

// Métodos de la clase HashStackTable
void hs_table_init(HashStackTable *ht);
void hs_table_push(HashStackTable *ht, Token tok);
bool hs_table_peek(HashStackTable *ht, const char *clave, Token *out_tok);
bool hs_table_pop(HashStackTable *ht, const char *clave, Token *out_tok);

#endif // HASH_STACK_H