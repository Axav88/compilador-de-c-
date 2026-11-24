#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// MANEJO DE LA PILA DE CONTEXTO (local_stack)
// ============================================================================

void ast_context_push(AstContextStack *s, ast_node *node) {
    if (!s) return;
    AstContextNode *nuevo = (AstContextNode *)malloc(sizeof(AstContextNode));
    if (!nuevo) return;
    nuevo->node = node;
    nuevo->siguiente = s->tope;
    s->tope = nuevo;
    s->tamano++;
}

ast_node *ast_context_pop(AstContextStack *s) {
    if (!s || s->tope == NULL) return NULL;
    AstContextNode *temp = s->tope;
    ast_node *ret = temp->node;
    s->tope = temp->siguiente;
    free(temp);
    s->tamano--;
    return ret;
}

void ast_context_free(AstContextStack *s) {
    if (!s) return;
    while (s->tope != NULL) {
        AstContextNode *temp = s->tope;
        s->tope = temp->siguiente;
        free(temp);
    }
    s->tamano = 0;
}

// ============================================================================
// CONVERSIÓN DE TIPO DE NODO A CADENA
// ============================================================================

const char *ast_node_type_name(ast_nodes type) {
    switch (type) {
        case else_body:            return "ELSE_BODY";
        case function_body:        return "FUNCTION_BODY";
        case function_name:        return "FUNCTION_NAME";
        case function_parameters:  return "FUNCTION_PARAMETERS";
        case if_body:              return "IF_BODY";
        case if_condition:         return "IF_CONDITION";
        case while_condition:      return "WHILE_CONDITION";
        case while_body:           return "WHILE_BODY";
        case statement_node:       return "STATEMENT";
        default:                   return "UNKNOWN_NODE";
    }
}

static bool is_container_node(ast_nodes type) {
    return (type == function_body || type == if_body || type == while_body || type == else_body);
}

// ============================================================================
// FUNCIONES INTERNAS DEL AST (EQUIVALENTES A PRIVATE/PROTECTED)
// ============================================================================

ast_node *make_new_node(ast_nodes type, Token *word) {
    ast_node *nodo = (ast_node *)malloc(sizeof(ast_node));
    if (!nodo) return NULL;

    nodo->branch = NULL;
    nodo->next = NULL;
    nodo->type = type;

    if (word != NULL) {
        nodo->word = (Token *)malloc(sizeof(Token));
        if (nodo->word) {
            *(nodo->word) = *word;
        }
    } else {
        nodo->word = NULL;
    }

    return nodo;
}

void push_branch(ast_t *tree, ast_nodes type, Token *word) {
    if (!tree) return;
    ast_node *nuevo = make_new_node(type, word);
    if (!nuevo) return;

    tree->number_nodes++;

    if (tree->root == NULL) {
        tree->root = nuevo;
        tree->active = nuevo;
        tree->branch = nuevo;
        return;
    }

    // Guarda el nodo activo en la pila para recuperar el contexto al cerrar el bloque con '}'
    ast_context_push(&tree->local_stack, tree->active);

    // Conectar como rama anidada (hijo) del nodo activo actual
    if (tree->active->branch == NULL) {
        tree->active->branch = nuevo;
    } else {
        // Si ya hay una rama existente (ej. else_body como hermano de if_body en la condición),
        // lo enlazamos al final de la lista de hermanos de esa rama
        ast_node *hermano = tree->active->branch;
        while (hermano->next != NULL) {
            hermano = hermano->next;
        }
        hermano->next = nuevo;
    }

    tree->branch = nuevo;
    tree->active = nuevo;
}

void push_node(ast_t *tree, ast_nodes type, Token *word) {
    if (!tree) return;
    ast_node *nuevo = make_new_node(type, word);
    if (!nuevo) return;

    tree->number_nodes++;

    if (tree->root == NULL) {
        tree->root = nuevo;
        tree->active = nuevo;
        tree->branch = nuevo;
        return;
    }

    // Si el nodo activo actual es un contenedor de bloque y aún no tiene sentencias hijas,
    // el nuevo nodo pasa a ser el primer hijo anidado (branch) del contenedor
    if (is_container_node(tree->active->type) && tree->active->branch == NULL) {
        tree->active->branch = nuevo;
        tree->active = nuevo;
        return;
    }

    // De lo contrario, se agrega como hermano al nivel actual del nodo activo
    if (tree->active->next == NULL) {
        tree->active->next = nuevo;
    } else {
        ast_node *hermano = tree->active;
        while (hermano->next != NULL) {
            hermano = hermano->next;
        }
        hermano->next = nuevo;
    }

    tree->active = nuevo;
}

