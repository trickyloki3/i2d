#ifndef i2d_script_h
#define i2d_script_h

#include "i2d_util.h"
#include "i2d_json.h"
#include "i2d_item.h"

enum i2d_token_type {
    I2D_HEAD,
    I2D_CURLY_OPEN,
    I2D_CURLY_CLOSE,
    I2D_PARENTHESIS_OPEN,
    I2D_PARENTHESIS_CLOSE,
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
    I2D_QUOTE
};

struct i2d_token {
    enum i2d_token_type type;
    i2d_buf * buffer;
    struct i2d_token * next;
    struct i2d_token * prev;
};

typedef struct i2d_token i2d_token;

struct i2d_lexer {
    i2d_token * list;
    i2d_token * cache;
};

typedef struct i2d_lexer i2d_lexer;

int i2d_token_init(i2d_token **, enum i2d_token_type);
void i2d_token_deit(i2d_token **);
void i2d_token_reset(i2d_token *);
int i2d_token_write(i2d_token *, void *, size_t);
int i2d_token_get_literal(i2d_token *, i2d_str *);
char i2d_token_get_last_symbol(i2d_token *);
void i2d_token_append(i2d_token *, i2d_token *);
void i2d_token_remove(i2d_token *);

int i2d_lexer_init(i2d_lexer **);
void i2d_lexer_deit(i2d_lexer **);
void i2d_lexer_reset(i2d_lexer *);
int i2d_lexer_token_init(i2d_lexer *, i2d_token **, enum i2d_token_type);
int i2d_lexer_tokenize(i2d_lexer *, i2d_str *);
void i2d_lexer_print(i2d_lexer *);

struct i2d_block {
    i2d_token * statement;
    struct i2d_block * parent;
    struct i2d_block * child;
    struct i2d_block * next;
    struct i2d_block * prev;
};

typedef struct i2d_block i2d_block;

struct i2d_parser {
    i2d_block * list;
    i2d_block * cache;
};

typedef struct i2d_parser i2d_parser;

int i2d_block_init(i2d_block **, i2d_token *, i2d_block *);
void i2d_block_deit(i2d_block **);

int i2d_parser_init(i2d_parser **);
void i2d_parser_deit(i2d_parser **);

struct i2d_script {
    i2d_json * json;
    i2d_lexer * lexer;
    i2d_parser * parser;
};

typedef struct i2d_script i2d_script;

int i2d_script_init(i2d_script **, i2d_str *);
void i2d_script_deit(i2d_script **);
int i2d_script_compile(i2d_script *, i2d_str *, i2d_str **);

#if i2d_debug
int i2d_script_test(i2d_script *, i2d_item *);
int i2d_lexer_test(void);
#endif
#endif
