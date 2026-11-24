#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "stack.h"
#include <stdint.h>
#include <stddef.h>

#define HASH_TABLE_SIZE 1024

typedef struct SymHashNode {
    char *clave;
    TipoToken tipo;
    struct SymHashNode *siguiente;
} SymHashNode;

uint32_t xxhash32(const void *input, size_t len, uint32_t seed);
void symtab_init(void);
void symtab_insert(const char *clave, TipoToken tipo);
TipoToken symtab_lookup(const char *clave);

#endif // SYMBOL_TABLE_H
