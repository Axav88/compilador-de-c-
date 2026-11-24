#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "logger.h"
#include "stack.h"
#include "symbol_table.h"
#include "ast.h"

static logger_t lex_logger;
static logger_t parse_logger;
static TokenStack token_stack;
static ast_t syntax_tree;
static unsigned int current_line = 1;

void emit_token(TipoToken tipo, const char *lexema) {
    Token tok;
    tok.tipo = tipo;
    strncpy(tok.lexema, lexema, sizeof(tok.lexema) - 1);
    tok.lexema[sizeof(tok.lexema) - 1] = '\0';
    tok.hash = xxhash32(lexema, strlen(lexema), 0);
    tok.linea = current_line;

    // 1. Fase 1: Encolar token en la pila LIFO
    stack_push(&token_stack, tok);

    // 2. Registrar en el Logger
    logger_print_log(&lex_logger, LOG_LEVEL_DEBUG, 
                     "Token emitido: '%s' (Tipo: %d, Hash: 0x%08X, Linea: %u)", 
                     tok.lexema, tok.tipo, tok.hash, tok.linea);

    // 3. Fase 2: Enviar token en orden secuencial (cronológico de izq. a der.) al parser del AST
    analyze_token(&syntax_tree, &tok);
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

        // Directivas de preprocesador (#include, #define, etc.)
        if (c == '#') {
            while ((c = fgetc(f)) != EOF && c != '\n');
            if (c == '\n') current_line++;
            continue;
        }

        // Comentarios u operador aritmético de división '/'
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
            } else if (sig == '=') {
                emit_token(TOKEN_ASIGNACION, "/=");
                continue;
            } else {
                ungetc(sig, f);
                emit_token(TOKEN_OPERADOR_ARITMETICO, "/");
                continue;
            }
        }

        // Cadenas literales
        if (c == '"') {
            idx = 0;
            buf[idx++] = c;
            while ((c = fgetc(f)) != EOF) {
                if (c == '\n') current_line++;
                if (c == '\\') {
                    if (idx < 126) buf[idx++] = c;
                    c = fgetc(f);
                    if (c == EOF) break;
                    if (idx < 126) buf[idx++] = c;
                } else if (c == '"') {
                    if (idx < 126) buf[idx++] = c;
                    break;
                } else {
                    if (idx < 126) buf[idx++] = c;
                }
            }
            buf[idx] = '\0';
            emit_token(TOKEN_IDENTIFICADOR, buf);
            continue;
        }

        // Caracteres literales ('c')
        if (c == '\'') {
            idx = 0;
            buf[idx++] = c;
            while ((c = fgetc(f)) != EOF) {
                if (c == '\\') {
                    if (idx < 126) buf[idx++] = c;
                    c = fgetc(f);
                    if (c == EOF) break;
                    if (idx < 126) buf[idx++] = c;
                } else if (c == '\'') {
                    if (idx < 126) buf[idx++] = c;
                    break;
                } else {
                    if (idx < 126) buf[idx++] = c;
                }
            }
            buf[idx] = '\0';
            emit_token(TOKEN_NUMERO, buf);
            continue;
        }

        // Operadores de Asignación y Relacional de Igualdad (=, ==)
        if (c == '=') {
            int sig = fgetc(f);
            if (sig == '=') {
                emit_token(TOKEN_OPERADOR_RELACIONAL, "==");
            } else {
                ungetc(sig, f);
                emit_token(TOKEN_ASIGNACION, "=");
            }
            continue;
        }

        // Operadores Relacionales y Negación Lógica (!, !=)
        if (c == '!') {
            int sig = fgetc(f);
            if (sig == '=') {
                emit_token(TOKEN_OPERADOR_RELACIONAL, "!=");
            } else {
                ungetc(sig, f);
                emit_token(TOKEN_OPERADOR_LOGICO, "!");
            }
            continue;
        }

        // Operadores Relacionales (<, <=)
        if (c == '<') {
            int sig = fgetc(f);
            if (sig == '=') {
                emit_token(TOKEN_OPERADOR_RELACIONAL, "<=");
            } else {
                ungetc(sig, f);
                emit_token(TOKEN_OPERADOR_RELACIONAL, "<");
            }
            continue;
        }

        // Operadores Relacionales (>, >=)
        if (c == '>') {
            int sig = fgetc(f);
            if (sig == '=') {
                emit_token(TOKEN_OPERADOR_RELACIONAL, ">=");
            } else {
                ungetc(sig, f);
                emit_token(TOKEN_OPERADOR_RELACIONAL, ">");
            }
            continue;
        }

        // Operadores Aritméticos (+, ++, +=)
        if (c == '+') {
            int sig = fgetc(f);
            if (sig == '+') {
                emit_token(TOKEN_OPERADOR_ARITMETICO, "++");
            } else if (sig == '=') {
                emit_token(TOKEN_ASIGNACION, "+=");
            } else {
                ungetc(sig, f);
                emit_token(TOKEN_OPERADOR_ARITMETICO, "+");
            }
            continue;
        }

        // Operadores Aritméticos (-, --, -=)
        if (c == '-') {
            int sig = fgetc(f);
            if (sig == '-') {
                emit_token(TOKEN_OPERADOR_ARITMETICO, "--");
            } else if (sig == '=') {
                emit_token(TOKEN_ASIGNACION, "-=");
            } else {
                ungetc(sig, f);
                emit_token(TOKEN_OPERADOR_ARITMETICO, "-");
            }
            continue;
        }

        // Operadores Aritméticos (*, *=)
        if (c == '*') {
            int sig = fgetc(f);
            if (sig == '=') {
                emit_token(TOKEN_ASIGNACION, "*=");
            } else {
                ungetc(sig, f);
                emit_token(TOKEN_OPERADOR_ARITMETICO, "*");
            }
            continue;
        }

        // Operadores Aritméticos (%, %=)
        if (c == '%') {
            int sig = fgetc(f);
            if (sig == '=') {
                emit_token(TOKEN_ASIGNACION, "%=");
            } else {
                ungetc(sig, f);
                emit_token(TOKEN_OPERADOR_ARITMETICO, "%");
            }
            continue;
        }

        // Operadores Lógicos (&&, ||, &, |)
        if (c == '&' || c == '|') {
            int sig = fgetc(f);
            if ((c == '&' && sig == '&') || (c == '|' && sig == '|')) {
                char op[3] = { (char)c, (char)sig, '\0' };
                emit_token(TOKEN_OPERADOR_LOGICO, op);
            } else {
                ungetc(sig, f);
                char op[2] = { (char)c, '\0' };
                emit_token(TOKEN_OPERADOR_LOGICO, op);
            }
            continue;
        }

        // Delimitadores
        if (strchr("(){}[];,:", c)) {
            buf[0] = (char)c;
            buf[1] = '\0';
            emit_token(TOKEN_DELIMITADOR, buf);
            continue;
        }

        // Números (Enteros, Hexadecimales, Binarios, Octales, Flotantes)
        if (isdigit(c)) {
            idx = 0;
            buf[idx++] = (char)c;

            // Detección de prefijos 0x (hex) o 0b (binario)
            if (c == '0') {
                int sig = fgetc(f);
                if (sig == 'x' || sig == 'X' || sig == 'b' || sig == 'B') {
                    buf[idx++] = (char)sig;
                } else {
                    ungetc(sig, f);
                }
            }

            while ((c = fgetc(f)) != EOF) {
                if (isalnum(c) || c == '.') {
                    if (idx < 126) buf[idx++] = (char)c;
                } else if ((c == '+' || c == '-') && (buf[idx - 1] == 'e' || buf[idx - 1] == 'E' || buf[idx - 1] == 'p' || buf[idx - 1] == 'P')) {
                    if (idx < 126) buf[idx++] = (char)c;
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
            buf[idx++] = (char)c;
            while ((c = fgetc(f)) != EOF && (isalnum(c) || c == '_')) {
                if (idx < 126) buf[idx++] = (char)c;
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

        // Caracter no reconocido (Error Léxico)
        logger_set_status(&lex_logger, 2, current_line, 101, "Caracter no reconocido");
        logger_print_log(&lex_logger, LOG_LEVEL_ERROR, "Caracter desconocido '%c' en linea %u", c, current_line);
    }

    // Emitir fin de archivo hacia el parser
    Token eof_tok;
    eof_tok.tipo = TOKEN_EOF;
    strncpy(eof_tok.lexema, "EOF", sizeof(eof_tok.lexema) - 1);
    eof_tok.hash = 0;
    eof_tok.linea = current_line;
    analyze_token(&syntax_tree, &eof_tok);

    fclose(f);
}

int main(int argc, char *argv[]) {
    const char *archivo = "numeros.c";
    if (argc > 1) archivo = argv[1];

    // 1. Inicializar subsistemas
    logger_init(&lex_logger, "LEXER", NULL, false, "compilador.log");
    logger_init(&parse_logger, "PARSER", NULL, false, "compilador.log");
    stack_init(&token_stack);
    symtab_init();
    ast_init(&syntax_tree, &parse_logger);

    logger_print_log(&lex_logger, LOG_LEVEL_INFO, "Iniciando analisis lexico y sintactico de '%s'...", archivo);

    // 2. Ejecutar análisis léxico y construcción top-down del AST
    parse_file(archivo);

    // 3. Imprimir el AST generado
    print_tree(&syntax_tree);

    // 4. Liberar memoria dinámica
    ast_free(&syntax_tree);
    stack_free(&token_stack);

    logger_print_log(&lex_logger, LOG_LEVEL_INFO, "Compilacion y analisis finalizados con exito.");
    return 0;
}
