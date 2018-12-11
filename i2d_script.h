#ifndef i2d_script_h
#define i2d_script_h

#include "i2d_util.h"
#include "i2d_range.h"
#include "i2d_logic.h"
#include "i2d_constant.h"
#include "i2d_db.h"
#include "i2d_json.h"
#include "i2d_config.h"

enum i2d_token_type {
    I2D_TOKEN,
    I2D_CURLY_OPEN,
    I2D_CURLY_CLOSE,
    I2D_PARENTHESIS_OPEN,
    I2D_PARENTHESIS_CLOSE,
    I2D_BRACKET_OPEN,
    I2D_BRACKET_CLOSE,
    I2D_COMMA,
    I2D_SEMICOLON,
    I2D_LITERAL,
    I2D_TEMPORARY_CHARACTER,
    I2D_PERMANENT_GLOBAL,
    I2D_TEMPORARY_GLOBAL,
    I2D_TEMPORARY_NPC,
    I2D_TEMPORARY_SCOPE,
    I2D_TEMPORARY_INSTANCE,
    I2D_PERMANENT_ACCOUNT_LOCAL,
    I2D_PERMANENT_ACCOUNT_GLOBAL,
    I2D_ADD,
    I2D_SUBTRACT,
    I2D_ADD_UNARY,
    I2D_SUBTRACT_UNARY,
    I2D_MULTIPLY,
    I2D_DIVIDE,
    I2D_MODULUS,
    I2D_ADD_ASSIGN,
    I2D_SUBTRACT_ASSIGN,
    I2D_MULTIPLY_ASSIGN,
    I2D_DIVIDE_ASSIGN,
    I2D_MODULUS_ASSIGN,
    I2D_GREATER,
    I2D_LESS,
    I2D_NOT,
    I2D_EQUAL,
    I2D_GREATER_EQUAL,
    I2D_LESS_EQUAL,
    I2D_NOT_EQUAL,
    I2D_RIGHT_SHIFT,
    I2D_LEFT_SHIFT,
    I2D_BIT_AND,
    I2D_BIT_OR,
    I2D_BIT_XOR,
    I2D_BIT_NOT,
    I2D_RIGHT_SHIFT_ASSIGN,
    I2D_LEFT_SHIFT_ASSIGN,
    I2D_BIT_AND_ASSIGN,
    I2D_BIT_OR_ASSIGN,
    I2D_BIT_XOR_ASSIGN,
    I2D_AND,
    I2D_OR,
    I2D_CONDITIONAL,
    I2D_COLON,
    I2D_UNIQUE_NAME,
    I2D_ASSIGN,
    I2D_LINE_COMMENT,
    I2D_BLOCK_COMMENT,
    I2D_QUOTE,
    I2D_POSITION,
    I2D_INCREMENT,
    I2D_DECREMENT
};

struct i2d_token {
    enum i2d_token_type type;
    i2d_buffer buffer;
    struct i2d_token * next;
    struct i2d_token * prev;
};

typedef struct i2d_token i2d_token;

int i2d_token_init(i2d_token **, enum i2d_token_type);
void i2d_token_deit(i2d_token **);
void i2d_token_list_deit(i2d_token **);
void i2d_token_append(i2d_token *, i2d_token *);
void i2d_token_remove(i2d_token *);
void i2d_token_print(i2d_token *);
char i2d_token_getc(i2d_token *);
int i2d_token_putc(i2d_token *, char);
int i2d_token_get_string(i2d_token *, i2d_string *);
int i2d_token_set_string(i2d_token *, i2d_string *);
int i2d_token_get_constant(i2d_token *, long *);


struct i2d_format {
    i2d_token * tokens;
};

typedef struct i2d_format i2d_format;

int i2d_format_create(i2d_format *, const char *, size_t);
int i2d_format_create_json(i2d_format *, json_t *);
void i2d_format_destroy(i2d_format *);
int i2d_format_tokenize(const char *, size_t, i2d_token **);
int i2d_format_write(i2d_format *, i2d_string_stack *, i2d_buffer *);

struct i2d_data {
    long constant;
    i2d_string name;
    i2d_range range;
    i2d_format description;
    i2d_string handler;
    i2d_string_stack argument_type;
    i2d_string_stack argument_default;
    struct {
        long * list;
        size_t size;
    } argument_order;
    long required;
    long optional;
    i2d_string positive;
    i2d_string negative;
    i2d_string zero;
    int empty_description_on_zero;  /* i2d_data_handler_evaluate */
    int empty_description_on_empty_string; /* i2d_script_statement_evaluate */
    int dump_stack_instead_of_description; /* i2d_script_statement_evaluate */
};

