#ifndef AST_H
#define AST_H

#include <stdint.h>
#include <stdbool.h>
#include "stack.h"
#include "logger.h"

// 1. Enumeración de tipos de nodo del AST según las especificaciones del profesor
typedef enum {
    else_body = 0,
    function_body,
    function_name,
    function_parameters,
    if_body,
    if_condition,
    while_condition,
    while_body,
    statement_node
} ast_nodes;

// 2. Estructura del nodo del AST según las especificaciones del profesor
typedef struct ast_node {
    struct ast_node *branch; // Nodo hijo / rama anidada
    struct ast_node *next;   // Siguiente nodo hermano en el mismo bloque
    ast_nodes type;          // Tipo de nodo
    Token *word;             // Puntero al token asociado
} ast_node;

// Pila auxiliar de nodos AST para guardar contexto al entrar a ramas/bloques
typedef struct AstContextNode {
    ast_node *node;
    struct AstContextNode *siguiente;
} AstContextNode;

typedef struct {
    AstContextNode *tope;
    int tamano;
} AstContextStack;

// Estados del parser descendente predictivo (máquina de estados)
typedef enum {
    STATE_GLOBAL = 0,
    STATE_EXPECT_FUNC_NAME,
    STATE_EXPECT_LPAREN,
    STATE_IN_PARAMS,
    STATE_EXPECT_LBRACE,
    STATE_IN_BLOCK,
    STATE_EXPECT_IF_LPAREN,
    STATE_IN_IF_COND,
    STATE_EXPECT_IF_LBRACE,
    STATE_EXPECT_ELSE_LBRACE,
    STATE_EXPECT_WHILE_LPAREN,
    STATE_IN_WHILE_COND,
    STATE_EXPECT_WHILE_LBRACE,
    STATE_IN_STATEMENT
} ParserState;

// 3. Estructura controladora del AST (adaptando la clase ast de C++ a C)
typedef struct ast_t {
    ast_node *root;                 // Raíz del árbol
    ast_node *active;               // Nodo activo actual
    ast_node *branch;               // Último nodo de rama abierta
    AstContextStack local_stack;    // Pila para guardar contexto en bloques { y restaurar en }
    unsigned int number_nodes;      // Contador de nodos creados
    logger_t *parser_logger;        // Referencia al logger para reportar errores sintácticos

    // Atributos internos del parser
    ParserState state;
    bool panic_mode;
    int paren_depth;
    int brace_depth;
    Token last_type_token;
    Token last_id_token;
    Token current_token_accum;
} ast_t;

// Funciones públicas
void ast_init(ast_t *tree, logger_t *log);
void analyze_token(ast_t *tree, Token *word);
void ast_free(ast_t *tree);
void print_tree(const ast_t *tree);

// Funciones internas del AST especificadas por el profesor
ast_node *make_new_node(ast_nodes type, Token *word);
void push_branch(ast_t *tree, ast_nodes type, Token *word);
void push_node(ast_t *tree, ast_nodes type, Token *word);
void push_word(ast_t *tree, Token *word);
void restore_environment(ast_t *tree);
void print_branch(ast_node *branch, int depth);
void print_node(ast_node *current, int depth);

// Funciones auxiliares para la pila de contexto
void ast_context_push(AstContextStack *s, ast_node *node);
ast_node *ast_context_pop(AstContextStack *s);
void ast_context_free(AstContextStack *s);
const char *ast_node_type_name(ast_nodes type);

#endif // AST_H
