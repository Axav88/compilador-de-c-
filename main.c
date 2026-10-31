#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "logger.h"
#include "stack.h"
//#include "fifo.h"
#include "symbol_table.h"

static logger_t lex_logger;
static TokenStack token_stack;
static unsigned int current_line = 1;

void emit_token(TipoToken tipo, const char *lexema) {
    Token tok;
    tok.tipo = tipo;
    strncpy(tok.lexema, lexema, sizeof(tok.lexema) - 1);
    tok.lexema[sizeof(tok.lexema) - 1] = '\0';
    tok.hash = xxhash32(lexema, strlen(lexema), 0);
    tok.linea = current_line;

    // Encolar token en la pila para fases posteriores (ej. Parser)
    stack_push(&token_stack, tok);
    // fifo_push(&token_stack, tok);

    // Registrar en el Logger
    logger_print_log(&lex_logger, LOG_LEVEL_DEBUG, 
                     "Token emitido: '%s' (Tipo: %d, Hash: 0x%08X, Linea: %u)", 
                     tok.lexema, tok.tipo, tok.hash, tok.linea);
}

void parse_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        logger_set_status(&lex_logger, 1, current_line, 404, "No se pudo abrir el archivo fuente");
        logger_print_log(&lex_logger, LOG_LEVEL_FATAL, "Error al abrir %s", filename);
        logger_print_status(&lex_logger);
        return;
    }

    int c;
    char buf[128];
    int idx = 0;

    while ((c = fgetc(f)) != EOF) {
        if (c == '\n') {
            current_line++;
            continue;
        }
        if (isspace(c)) continue;

        // Comentarios
        if (c == '/') {
            int sig = fgetc(f);
            if (sig == '/') {
                while ((c = fgetc(f)) != EOF && c != '\n');
                if (c == '\n') current_line++;
                continue;
            } else if (sig == '*') {
                while ((c = fgetc(f)) != EOF) {
                    if (c == '\n') current_line++;
                    if (c == '*') {
                        int sig2 = fgetc(f);
                        if (sig2 == '/') break;
                        ungetc(sig2, f);
                    }
                }
                continue;
            } else {
                ungetc(sig, f);
            }
        }

        // Cadenas literales
        if (c == '"') {
            while ((c = fgetc(f)) != EOF) {
                if (c == '\n') current_line++;
                if (c == '\\') {
                    fgetc(f);
                } else if (c == '"') {
                    break;
                }
            }
            continue;
        }

        // Números
        if (isdigit(c)) {
            idx = 0;
            buf[idx++] = c;
            while ((c = fgetc(f)) != EOF) {
                if (isalnum(c) || c == '.' || c == '+' || c == '-') {
                    buf[idx++] = c;
                } else {
                    ungetc(c, f);
                    break;
                }
            }
            buf[idx] = '\0';
            emit_token(TOKEN_NUMERO, buf);
            continue;
        }

        // Identificadores y Palabras Reservadas
        if (isalpha(c) || c == '_') {
            idx = 0;
            buf[idx++] = c;
            while ((c = fgetc(f)) != EOF && (isalnum(c) || c == '_')) {
                buf[idx++] = c;
            }
            ungetc(c, f);
            buf[idx] = '\0';

            TipoToken tipo = symtab_lookup(buf);
            if (tipo == TOKEN_IDENTIFICADOR) {
                symtab_insert(buf, TOKEN_IDENTIFICADOR);
            }
            emit_token(tipo, buf);
            continue;
        }

        // Operadores Lógicos
        if (c == '&' || c == '|' || c == '!') {
            idx = 0;
            buf[idx++] = c;
            int sig = fgetc(f);
            if ((c == '&' && sig == '&') || (c == '|' && sig == '|')) {
                buf[idx++] = sig;
            } else {
                ungetc(sig, f);
            }
            buf[idx] = '\0';

            TipoToken tipo = symtab_lookup(buf);
            emit_token(tipo, buf);
            continue;
        }

        // Delimitadores
        if (strchr("(){}[];,:", c)) {
            buf[0] = c;
            buf[1] = '\0';
            emit_token(TOKEN_DELIMITADOR, buf);
            continue;
        }

        // Caracter no reconocido (Error Léxico)
        logger_set_status(&lex_logger, 2, current_line, 101, "Caracter no reconocido");
        logger_print_log(&lex_logger, LOG_LEVEL_ERROR, "Caracter desconocido '%c' en linea %u", c, current_line);
    }

    fclose(f);
}

int main(int argc, char *argv[]) {
    const char *archivo = "numeros.c";
    if (argc > 1) archivo = argv[1];

    // 1. Inicializar subsistemas
    logger_init(&lex_logger, "LEXER", NULL, false, "compilador.log");
    stack_init(&token_stack);           
    //fifo_init(&token_queue);  
    symtab_init();

    logger_print_log(&lex_logger, LOG_LEVEL_INFO, "Iniciando analisis de %s", archivo);

    // 2. Ejecutar análisis léxico
    parse_file(archivo);

    // 3. Procesar pila
    logger_print_log(&lex_logger, LOG_LEVEL_INFO, "Procesando tokens almacenados en la pila (%d tokens)...", token_stack.tamano);
    Token tok;
    while (stack_pop(&token_stack, &tok)) {
        printf("[STACK POP] %-15s | Tipo: %d | Hash: 0x%08X | Linea: %u\n", 
               tok.lexema, tok.tipo, tok.hash, tok.linea);
    }

    stack_free(&token_stack);
    logger_print_log(&lex_logger, LOG_LEVEL_INFO, "Analisis finalizado con exito.");
    return 0;
}