typedef struct i2d_data i2d_data;

int i2d_data_create(i2d_data *, const char *, json_t *, i2d_constant_db *);
void i2d_data_destroy(i2d_data *);

enum i2d_data_map_type {
    data_map_by_constant = 1,
    data_map_by_name
};

struct i2d_data_map {
    i2d_rbt * map;
    i2d_data * list;
    size_t size;
};

typedef struct i2d_data_map i2d_data_map;

int i2d_data_map_init(i2d_data_map **, enum i2d_data_map_type, json_t *, i2d_constant_db *);
void i2d_data_map_deit(i2d_data_map **);
int i2d_data_map_get(i2d_data_map *, void *, i2d_data **);

struct i2d_lexer {
    i2d_token * cache;
};

typedef struct i2d_lexer i2d_lexer;

int i2d_lexer_init(i2d_lexer **);
void i2d_lexer_deit(i2d_lexer **);
void i2d_lexer_reset(i2d_lexer *, i2d_token **);
int i2d_lexer_token_init(i2d_lexer *, i2d_token **, enum i2d_token_type);
int i2d_lexer_tokenize(i2d_lexer *, i2d_string *, i2d_token **);

enum i2d_node_type {
    I2D_NODE,
    I2D_NUMBER,
    I2D_VARIABLE,
    I2D_FUNCTION,
    I2D_INDEX,
    I2D_UNARY,
    I2D_BINARY
};

struct i2d_node {
    enum i2d_node_type type;
    i2d_constant * constant;
    i2d_range range;
    i2d_logic * logic;
    i2d_token * tokens;
    struct i2d_node * index;
    struct i2d_node * left;
    struct i2d_node * right;
};

typedef struct i2d_node i2d_node;

int i2d_rbt_cmp_node(const void *, const void *);
int i2d_node_init(i2d_node **, enum i2d_node_type, i2d_token *);
void i2d_node_deit(i2d_node **);
void i2d_node_list_deit(i2d_node **);
void i2d_node_append(i2d_node *, i2d_node *);
void i2d_node_remove(i2d_node *);
void i2d_node_print(i2d_node *, int);
int i2d_node_copy(i2d_node *, i2d_node *);
int i2d_node_get_arguments(i2d_node *, i2d_node **, size_t, size_t);
int i2d_node_get_constant(i2d_node *, long *);
int i2d_node_set_constant(i2d_node *, i2d_constant *);
int i2d_node_get_string(i2d_node *, i2d_string *);
int i2d_node_set_string(i2d_node *, i2d_string *);
int i2d_node_get_predicate(i2d_node *, i2d_string *);
int i2d_node_get_predicate_all(i2d_node *, i2d_string_stack *);
int i2d_node_get_predicate_all_recursive(i2d_node *, i2d_string_stack *);
int i2d_node_is_conditional(i2d_node *);
int i2d_node_is_colon(i2d_node *);

enum i2d_block_type {
    I2D_BLOCK,
    I2D_STATEMENT,
    I2D_IF,
    I2D_ELSE,
    I2D_FOR
};

struct i2d_block {
    enum i2d_block_type type;
    i2d_buffer buffer;
    i2d_token * tokens;
    i2d_node * nodes;
    i2d_logic * logics;
    i2d_data * statement;
    struct i2d_block * parent;
    struct i2d_block * child;
    struct i2d_block * next;
    struct i2d_block * prev;
};

typedef struct i2d_block i2d_block;

int i2d_block_init(i2d_block **, enum i2d_block_type, i2d_token *, i2d_block *);
void i2d_block_deit(i2d_block **);
void i2d_block_list_deit(i2d_block **);
void i2d_block_append(i2d_block *, i2d_block *);
void i2d_block_remove(i2d_block *);
void i2d_block_print(i2d_block *, int);
void i2d_block_list_print(i2d_block *, int);

struct i2d_parser {
    i2d_block * block_cache;
    i2d_node * node_cache;
};

typedef struct i2d_parser i2d_parser;