void push_word(ast_t *tree, Token *word) {
    if (!tree || !word) return;

    if (tree->active != NULL && tree->active->word == NULL) {
        tree->active->word = (Token *)malloc(sizeof(Token));
        if (tree->active->word) {
            *(tree->active->word) = *word;
        }
    } else {
        push_node(tree, statement_node, word);
    }
}

void restore_environment(ast_t *tree) {
    if (!tree) return;
    ast_node *padre = ast_context_pop(&tree->local_stack);
    if (padre != NULL) {
        tree->active = padre;
        tree->branch = padre->branch;
    }
}

void print_node(ast_node *current, int depth) {
    if (!current) return;

    for (int i = 0; i < depth; i++) {
        printf("  | ");
    }

    const char *tipo_str = ast_node_type_name(current->type);
    if (current->word != NULL && current->word->lexema[0] != '\0') {
        printf("  +-- [%s] '%s' (linea %u)\n", tipo_str, current->word->lexema, current->word->linea);
    } else {
        printf("  +-- [%s]\n", tipo_str);
    }

    if (current->branch != NULL) {
        print_branch(current->branch, depth + 1);
    }
}

void print_branch(ast_node *branch, int depth) {
    ast_node *actual = branch;
    while (actual != NULL) {
        print_node(actual, depth);
        actual = actual->next;
    }
}

void print_tree(const ast_t *tree) {
    printf("\n================ ARBOL DE ANALISIS SINTACTICO (AST) ================\n");
    if (!tree || !tree->root) {
        printf("El arbol sintactico esta vacio.\n");
    } else {
        printf("Total de nodos construidos: %u\n", tree->number_nodes);
        printf("Jerarquia del AST:\n");
        print_branch(tree->root, 0);
    }
    printf("====================================================================\n\n");
}

// ============================================================================
// FUNCIONES PÚBLICAS
// ============================================================================

void ast_init(ast_t *tree, logger_t *log) {
    if (!tree) return;
    tree->root = NULL;
    tree->active = NULL;
    tree->branch = NULL;
    tree->local_stack.tope = NULL;
    tree->local_stack.tamano = 0;
    tree->number_nodes = 0;
    tree->parser_logger = log;

    tree->state = STATE_GLOBAL;
    tree->panic_mode = false;
    tree->paren_depth = 0;
    tree->brace_depth = 0;

    memset(&tree->last_type_token, 0, sizeof(Token));
    memset(&tree->last_id_token, 0, sizeof(Token));
    memset(&tree->current_token_accum, 0, sizeof(Token));
}

static void free_ast_subtree(ast_node *nodo) {
    if (!nodo) return;
    ast_node *actual = nodo;
    while (actual != NULL) {
        ast_node *siguiente = actual->next;
        if (actual->branch != NULL) {
            free_ast_subtree(actual->branch);
            actual->branch = NULL;
        }
        if (actual->word != NULL) {
            free(actual->word);
            actual->word = NULL;
        }
        free(actual);
        actual = siguiente;
    }
}

void ast_free(ast_t *tree) {
    if (!tree) return;
    if (tree->root != NULL) {
        free_ast_subtree(tree->root);
        tree->root = NULL;
    }
    ast_context_free(&tree->local_stack);
    tree->active = NULL;
    tree->branch = NULL;
    tree->number_nodes = 0;
}

// ============================================================================
// MÁQUINA DE ESTADOS / PARSER DESCENDENTE PREDICTIVO (analyze_token)
// ============================================================================

static void parser_report_error(ast_t *tree, unsigned int linea, const char *esperado, const char *encontrado) {
    char mensaje[128];
    snprintf(mensaje, sizeof(mensaje), "Se esperaba %s pero se encontro '%s'", esperado, encontrado);
    logger_set_status(tree->parser_logger, 3, linea, 300, mensaje);
    logger_print_log(tree->parser_logger, LOG_LEVEL_ERROR, 
                     "Error sintactico en linea %u: %s", linea, mensaje);
    tree->panic_mode = true;
}

