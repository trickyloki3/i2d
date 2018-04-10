#ifndef i2d_script_h
#define i2d_script_h

#include "i2d_util.h"
#include "i2d_json.h"
#include "i2d_item.h"

enum i2d_token_type {
    I2D_TOKEN,
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
void i2d_token_list_deit(i2d_token **);
int i2d_token_write(i2d_token *, void *, size_t);
int i2d_token_get_literal(i2d_token *, i2d_str *);
char i2d_token_get_last_symbol(i2d_token *);
void i2d_token_append(i2d_token *, i2d_token *);
void i2d_token_remove(i2d_token *);
void i2d_token_print(i2d_token *);

int i2d_lexer_init(i2d_lexer **);
void i2d_lexer_deit(i2d_lexer **);
void i2d_lexer_reset(i2d_lexer *, i2d_token **);
int i2d_lexer_token_init(i2d_lexer *, i2d_token **, enum i2d_token_type);
int i2d_lexer_tokenize(i2d_lexer *, i2d_str *);

enum i2d_node_type {
    I2D_NODE,
    I2D_VARIABLE,
    I2D_FUNCTION,
    I2D_UNARY,
    I2D_BINARY
};

struct i2d_node {
    enum i2d_node_type type;
    i2d_token * tokens;
    struct i2d_node * left;
    struct i2d_node * right;
};

typedef struct i2d_node i2d_node;

enum i2d_statement_type {
    I2D_BONUS,
    I2D_BONUS2,
    I2D_BONUS3,
    I2D_BONUS4,
    I2D_BONUS5,
    I2D_AUTOBONUS,
    I2D_AUTOBONUS2,
    I2D_AUTOBONUS3,
    I2D_HEAL,
    I2D_PERCENTHEAL,
    I2D_ITEMHEAL,
    I2D_SKILL,
    I2D_ITEMSKILL,
    I2D_UNITSKILLUSEID,
    I2D_SC_START,
    I2D_SC_START4,
    I2D_SC_END,
    I2D_GETITEM,
    I2D_RENTITEM,
    I2D_DELITEM,
    I2D_GETRANDGROUPITEM,
    I2D_SKILLEFFECT,
    I2D_SPECIALEFFECT2,
    I2D_SETFONT,
    I2D_BUYINGSTORE,
    I2D_SEARCHSTORES,
    I2D_SET,
    I2D_INPUT,
    I2D_ANNOUNCE,
    I2D_CALLFUNC,
    I2D_END,
    I2D_WARP,
    I2D_PET,
    I2D_BPET,
    I2D_MERCENARY_CREATE,
    I2D_MERCENARY_HEAL,
    I2D_MERCENARY_SC_START,
    I2D_PRODUCE,
    I2D_COOKING,
    I2D_MAKERUNE,
    I2D_GUILDGETEXP,
    I2D_GETEXP,
    I2D_MONSTER,
    I2D_HOMEVOLUTION,
    I2D_SETOPTION,
    I2D_SETMOUNTING,
    I2D_SETFALCON,
    I2D_GETGROUPITEM,
    I2D_RESETSTATUS,
    I2D_BONUS_SCRIPT,
    I2D_PLAYBGM,
    I2D_TRANSFORM,
    I2D_SC_START2,
    I2D_PETLOOT,
    I2D_PETRECOVERY,
    I2D_PETSKILLBONUS,
    I2D_PETSKILLATTACK,
    I2D_PETSKILLATTACK2,
    I2D_PETSKILLSUPPORT,
    I2D_PETHEAL,
    I2D_FOR,
    I2D_GETMAPXY,
    I2D_SPECIALEFFECT,
    I2D_SHOWSCRIPT
};

struct i2d_statement {
    enum i2d_statement_type type;
    i2d_str * name;
    struct i2d_statement * next;
    struct i2d_statement * prev;
};

typedef struct i2d_statement i2d_statement;

enum i2d_block_type {
    I2D_BLOCK,
    I2D_EXPRESSION,
    I2D_STATEMENT,
    I2D_IF,
    I2D_ELSE
};

struct i2d_block {
    enum i2d_block_type type;
    i2d_token * tokens;
    i2d_node * nodes;
    i2d_block_data * block_data;
    struct i2d_block * parent;
    struct i2d_block * child;
    struct i2d_block * next;
    struct i2d_block * prev;
};

typedef struct i2d_block i2d_block;

struct i2d_parser {
    i2d_block * block_list;
    i2d_block * block_cache;
    i2d_node * node_cache;
};

typedef struct i2d_parser i2d_parser;

int i2d_node_init(i2d_node **, enum i2d_node_type, i2d_token *);
void i2d_node_deit(i2d_node **);
void i2d_node_append(i2d_node *, i2d_node *);
void i2d_node_remove(i2d_node *);
void i2d_node_print(i2d_node *, int);

int i2d_statement_init(i2d_statement **, enum i2d_statement_type);
void i2d_statement_deit(i2d_statement **);
void i2d_statement_append(i2d_statement *, i2d_statement *);
void i2d_statement_remove(i2d_statement *);

int i2d_block_init(i2d_block **, enum i2d_block_type, i2d_token *, i2d_block *);
void i2d_block_deit(i2d_block **);
void i2d_block_list_deit(i2d_block **);
void i2d_block_append(i2d_block *, i2d_block *);
void i2d_block_remove(i2d_block *);
void i2d_block_print(i2d_block *, int);
void i2d_block_list_print(i2d_block *, int);
int i2d_block_token_augment(i2d_block *, i2d_lexer *);
int i2d_block_data_lookup(i2d_block *, i2d_lexer *, i2d_json *);

int i2d_parser_init(i2d_parser **);
void i2d_parser_deit(i2d_parser **);
void i2d_parser_reset(i2d_parser *, i2d_lexer *, i2d_block **);
void i2d_parser_node_reset(i2d_parser *, i2d_lexer *, i2d_node **);
int i2d_parser_block_init(i2d_parser *, i2d_block **, enum i2d_block_type, i2d_token *, i2d_block *);
int i2d_parser_node_init(i2d_parser *, i2d_node **, enum i2d_node_type, i2d_token *);
int i2d_parser_analysis(i2d_parser *, i2d_lexer *, i2d_json *);
int i2d_parser_analysis_recursive(i2d_parser *, i2d_lexer *, i2d_json *, i2d_block *, i2d_block **, i2d_token *);
int i2d_parser_statement_recursive(i2d_parser *, i2d_lexer *, i2d_block *, i2d_block **, i2d_token *);
int i2d_parser_expression_recursive(i2d_parser *, i2d_lexer *, i2d_token *, i2d_node **);

struct i2d_translator {

};

typedef struct i2d_translator i2d_translator;

int i2d_translator_init(i2d_translator **);
void i2d_translator_deit(i2d_translator **);
int i2d_translator_translate(i2d_translator *, i2d_block *);
int i2d_translator_statement(i2d_translator *, i2d_block *);
int i2d_translator_expression(i2d_translator *, i2d_block *);

struct i2d_script {
    i2d_json * json;
    i2d_lexer * lexer;
    i2d_parser * parser;
    i2d_translator * translator;
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