int i2d_parser_init(i2d_parser **);
void i2d_parser_deit(i2d_parser **);
void i2d_parser_reset(i2d_parser *, i2d_lexer *, i2d_block **);
void i2d_parser_node_reset(i2d_parser *, i2d_lexer *, i2d_node **);
int i2d_parser_block_init(i2d_parser *, i2d_block **, enum i2d_block_type, i2d_token *, i2d_block *);
int i2d_parser_node_init(i2d_parser *, i2d_node **, enum i2d_node_type, i2d_token *);
int i2d_parser_get_statement(i2d_parser *, i2d_lexer *, i2d_data_map *, i2d_block *);
int i2d_parser_analysis(i2d_parser *, i2d_lexer *, i2d_data_map *, i2d_token *, i2d_block **);
int i2d_parser_analysis_recursive(i2d_parser *, i2d_lexer *, i2d_data_map *, i2d_block *, i2d_block **, i2d_token *);
int i2d_parser_expression_recursive(i2d_parser *, i2d_lexer *, i2d_token *, i2d_node **);

enum {
    I2D_FLAG_NONE = 0x0,
    I2D_FLAG_CONDITIONAL = 0x1
};

struct i2d_script {
    i2d_db * db;
    i2d_json * json;
    i2d_lexer * lexer;
    i2d_parser * parser;
    i2d_constant_db * constant_db;
    i2d_value_map * getiteminfo;
    i2d_value_map * strcharinfo;
    i2d_value_map * weapons;
    i2d_value_map * ammos;
    i2d_value_map * skill_flags;
    i2d_value_map * searchstore_effect;
    i2d_value_map * bonus_script_flag;
    i2d_data_map * bonus;
    i2d_data_map * bonus2;
    i2d_data_map * bonus3;
    i2d_data_map * bonus4;
    i2d_data_map * bonus5;
    i2d_data_map * sc_start;
    i2d_data_map * sc_start2;
    i2d_data_map * sc_start4;
    i2d_data_map * functions;
    i2d_data_map * arguments;
    i2d_data_map * statements;
    i2d_buffer_cache * buffer_cache;
    i2d_string_stack_cache * stack_cache;
    i2d_rbt * function_handlers;
    i2d_rbt * argument_handlers;
    i2d_rbt * statement_handlers;
    void * handlers;
};

typedef struct i2d_script i2d_script;

int i2d_script_init(i2d_script **, i2d_config *);
void i2d_script_deit(i2d_script **);
int i2d_script_compile(i2d_script *, i2d_string *, i2d_string *, i2d_rbt *);
int i2d_script_compile_node(i2d_script *, const char *, i2d_node **, i2d_rbt *);
int i2d_script_translate(i2d_script *, i2d_block *, i2d_rbt *, i2d_logic *);
int i2d_script_generate(i2d_script *, i2d_block *, i2d_buffer *);
int i2d_script_generate_or(i2d_script *, i2d_logic *, i2d_buffer *);
int i2d_script_generate_and(i2d_script *, i2d_logic *, i2d_buffer *);
int i2d_script_generate_var(i2d_script *, i2d_logic *, i2d_buffer *);
int i2d_script_statement(i2d_script *, i2d_block *, i2d_rbt *, i2d_logic *);
int i2d_script_statement_ignore(i2d_script *, i2d_block *, i2d_rbt *, i2d_data *);
int i2d_script_statement_set(i2d_script *, i2d_block *, i2d_rbt *, i2d_data *);
int i2d_script_statement_generic(i2d_script *, i2d_block *, i2d_rbt *, i2d_data *);
int i2d_script_statement_evaluate(i2d_script *, i2d_rbt *, i2d_node **, i2d_data *, i2d_buffer *);
int i2d_script_expression(i2d_script *, i2d_node *, int, i2d_rbt *, i2d_logic *);
int i2d_script_expression_conditional(i2d_script *, i2d_node *, i2d_logic *, i2d_logic **);
int i2d_script_expression_variable_predicate(i2d_script *, i2d_node *, i2d_node *);
int i2d_script_expression_variable(i2d_script *, i2d_node *, i2d_rbt *, i2d_logic *);
int i2d_script_expression_function(i2d_script *, i2d_node *, i2d_rbt *);
int i2d_script_expression_unary(i2d_script *, i2d_node *, int);
int i2d_script_expression_binary_assign(i2d_node *, int, i2d_rbt *);
int i2d_script_expression_binary_relational(i2d_node *, int, int);
int i2d_script_expression_binary_logical(i2d_node *, int, int);
int i2d_script_expression_binary(i2d_script *, i2d_node *, int, i2d_rbt *);

struct i2d_item_script {
    i2d_string script;
    i2d_string onequip;
    i2d_string onunequip;
    i2d_string combo;
};

typedef struct i2d_item_script i2d_item_script;

void i2d_item_script_destroy(i2d_item_script *);

int i2d_script_compile_item(i2d_script *, i2d_item *, i2d_item_script *);
int i2d_script_compile_item_combo(i2d_script *, i2d_item *, i2d_string *);
#endif
