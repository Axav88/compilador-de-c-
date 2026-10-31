#include "hash_stack.h"
#include <stdlib.h>
#include <string.h>

// --- CORE DEL HASH ---
// Aquí asumo que mantienes tus constantes PRIME32_1 al 5 y la función xxh_rotl del código anterior
// Solo tienes que pegar esa parte del algoritmo aquí.

// Función original de 32 bits (interna)
static uint32_t xxhash32_internal(const void *input, size_t len, uint32_t seed) {
    // (Pega aquí la lógica completa del algoritmo xxhash32 que ya tenías)
    return 0; // Reemplaza esto con tu algoritmo real
}

// 1. Modificación a 16 bits (Retorno solicitado)
uint16_t xxhash16(const void *input, size_t len, uint32_t seed) {
    uint32_t h32 = xxhash32_internal(input, len, seed);
    
    // PLEGADO (Folding): Para no perder la calidad matemática de los 16 bits más altos,
    // aplicamos un XOR entre la mitad superior y la mitad inferior.
    return (uint16_t)((h32 >> 16) ^ (h32 & 0xFFFF));
}

// 2. Función de Máscara
uint16_t aplicar_mascara(uint16_t hash) {
    // Un simple AND bit a bit garantiza que el número jamás pase de 1023
    return hash & HASH_MASK; 
}


// --- MÉTODOS DE LA "CLASE" HASH-STACK ---

void hs_table_init(HashStackTable *ht) {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        ht->tabla[i] = NULL;
    }
}

// PUSH: Inserta el símbolo en la tabla. Si hay colisión o el símbolo ya existe, lo pone encima (LIFO)
void hs_table_push(HashStackTable *ht, Token tok) {
    uint16_t hash_16 = xxhash16(tok.lexema, strlen(tok.lexema), 0);
    uint16_t indice = aplicar_mascara(hash_16); // Reemplaza el módulo (%)

    HashNode *nuevo = (HashNode *)malloc(sizeof(HashNode));
    nuevo->token = tok;
    nuevo->token.hash = hash_16; // Guardamos el hash corto

    // Empuja al tope de la lista enlazada de este índice específico
    nuevo->siguiente = ht->tabla[indice];
    ht->tabla[indice] = nuevo;
}

// PEEK: Busca y lee el último símbolo guardado con esa clave sin borrarlo
bool hs_table_peek(HashStackTable *ht, const char *clave, Token *out_tok) {
    uint16_t hash_16 = xxhash16(clave, strlen(clave), 0);
    uint16_t indice = aplicar_mascara(hash_16);

    HashNode *actual = ht->tabla[indice];
    while (actual != NULL) {
        if (strcmp(actual->token.lexema, clave) == 0) {
            *out_tok = actual->token;
            return true; // Encontrado
        }
        actual = actual->siguiente;
    }
    return false; // No existe
}

// POP: Saca y elimina el último símbolo guardado con esa clave
bool hs_table_pop(HashStackTable *ht, const char *clave, Token *out_tok) {
    uint16_t hash_16 = xxhash16(clave, strlen(clave), 0);
    uint16_t indice = aplicar_mascara(hash_16);

    HashNode *actual = ht->tabla[indice];
    HashNode *anterior = NULL;

    while (actual != NULL) {
        if (strcmp(actual->token.lexema, clave) == 0) {
            *out_tok = actual->token;

            // Desconectar el nodo (hacer pop)
            if (anterior == NULL) {
                ht->tabla[indice] = actual->siguiente; // Era el primer elemento
            } else {
                anterior->siguiente = actual->siguiente; // Estaba encadenado
            }
            free(actual);
            return true;
        }
        anterior = actual;
        actual = actual->siguiente;
    }
    return false;
}