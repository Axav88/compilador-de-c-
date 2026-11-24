#include "symbol_table.h"
#include <stdlib.h>
#include <string.h>

#define PRIME32_1 2654435761U
#define PRIME32_2 2246822519U
#define PRIME32_3 3266489917U
#define PRIME32_4  668265263U
#define PRIME32_5  374761393U

static inline uint32_t xxh_rotl(uint32_t x, int r) {
    return (x << r) | (x >> (32 - r));
}

uint32_t xxhash32(const void *input, size_t len, uint32_t seed) {
    const uint8_t *p = (const uint8_t *)input;
    const uint8_t *const bEnd = p + len;
    uint32_t h32;

    if (len >= 16) {
        const uint8_t *const limit = bEnd - 16;
        uint32_t v1 = seed + PRIME32_1 + PRIME32_2;
        uint32_t v2 = seed + PRIME32_2;
        uint32_t v3 = seed + 0;
        uint32_t v4 = seed - PRIME32_1;

        do {
            uint32_t read1 = *(const uint32_t *)p; p += 4;
            v1 = xxh_rotl(v1 + read1 * PRIME32_2, 13) * PRIME32_1;
            uint32_t read2 = *(const uint32_t *)p; p += 4;
            v2 = xxh_rotl(v2 + read2 * PRIME32_2, 13) * PRIME32_1;
            uint32_t read3 = *(const uint32_t *)p; p += 4;
            v3 = xxh_rotl(v3 + read3 * PRIME32_2, 13) * PRIME32_1;
            uint32_t read4 = *(const uint32_t *)p; p += 4;
            v4 = xxh_rotl(v4 + read4 * PRIME32_2, 13) * PRIME32_1;
        } while (p <= limit);

        h32 = xxh_rotl(v1, 1) + xxh_rotl(v2, 7) + xxh_rotl(v3, 12) + xxh_rotl(v4, 18);
    } else {
        h32 = seed + PRIME32_5;
    }

    h32 += (uint32_t)len;

    while (p + 4 <= bEnd) {
        h32 += (*(const uint32_t *)p) * PRIME32_3;
        h32 = xxh_rotl(h32, 17) * PRIME32_4;
        p += 4;
    }

    while (p < bEnd) {
        h32 += (*p) * PRIME32_5;
        h32 = xxh_rotl(h32, 11) * PRIME32_1;
        p++;
    }

    h32 ^= h32 >> 15;
    h32 *= PRIME32_2;
    h32 ^= h32 >> 13;
    h32 *= PRIME32_3;
    h32 ^= h32 >> 16;

    return h32 & 0x3ff;
}

static SymHashNode *tabla_simbolos[HASH_TABLE_SIZE];

void symtab_init(void) {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        tabla_simbolos[i] = NULL;
    }

    const char *keywords[] = {
        "int", "float", "double", "char", "void", "if", "else", 
        "while", "for", "return", "switch", "case", "default", 
        "struct", "typedef", "sizeof", "break", "continue",
        "short", "long", "unsigned", "signed"
    };
    for (size_t i = 0; i < sizeof(keywords)/sizeof(keywords[0]); i++) {
        symtab_insert(keywords[i], TOKEN_PALABRA_RESERVADA);
    }

    symtab_insert("&&", TOKEN_OPERADOR_LOGICO);
    symtab_insert("||", TOKEN_OPERADOR_LOGICO);
    symtab_insert("!",  TOKEN_OPERADOR_LOGICO);

    symtab_insert("==", TOKEN_OPERADOR_RELACIONAL);
    symtab_insert("!=", TOKEN_OPERADOR_RELACIONAL);
    symtab_insert("<=", TOKEN_OPERADOR_RELACIONAL);
    symtab_insert(">=", TOKEN_OPERADOR_RELACIONAL);
    symtab_insert("<",  TOKEN_OPERADOR_RELACIONAL);
    symtab_insert(">",  TOKEN_OPERADOR_RELACIONAL);

    symtab_insert("=",  TOKEN_ASIGNACION);

    symtab_insert("+",  TOKEN_OPERADOR_ARITMETICO);
    symtab_insert("-",  TOKEN_OPERADOR_ARITMETICO);
    symtab_insert("*",  TOKEN_OPERADOR_ARITMETICO);
    symtab_insert("/",  TOKEN_OPERADOR_ARITMETICO);
    symtab_insert("%",  TOKEN_OPERADOR_ARITMETICO);

    const char *delims[] = {"(", ")", "{", "}", "[", "]", ";", ",", ":"};
    for (size_t i = 0; i < sizeof(delims)/sizeof(delims[0]); i++) {
        symtab_insert(delims[i], TOKEN_DELIMITADOR);
    }
}

void symtab_insert(const char *clave, TipoToken tipo) {
    uint32_t hash = xxhash32(clave, strlen(clave), 0);
    size_t idx = hash % HASH_TABLE_SIZE;

    SymHashNode *nodo = (SymHashNode *)malloc(sizeof(SymHashNode));
    if (!nodo) return;
    nodo->clave = strdup(clave);
    nodo->tipo = tipo;
    nodo->siguiente = tabla_simbolos[idx];
    tabla_simbolos[idx] = nodo;
}

TipoToken symtab_lookup(const char *clave) {
    uint32_t hash = xxhash32(clave, strlen(clave), 0);
    size_t idx = hash % HASH_TABLE_SIZE;

    SymHashNode *act = tabla_simbolos[idx];
    while (act) {
        if (strcmp(act->clave, clave) == 0) {
            return act->tipo;
        }
        act = act->siguiente;
    }
    return TOKEN_IDENTIFICADOR;
}