void analyze_token(ast_t *tree, Token *word) {
    if (!tree || !word) return;

    // ------------------------------------------------------------------------
    // RECUPERACIÓN DE ERRORES: MODO PÁNICO
    // Sincronización en ';' o '}' balanceando llaves para no abortar el análisis
    // ------------------------------------------------------------------------
    if (tree->panic_mode) {
        if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, "{") == 0) {
            tree->brace_depth++;
            return;
        }

        if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, ";") == 0) {
            tree->panic_mode = false;
            if (tree->brace_depth > 0) {
                tree->state = STATE_IN_BLOCK;
            } else {
                tree->state = STATE_GLOBAL;
            }
            logger_print_log(tree->parser_logger, LOG_LEVEL_INFO, 
                             "Recuperacion sintactica: sincronizado en ';' (linea %u)", word->linea);
            return;
        }

        if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, "}") == 0) {
            if (tree->brace_depth > 0) {
                tree->brace_depth--;
                restore_environment(tree);
            }
            tree->panic_mode = false;
            if (tree->brace_depth > 0) {
                tree->state = STATE_IN_BLOCK;
            } else {
                tree->state = STATE_GLOBAL;
            }
            logger_print_log(tree->parser_logger, LOG_LEVEL_INFO, 
                             "Recuperacion sintactica: sincronizado en '}' (linea %u)", word->linea);
            return;
        }

        // Descarta el token hasta encontrar un punto de sincronización
        return;
    }

    // ------------------------------------------------------------------------
    // FIN DE ARCHIVO (TOKEN_EOF)
    // ------------------------------------------------------------------------
    if (word->tipo == TOKEN_EOF) {
        if (tree->brace_depth > 0) {
            logger_set_status(tree->parser_logger, 3, word->linea, 399, "Fin de archivo inesperado: llaves sin cerrar");
            logger_print_log(tree->parser_logger, LOG_LEVEL_ERROR, 
                             "Error sintactico en linea %u: Fin de archivo inesperado, faltan %d llaves '}' por cerrar", 
                             word->linea, tree->brace_depth);
            while (tree->brace_depth > 0) {
                restore_environment(tree);
                tree->brace_depth--;
            }
        }
        return;
    }

    // ------------------------------------------------------------------------
    // MÁQUINA DE ESTADOS TOP-DOWN
    // ------------------------------------------------------------------------
    switch (tree->state) {
        case STATE_GLOBAL: {
            if (word->tipo == TOKEN_PALABRA_RESERVADA || word->tipo == TOKEN_IDENTIFICADOR) {
                tree->last_type_token = *word;
                tree->state = STATE_EXPECT_FUNC_NAME;
            } else if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, ";") == 0) {
                // Sentencia vacía a nivel global
            } else {
                parser_report_error(tree, word->linea, "tipo de dato en ambito global", word->lexema);
            }
            break;
        }

        case STATE_EXPECT_FUNC_NAME: {
            if (word->tipo == TOKEN_IDENTIFICADOR) {
                tree->last_id_token = *word;
                tree->state = STATE_EXPECT_LPAREN;
            } else if (word->tipo == TOKEN_PALABRA_RESERVADA) {
                size_t len_act = strlen(tree->last_type_token.lexema);
                if (len_act + strlen(word->lexema) + 2 < sizeof(tree->last_type_token.lexema)) {
                    strcat(tree->last_type_token.lexema, " ");
                    strcat(tree->last_type_token.lexema, word->lexema);
                }
            } else {
                parser_report_error(tree, word->linea, "nombre de funcion o identificador", word->lexema);
            }
            break;
        }

        case STATE_EXPECT_LPAREN: {
            if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, "(") == 0) {
                // Declaración de función
                if (tree->root == NULL) {
                    push_branch(tree, function_name, &tree->last_id_token);
                } else {
                    push_node(tree, function_name, &tree->last_id_token);
                }

                // Rama de parámetros
                Token param_init = *word;
                param_init.lexema[0] = '\0';
                push_branch(tree, function_parameters, &param_init);

                tree->paren_depth = 1;
                tree->state = STATE_IN_PARAMS;
            } else if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, ";") == 0) {
                // Variable global no inicializada
                push_node(tree, statement_node, &tree->last_id_token);
                tree->state = STATE_GLOBAL;
            } else if (word->tipo == TOKEN_ASIGNACION && strcmp(word->lexema, "=") == 0) {
                // Variable global inicializada
                tree->current_token_accum = tree->last_id_token;
                strncat(tree->current_token_accum.lexema, " = ", 
                        sizeof(tree->current_token_accum.lexema) - strlen(tree->current_token_accum.lexema) - 1);
                tree->paren_depth = 0;
                tree->state = STATE_IN_STATEMENT;
            } else {
                parser_report_error(tree, word->linea, "'(' tras nombre de funcion o ';' para variable", word->lexema);
            }
            break;
        }

        case STATE_IN_PARAMS: {
            if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, "(") == 0) {
                tree->paren_depth++;
            } else if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, ")") == 0) {
                tree->paren_depth--;
                if (tree->paren_depth == 0) {
                    if (tree->active != NULL && tree->active->word != NULL) {
                        if (tree->active->word->lexema[0] == '\0') {
                            strncpy(tree->active->word->lexema, "void", sizeof(tree->active->word->lexema) - 1);
                        }
                    }
                    restore_environment(tree);
                    tree->state = STATE_EXPECT_LBRACE;
                    return;
                }
            } else if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, "{") == 0) {
                // Error: faltó ')' al cerrar parámetros
                parser_report_error(tree, word->linea, "')' para cerrar los parametros de funcion", "{");
                restore_environment(tree);
                push_branch(tree, function_body, word);
                tree->brace_depth++;
                tree->state = STATE_IN_BLOCK;
                tree->panic_mode = false;
                return;
            }

            if (tree->active != NULL && tree->active->word != NULL) {
                size_t len_act = strlen(tree->active->word->lexema);
                if (len_act + strlen(word->lexema) + 2 < sizeof(tree->active->word->lexema)) {
                    if (len_act > 0 && word->lexema[0] != ',') {
                        strcat(tree->active->word->lexema, " ");
                    }
                    strcat(tree->active->word->lexema, word->lexema);
                }
            }
            break;
        }

        case STATE_EXPECT_LBRACE: {
            if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, "{") == 0) {
                push_branch(tree, function_body, word);
                tree->brace_depth++;
                tree->state = STATE_IN_BLOCK;
            } else if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, ";") == 0) {
                // Prototipo de función
                tree->state = STATE_GLOBAL;
            } else {
                parser_report_error(tree, word->linea, "'{' para iniciar cuerpo de la funcion", word->lexema);
            }
            break;
        }

        case STATE_IN_BLOCK: {
            if (word->tipo == TOKEN_PALABRA_RESERVADA) {
                if (strcmp(word->lexema, "if") == 0) {
                    tree->state = STATE_EXPECT_IF_LPAREN;
                    return;
                }
                if (strcmp(word->lexema, "while") == 0) {
                    tree->state = STATE_EXPECT_WHILE_LPAREN;
                    return;
                }
                if (strcmp(word->lexema, "else") == 0) {
                    tree->state = STATE_EXPECT_ELSE_LBRACE;
                    return;
                }
            }

            if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, "}") == 0) {
                if (tree->brace_depth > 0) {
                    tree->brace_depth--;
                    restore_environment(tree);
                }
                if (tree->brace_depth == 0) {
                    tree->state = STATE_GLOBAL;
                } else {
                    tree->state = STATE_IN_BLOCK;
                }
                return;
            }

            if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, "{") == 0) {
                // Sub-bloque anidado
                push_branch(tree, statement_node, word);
                tree->brace_depth++;
                tree->state = STATE_IN_BLOCK;
                return;
            }

            if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, ";") == 0) {
                // Sentencia vacía
                return;
            }

            // Comienzo de una sentencia ordinaria
            tree->current_token_accum = *word;
            tree->paren_depth = (strcmp(word->lexema, "(") == 0) ? 1 : 0;
            tree->state = STATE_IN_STATEMENT;
            break;
        }

        case STATE_IN_STATEMENT: {
            if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, ";") == 0 && tree->paren_depth == 0) {
                // Fin de sentencia ordinaria
                push_node(tree, statement_node, &tree->current_token_accum);
                tree->state = STATE_IN_BLOCK;
                return;
            }

            if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, "(") == 0) {
                tree->paren_depth++;
            } else if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, ")") == 0) {
                if (tree->paren_depth > 0) tree->paren_depth--;
            }

            if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, "}") == 0 && tree->paren_depth == 0) {
                // Falta ';' antes del cierre de llave
                parser_report_error(tree, word->linea, "';' al final de la sentencia", "}");
                push_node(tree, statement_node, &tree->current_token_accum);
                if (tree->brace_depth > 0) {
                    tree->brace_depth--;
                    restore_environment(tree);
                }
                tree->state = (tree->brace_depth == 0) ? STATE_GLOBAL : STATE_IN_BLOCK;
                return;
            }

            size_t len_act = strlen(tree->current_token_accum.lexema);
            size_t len_nuevo = strlen(word->lexema);
            if (len_act + len_nuevo + 2 < sizeof(tree->current_token_accum.lexema)) {
                char ultimo = (len_act > 0) ? tree->current_token_accum.lexema[len_act - 1] : '\0';
                if (ultimo != '(' && ultimo != '[' && word->lexema[0] != ')' && 
                    word->lexema[0] != ']' && word->lexema[0] != ',' && word->lexema[0] != ';') {
                    strcat(tree->current_token_accum.lexema, " ");
                }
                strcat(tree->current_token_accum.lexema, word->lexema);
            }
            break;
        }

        case STATE_EXPECT_IF_LPAREN: {
            if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, "(") == 0) {
                Token cond_tok = *word;
                cond_tok.lexema[0] = '\0';
                push_node(tree, if_condition, &cond_tok);
                tree->paren_depth = 1;
                tree->state = STATE_IN_IF_COND;
            } else {
                parser_report_error(tree, word->linea, "'(' tras 'if'", word->lexema);
            }
            break;
        }

        case STATE_IN_IF_COND: {
            if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, "(") == 0) {
                tree->paren_depth++;
            } else if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, ")") == 0) {
                tree->paren_depth--;
                if (tree->paren_depth == 0) {
                    tree->state = STATE_EXPECT_IF_LBRACE;
                    return;
                }
            } else if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, "{") == 0) {
                // Falta ')' en la condición de if
                parser_report_error(tree, word->linea, "')' para cerrar la condicion de 'if'", "{");
                tree->paren_depth = 0;
                push_branch(tree, if_body, word);
                tree->brace_depth++;
                tree->state = STATE_IN_BLOCK;
                tree->panic_mode = false;
                return;
            }

            if (tree->active != NULL && tree->active->word != NULL) {
                size_t len_act = strlen(tree->active->word->lexema);
                if (len_act + strlen(word->lexema) + 2 < sizeof(tree->active->word->lexema)) {
                    if (len_act > 0 && tree->active->word->lexema[len_act - 1] != '(') {
                        strcat(tree->active->word->lexema, " ");
                    }
                    strcat(tree->active->word->lexema, word->lexema);
                }
            }
            break;
        }

        case STATE_EXPECT_IF_LBRACE: {
            if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, "{") == 0) {
                push_branch(tree, if_body, word);
                tree->brace_depth++;
                tree->state = STATE_IN_BLOCK;
            } else {
                parser_report_error(tree, word->linea, "'{' para iniciar cuerpo de 'if'", word->lexema);
            }
            break;
        }

        case STATE_EXPECT_ELSE_LBRACE: {
            if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, "{") == 0) {
                push_branch(tree, else_body, word);
                tree->brace_depth++;
                tree->state = STATE_IN_BLOCK;
            } else if (word->tipo == TOKEN_PALABRA_RESERVADA && strcmp(word->lexema, "if") == 0) {
                tree->state = STATE_EXPECT_IF_LPAREN;
            } else {
                parser_report_error(tree, word->linea, "'{' o 'if' tras 'else'", word->lexema);
            }
            break;
        }

        case STATE_EXPECT_WHILE_LPAREN: {
            if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, "(") == 0) {
                Token cond_tok = *word;
                cond_tok.lexema[0] = '\0';
                push_node(tree, while_condition, &cond_tok);
                tree->paren_depth = 1;
                tree->state = STATE_IN_WHILE_COND;
            } else {
                parser_report_error(tree, word->linea, "'(' tras 'while'", word->lexema);
            }
            break;
        }

        case STATE_IN_WHILE_COND: {
            if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, "(") == 0) {
                tree->paren_depth++;
            } else if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, ")") == 0) {
                tree->paren_depth--;
                if (tree->paren_depth == 0) {
                    tree->state = STATE_EXPECT_WHILE_LBRACE;
                    return;
                }
            } else if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, "{") == 0) {
                // Falta ')' en la condición de while
                parser_report_error(tree, word->linea, "')' para cerrar la condicion de 'while'", "{");
                tree->paren_depth = 0;
                push_branch(tree, while_body, word);
                tree->brace_depth++;
                tree->state = STATE_IN_BLOCK;
                tree->panic_mode = false;
                return;
            }

            if (tree->active != NULL && tree->active->word != NULL) {
                size_t len_act = strlen(tree->active->word->lexema);
                if (len_act + strlen(word->lexema) + 2 < sizeof(tree->active->word->lexema)) {
                    if (len_act > 0 && tree->active->word->lexema[len_act - 1] != '(') {
                        strcat(tree->active->word->lexema, " ");
                    }
                    strcat(tree->active->word->lexema, word->lexema);
                }
            }
            break;
        }

        case STATE_EXPECT_WHILE_LBRACE: {
            if (word->tipo == TOKEN_DELIMITADOR && strcmp(word->lexema, "{") == 0) {
                push_branch(tree, while_body, word);
                tree->brace_depth++;
                tree->state = STATE_IN_BLOCK;
            } else {
                parser_report_error(tree, word->linea, "'{' para iniciar cuerpo de 'while'", word->lexema);
            }
            break;
        }

        default:
            break;
    }
}
