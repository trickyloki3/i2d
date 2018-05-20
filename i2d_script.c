#include "i2d_script.h"

struct i2d_handler {
    i2d_string name;
    int (*handler) (i2d_script *, i2d_node *, i2d_context *);
};

typedef struct i2d_handler i2d_handler;

static int i2d_handler_general(i2d_script *, i2d_node *, i2d_context *);
static int i2d_handler_readparam(i2d_script *, i2d_node *, i2d_context *);
static int i2d_handler_getskilllv(i2d_script *, i2d_node *, i2d_context *);
static int i2d_handler_isequipped(i2d_script *, i2d_node *, i2d_context *);
static int i2d_handler_countitem(i2d_script *, i2d_node *, i2d_context *);
static int i2d_handler_gettime(i2d_script *, i2d_node *, i2d_context *);
static int i2d_handler_strcharinfo(i2d_script *, i2d_node *, i2d_context *);
static int i2d_handler_getequipid(i2d_script *, i2d_node *, i2d_context *);
static int i2d_handler_getiteminfo(i2d_script *, i2d_node *, i2d_context *);
static int i2d_handler_getmapflag(i2d_script *, i2d_node *, i2d_context *);

i2d_handler function_list[] = {
    { {"getrefine", 9}, i2d_handler_general },
    { {"readparam", 9}, i2d_handler_readparam },
    { {"getskilllv", 10}, i2d_handler_getskilllv },
    { {"isequipped", 10}, i2d_handler_isequipped },
    { {"getpartnerid", 12}, i2d_handler_general },
    { {"checkmadogear", 12}, i2d_handler_general },
    { {"eaclass", 7}, i2d_handler_general },
    { {"countitem", 9}, i2d_handler_countitem },
    { {"gettime", 7}, i2d_handler_gettime },
    { {"strcharinfo", 11}, i2d_handler_strcharinfo },
    { {"getequipid", 10}, i2d_handler_getequipid },
    { {"getiteminfo", 11}, i2d_handler_getiteminfo },
    { {"getmapflag", 10}, i2d_handler_getmapflag }
};

static int i2d_bonus_handler_expression(i2d_script *, i2d_node *, i2d_context *);
static int i2d_bonus_handler_time(i2d_script *, i2d_node *, i2d_context *);
static int i2d_bonus_handler_regen(i2d_script *, i2d_node *, i2d_context *);
static int i2d_bonus_handler_splash(i2d_script *, i2d_node *, i2d_context *);
static int i2d_bonus_handler_elements(i2d_script *, i2d_node *, i2d_context *);
static int i2d_bonus_handler_races(i2d_script *, i2d_node *, i2d_context *);
static int i2d_bonus_handler_classes(i2d_script *, i2d_node *, i2d_context *);
static int i2d_bonus_handler_integer(i2d_script *, i2d_node *, i2d_context *);
static int i2d_bonus_handler_percent(i2d_script *, i2d_node *, i2d_context *);
static int i2d_bonus_handler_percent_invert(i2d_script *, i2d_node *, i2d_context *);
static int i2d_bonus_handler_percent100(i2d_script *, i2d_node *, i2d_context *);
static int i2d_bonus_handler_ignore(i2d_script *, i2d_node *, i2d_context *);
static int i2d_bonus_handler_sizes(i2d_script *, i2d_node *, i2d_context *);

i2d_handler bonus_list[] = {
    { {"time", 4}, i2d_bonus_handler_time },
    { {"regen", 5}, i2d_bonus_handler_regen },
    { {"splash", 6}, i2d_bonus_handler_splash },
    { {"elements", 8}, i2d_bonus_handler_elements },
    { {"races", 4}, i2d_bonus_handler_races },
    { {"classes", 7}, i2d_bonus_handler_classes },
    { {"integer", 7}, i2d_bonus_handler_integer },
    { {"percent", 7}, i2d_bonus_handler_percent },
    { {"percent_invert", 14}, i2d_bonus_handler_percent_invert },
    { {"percent100", 14}, i2d_bonus_handler_percent100 },
    { {"ignore", 6}, i2d_bonus_handler_ignore },
    { {"sizes", 4}, i2d_bonus_handler_sizes }
};

const char * i2d_token_string[] = {
    "token",
    "{",
    "}",
    "(",
    ")",
    ",",
    ";",
    "id",
    "@",
    "$",
    "$@",
    ".",
    ".@",
    "'",
    "#",
    "##",
    "+",
    "-",
    "+",
    "-",
    "*",
    "/",
    "%",
    "+=",
    "-=",
    "*=",
    "/=",
    "%=",
    ">",
    "<",
    "!",
    "==",
    ">=",
    "<=",
    "!=",
    ">>",
    "<<",
    "&",
    "|",
    "^",
    "~",
    ">>=",
    "<<=",
    "&=",
    "|=",
    "^=",
    "&&",
    "||",
    "?",
    ":",
    "::",
    "=",
    "//",
    "/**/",
    "\"\"",
    "pos"
};

int i2d_token_precedence[] = {
    0,
    0,
    0,
    0,
    0,
    15, /* , */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    4, /* + binary */
    4, /* - binary */
    2, /* +  unary*/
    2, /* -  unary*/
    3, /* * */
    3, /* / */
    3, /* % */
    14, /* += */
    14, /* -= */
    14, /* *= */
    14, /* /= */
    14, /* %= */
    6, /* > */
    6, /* < */
    2, /* ! */
    7, /* == */
    6, /* >= */
    6, /* <= */
    7, /* != */
    5, /* >> */
    5, /* << */
    8, /* & */
    10, /* | */
    9, /* ^ */
    2, /* ~ */
    14, /* >>= */
    14, /* <<= */
    14, /* &= */
    14, /* |= */
    14, /* ^= */
    11, /* && */
    12, /* || */
    13, /* ? */
    13, /* : */
    0,
    14, /* = */
    0,
    0,
    0
};

int i2d_token_init(i2d_token ** result, enum i2d_token_type type) {
    int status = I2D_OK;
    i2d_token * object = NULL;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_buffer_create(&object->buffer, 256)) {
                status = i2d_panic("failed to create buffer object");
            } else {
                object->type = type;
                object->next = object;
                object->prev = object;
            }

            if(status) {
                i2d_token_deit(&object);
            } else {
                *result = object;
            }
        }
    }

    return status;
}

void i2d_token_deit(i2d_token ** result) {
    i2d_token * object;

    object = *result;
    i2d_buffer_destroy(&object->buffer);
    i2d_free(object);
    *result = NULL;
}

void i2d_token_list_deit(i2d_token ** result) {
    i2d_token * object;
    i2d_token * token;

    object = *result;
    if(object) {
        while(object != object->next) {
            token = object->next;
            i2d_token_remove(token);
            i2d_token_deit(&token);
        }
        i2d_token_deit(&object);
    }
    *result = NULL;
}

void i2d_token_append(i2d_token * x, i2d_token * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_token_remove(i2d_token * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

void i2d_token_print(i2d_token * token) {
    i2d_token * iterator;
    i2d_string string;

    iterator = token;
    do {
        if(!i2d_token_get_string(iterator, &string)) {
            fprintf(stdout, "%s(%s) ", i2d_token_string[iterator->type], string.string);
        } else {
            fprintf(stdout, "%s ", i2d_token_string[iterator->type]);
        }
        iterator = iterator->next;
    } while(iterator != token);
}

int i2d_token_putc(i2d_token * token, char character) {
    return i2d_buffer_putc(&token->buffer, character);
}

char i2d_token_getc(i2d_token * token) {
    return token->buffer.offset ? token->buffer.buffer[token->buffer.offset - 1] : 0;
}

int i2d_token_get_string(i2d_token * token, i2d_string * result) {
    int status = I2D_OK;

    if(I2D_LITERAL != token->type && I2D_POSITION != token->type) {
        status = I2D_FAIL;
    } else {
        i2d_buffer_get(&token->buffer, &result->string, &result->length);
    }

    return status;
}

int i2d_token_set_string(i2d_token * token, i2d_string * string) {
    int status = I2D_OK;

    i2d_buffer_clear(&token->buffer);
    if(i2d_buffer_printf(&token->buffer, "%s", string->string))
        status = i2d_panic("failed to copy string");

    return status;
}

int i2d_token_get_constant(i2d_token * token, long * result) {
    int status = I2D_OK;
    i2d_string string;
    int base;

    if(i2d_token_get_string(token, &string)) {
        status = i2d_panic("failed to get token string");
    } else {
        if(!strncmp(string.string, "0x", 2) && string.length > 2) {
            base = 16;
        } else if(string.string[0] == '0' && string.length > 1) {
            base = 8;
        } else {
            base = 10;
        }

        status = i2d_strtol(result, string.string, string.length, base);
    }

    return status;
}

int i2d_lexer_init(i2d_lexer ** result) {
    int status = I2D_OK;
    i2d_lexer * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_token_init(&object->cache, I2D_TOKEN))
                status = i2d_panic("failed to create token object");

            if(status)
                i2d_lexer_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_lexer_deit(i2d_lexer ** result) {
    i2d_lexer * object;

    object = *result;
    i2d_deit(object->cache, i2d_token_list_deit);
    i2d_free(object);
    *result = NULL;
}

void i2d_lexer_reset(i2d_lexer * lexer, i2d_token ** result) {
    i2d_token * token;

    token = *result;
    do {
        i2d_buffer_clear(&token->buffer);
        token->type = I2D_TOKEN;
        token = token->next;
    } while(token != *result);
    i2d_token_append(token, lexer->cache);
    *result = NULL;
}

int i2d_lexer_token_init(i2d_lexer * lexer, i2d_token ** result, enum i2d_token_type type) {
    int status = I2D_OK;
    i2d_token * token;

    if(lexer->cache != lexer->cache->next) {
        token = lexer->cache->next;
        i2d_token_remove(token);
        token->type = type;
        *result = token;
    } else {
        status = i2d_token_init(result, type);
    }

    return status;
}

int i2d_lexer_tokenize(i2d_lexer * lexer, i2d_string * script, i2d_token ** result) {
    int status = I2D_OK;
    size_t i;
    char symbol;
    i2d_token * root = NULL;
    i2d_token * token = NULL;
    i2d_token * state = NULL;

    if(i2d_lexer_token_init(lexer, &root, I2D_TOKEN)) {
        status = i2d_panic("failed to create token object");
    } else {
        for(i = 0; i < script->length && !status; i++) {
            symbol = script->string[i];
            token = NULL;

            if(state) {
                if(I2D_QUOTE == state->type) {
                    if('"' == symbol && '\\' != i2d_token_getc(state)) {
                        state->type = I2D_LITERAL;
                    } else {
                        status = i2d_token_putc(state, symbol);
                    }
                    continue;
                } else if(I2D_LINE_COMMENT == state->type) {
                    if('\n' == symbol) {
                        i2d_token_remove(state);
                        i2d_token_deit(&state);
                    }
                    continue;
                } else if(I2D_BLOCK_COMMENT == state->type) {
                    if('/' != symbol) {
                        status = i2d_token_putc(state, symbol);
                    } else if('*' == i2d_token_getc(state)) {
                        i2d_token_remove(state);
                        i2d_token_deit(&state);
                    }
                    continue;
                }
            }

            switch(symbol) {
                case  '{': status = i2d_lexer_token_init(lexer, &token, I2D_CURLY_OPEN); break;
                case  '}': status = i2d_lexer_token_init(lexer, &token, I2D_CURLY_CLOSE); break;
                case  '(': status = i2d_lexer_token_init(lexer, &token, I2D_PARENTHESIS_OPEN); break;
                case  ')': status = i2d_lexer_token_init(lexer, &token, I2D_PARENTHESIS_CLOSE); break;
                case  ',': status = i2d_lexer_token_init(lexer, &token, I2D_COMMA); break;
                case  ';': status = i2d_lexer_token_init(lexer, &token, I2D_SEMICOLON); break;
                case  '$':
                    if(state && I2D_LITERAL == state->type && '$' != i2d_token_getc(state)) {
                        status = i2d_token_putc(state, '$');
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_PERMANENT_GLOBAL);
                    }
                    break;
                case  '.': status = i2d_lexer_token_init(lexer, &token, I2D_TEMPORARY_NPC); break;
                case '\'': status = i2d_lexer_token_init(lexer, &token, I2D_TEMPORARY_INSTANCE); break;
                case  '@':
                    if(state && I2D_PERMANENT_GLOBAL == state->type) {
                        state->type = I2D_TEMPORARY_GLOBAL;
                    } else if(state && I2D_TEMPORARY_NPC == state->type) {
                        state->type = I2D_TEMPORARY_SCOPE;
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_TEMPORARY_CHARACTER); break;
                    }
                    break;
                case  '#':
                    if(state && I2D_PERMANENT_ACCOUNT_LOCAL == state->type) {
                        state->type = I2D_PERMANENT_ACCOUNT_GLOBAL;
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_PERMANENT_ACCOUNT_LOCAL);
                    }
                    break;
                case  '+': status = i2d_lexer_token_init(lexer, &token, I2D_ADD); break;
                case  '-': status = i2d_lexer_token_init(lexer, &token, I2D_SUBTRACT); break;
                case  '*':
                    if(state && I2D_DIVIDE == state->type) {
                        state->type = I2D_BLOCK_COMMENT;
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_MULTIPLY);
                    }
                    break;
                case  '/':
                    if(state && I2D_DIVIDE == state->type) {
                        state->type = I2D_LINE_COMMENT;
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_DIVIDE);
                    }
                    break;
                case  '%': status = i2d_lexer_token_init(lexer, &token, I2D_MODULUS); break;
                case  '>':
                    if(state && I2D_GREATER == state->type) {
                        state->type = I2D_RIGHT_SHIFT;
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_GREATER);
                    }
                    break;
                case  '<':
                    if(state && I2D_LESS == state->type) {
                        state->type = I2D_LEFT_SHIFT;
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_LESS);
                    }
                    break;
                case  '!': status = i2d_lexer_token_init(lexer, &token, I2D_NOT); break;
                case  '&':
                    if(state && I2D_BIT_AND == state->type) {
                        state->type = I2D_AND;
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_BIT_AND);
                    }
                    break;
                case  '|':
                    if(state && I2D_BIT_OR == state->type) {
                        state->type = I2D_OR;
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_BIT_OR);
                    }
                    break;
                case  '^': status = i2d_lexer_token_init(lexer, &token, I2D_BIT_XOR); break;
                case  '~': status = i2d_lexer_token_init(lexer, &token, I2D_BIT_NOT); break;
                case  '=':
                    if(state) {
                        switch(state->type) {
                            case I2D_ADD: state->type = I2D_ADD_ASSIGN; break;
                            case I2D_SUBTRACT: state->type = I2D_SUBTRACT_ASSIGN; break;
                            case I2D_MULTIPLY: state->type = I2D_MULTIPLY_ASSIGN; break;
                            case I2D_DIVIDE: state->type = I2D_DIVIDE_ASSIGN; break;
                            case I2D_MODULUS: state->type = I2D_MODULUS_ASSIGN; break;
                            case I2D_ASSIGN: state->type = I2D_EQUAL; break;
                            case I2D_GREATER: state->type = I2D_GREATER_EQUAL; break;
                            case I2D_LESS: state->type = I2D_LESS_EQUAL; break;
                            case I2D_NOT: state->type = I2D_NOT_EQUAL; break;
                            case I2D_RIGHT_SHIFT: state->type = I2D_RIGHT_SHIFT_ASSIGN; break;
                            case I2D_LEFT_SHIFT: state->type = I2D_LEFT_SHIFT_ASSIGN; break;
                            case I2D_BIT_AND: state->type = I2D_BIT_AND_ASSIGN; break;
                            case I2D_BIT_OR: state->type = I2D_BIT_OR_ASSIGN; break;
                            case I2D_BIT_XOR: state->type = I2D_BIT_XOR_ASSIGN; break;
                            default: status = i2d_lexer_token_init(lexer, &token, I2D_ASSIGN); break;
                        }
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_ASSIGN);
                    }
                    break;
                case  '?': status = i2d_lexer_token_init(lexer, &token, I2D_CONDITIONAL); break;
                case  ':':
                    if(state && I2D_COLON == state->type) {
                        state->type = I2D_UNIQUE_NAME;
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_COLON);
                    }
                    break;
                case  '"': status = i2d_lexer_token_init(lexer, &token, I2D_QUOTE); break;
                case '\\': continue;
                default:
                    if('_' == symbol || isalpha(symbol) || isdigit(symbol)) {
                        if(state) {
                            switch(state->type) {
                                case I2D_LITERAL:
                                    status = i2d_token_putc(state, symbol);
                                    break;
                                case I2D_TEMPORARY_CHARACTER:
                                    status = i2d_token_putc(state, '@') ||
                                             i2d_token_putc(state, symbol);
                                    state->type = I2D_LITERAL;
                                    break;
                                case I2D_PERMANENT_GLOBAL:
                                    status = i2d_token_putc(state, '$') ||
                                             i2d_token_putc(state, symbol);
                                    state->type = I2D_LITERAL;
                                    break;
                                case I2D_TEMPORARY_GLOBAL:
                                    status = i2d_token_putc(state, '$') ||
                                             i2d_token_putc(state, '@') ||
                                             i2d_token_putc(state, symbol);
                                    state->type = I2D_LITERAL;
                                    break;
                                case I2D_TEMPORARY_NPC:
                                    status = i2d_token_putc(state, '.') ||
                                             i2d_token_putc(state, symbol);
                                    state->type = I2D_LITERAL;
                                    break;
                                case I2D_TEMPORARY_SCOPE:
                                    status = i2d_token_putc(state, '.') ||
                                             i2d_token_putc(state, '@') ||
                                             i2d_token_putc(state, symbol);
                                    state->type = I2D_LITERAL;
                                    break;
                                case I2D_TEMPORARY_INSTANCE:
                                    status = i2d_token_putc(state, '\'') ||
                                             i2d_token_putc(state, symbol);
                                    state->type = I2D_LITERAL;
                                    break;
                                case I2D_PERMANENT_ACCOUNT_LOCAL:
                                    status = i2d_token_putc(state, '#') ||
                                             i2d_token_putc(state, symbol);
                                    state->type = I2D_LITERAL;
                                    break;
                                case I2D_PERMANENT_ACCOUNT_GLOBAL:
                                    status = i2d_token_putc(state, '#') ||
                                             i2d_token_putc(state, '#') ||
                                             i2d_token_putc(state, symbol);
                                    state->type = I2D_LITERAL;
                                    break;
                                default:
                                    status = i2d_lexer_token_init(lexer, &token, I2D_LITERAL) ||
                                             i2d_token_putc(token, symbol);
                                    break;
                            }
                        } else {
                            status = i2d_lexer_token_init(lexer, &token, I2D_LITERAL) ||
                                     i2d_token_putc(token, symbol);
                        }
                    } else if(isspace(symbol)) {
                        state = NULL;
                    } else {
                        status = i2d_panic("unknown symbol %c", symbol);
                    }
                    break;
            }

            if(token) {
                i2d_token_append(token, root);
                state = token;
            }
        }
    }

    if(status)
        i2d_lexer_reset(lexer, &root);
    else
        *result = root;

    return status;
}

const char * i2d_node_string[] = {
    "node",
    "number",
    "variable",
    "function",
    "unary",
    "binary"
};

int i2d_rbt_cmp_node(const void * left, const void * right) {
    int status = I2D_OK;
    i2d_string l;
    i2d_string r;

    if(!i2d_node_get_string((i2d_node *) left, &l) && !i2d_node_get_string((i2d_node *) right, &r))
        status = strcasecmp(l.string, r.string);

    return status;
}

int i2d_node_init(i2d_node ** result, enum i2d_node_type type, i2d_token * tokens) {
    int status = I2D_OK;
    i2d_node * object = NULL;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            object->type = type;
            object->tokens = tokens;

            if(status)
                i2d_node_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_node_deit(i2d_node ** result) {
    i2d_node * object;

    object = *result;
    i2d_deit(object->right, i2d_node_deit);
    i2d_deit(object->left, i2d_node_deit);
    i2d_deit(object->tokens, i2d_token_list_deit);
    i2d_deit(object->logic, i2d_logic_deit);
    i2d_range_destroy(&object->range);
    i2d_free(object);
    *result = NULL;
}

void i2d_node_list_deit(i2d_node ** result) {
    i2d_node * object;
    i2d_node * node;

    object = *result;
    if(object) {
        while(object != object->left) {
            node = object->left;
            i2d_node_remove(node);
            i2d_free(node);
        }
        i2d_free(object);
    }
    *result = NULL;
}

void i2d_node_append(i2d_node * x, i2d_node * y) {
    x->left->right = y->right;
    y->right->left = x->left;
    x->left = y;
    y->right = x;
}

void i2d_node_remove(i2d_node * x) {
    x->right->left = x->left;
    x->left->right = x->right;
    x->left = x;
    x->right = x;
}

void i2d_node_print(i2d_node * node, int level) {
    int i;

    for(i = 0; i < level; i++)
        fprintf(stdout, "    ");

    fprintf(stdout, "(%s) ", i2d_node_string[node->type]);

    i2d_token_print(node->tokens);
    i2d_range_print(&node->range, NULL);

    if(node->logic)
        i2d_logic_print(node->logic, level + 1);

    if(node->left)
        i2d_node_print(node->left, level + 1);
    if(node->right)
        i2d_node_print(node->right, level + 1);
}

int i2d_node_copy(i2d_node * source, i2d_node * target) {
    int status = I2D_OK;

    i2d_range_destroy(&source->range);
    i2d_deit(source->logic, i2d_logic_deit);

    if(i2d_range_copy(&source->range, &target->range)) {
        status = i2d_panic("failed to copy range list object");
    } else if(target->logic) {
        if(i2d_logic_copy(&source->logic, target->logic))
            status = i2d_panic("failed to copy logic object");
    }

    return status;
}

int i2d_node_get_arguments(i2d_node * node, i2d_node ** nodes, size_t required, size_t optional) {
    int status = I2D_OK;
    size_t i;
    size_t size;
    i2d_node * iter;

    if(!node) {
        status = i2d_panic("failed on empty argument list");
    } else {
        if(node->type == I2D_NODE)
            node = node->left;

        if(!node) {
            status = i2d_panic("failed on empty argument list");
        } else if(required > 0) {
            size = 1;
            iter = node;
            while(I2D_COMMA == iter->tokens->type) {
                iter = iter->left;
                size++;
            }

            if(size < required) {
                status = i2d_panic("failed on insufficient argument list");
            } else if(required + optional < size) {
                status = i2d_panic("failed on excessive argument list");
            } else {
                i = size - 1;
                while(i > 0 && I2D_COMMA == node->tokens->type) {
                    nodes[i] = node->right;
                    node = node->left;
                    i--;
                }

                if(i || !node) {
                    status = i2d_panic("failed on insufficient argument list");
                } else if(I2D_COMMA == node->tokens->type) {
                    status = i2d_panic("failed on excessive argument list");
                } else {
                    nodes[0] = node;
                }
            }
        } else if(I2D_NODE != node->type || node->left || node->right) {
            status = i2d_panic("failed on excessive argument list");
        }
    }

    return status;
}

int i2d_node_get_constant(i2d_node * node, long * result) {
    int status = I2D_OK;

    long min;
    long max;

    i2d_range_get_range(&node->range, &min, &max);
    if(min != max) {
        status = i2d_panic("failed on invalid range");
    } else {
        *result = min;
    }

    return status;
}

int i2d_node_set_constant(i2d_node * node, i2d_constant * constant) {
    int status = I2D_OK;

    if(i2d_token_set_string(node->tokens, &constant->name)) {
        status = i2d_panic("failed to copy name");
    } else if(i2d_range_copy(&node->range, &constant->range)) {
        status = i2d_panic("failed to copy range");
    }

    return status;
}

int i2d_node_get_string(i2d_node * node, i2d_string * result) {
    return i2d_token_get_string(node->tokens, result);
}

int i2d_node_get_predicate(i2d_node * node, i2d_string * result) {
    int status = I2D_FAIL;

    if(I2D_VARIABLE == node->type) {
        status = i2d_node_get_string(node, result);
    } else if(I2D_FUNCTION == node->type) {
        status = i2d_node_get_string(node, result);
    } else {
        if(node->left)
            status = i2d_node_get_predicate(node->left, result);
        if(node->right && status)
            status = i2d_node_get_predicate(node->right, result);
    }

    return status;
}

int i2d_node_get_predicate_all(i2d_node * node, i2d_string_stack * stack) {
    int status = I2D_OK;
    i2d_string string;

    if( I2D_VARIABLE == node->type ||
        I2D_FUNCTION == node->type ) {
        if(i2d_node_get_string(node, &string)) {
            status = i2d_panic("failed to get node string");
        } else if(i2d_string_stack_push(stack, string.string, string.length)) {
            status = i2d_panic("failed to push node string");
        }
    } else {
        if(node->left)
            status = i2d_node_get_predicate_all(node->left, stack);
        if(!status && node->right)
            status = i2d_node_get_predicate_all(node->right, stack);
    }

    return status;
}

i2d_statement statements[] = {
    {I2D_BONUS, {"bonus", 5}},
    {I2D_BONUS2, {"bonus2", 6}},
    {I2D_BONUS3, {"bonus3", 6}},
    {I2D_BONUS4, {"bonus4", 6}},
    {I2D_BONUS5, {"bonus5", 6}},
    {I2D_AUTOBONUS, {"autobonus", 9}},
    {I2D_AUTOBONUS2, {"autobonus2", 10}},
    {I2D_AUTOBONUS3, {"autobonus3", 10}},
    {I2D_HEAL, {"heal", 4}},
    {I2D_PERCENTHEAL, {"percentheal", 11}},
    {I2D_ITEMHEAL, {"itemheal", 8}},
    {I2D_SKILL, {"skill", 5}},
    {I2D_ITEMSKILL, {"itemskill", 9}},
    {I2D_UNITSKILLUSEID, {"unitskilluseid", 14}},
    {I2D_SC_START, {"sc_start", 8}},
    {I2D_SC_START4, {"sc_start4", 9}},
    {I2D_SC_END, {"sc_end", 6}},
    {I2D_GETITEM, {"getitem", 7}},
    {I2D_RENTITEM, {"rentitem", 8}},
    {I2D_DELITEM, {"delitem", 7}},
    {I2D_GETRANDGROUPITEM, {"getrandgroupitem", 16}},
    {I2D_SKILLEFFECT, {"skilleffect", 11}},
    {I2D_SPECIALEFFECT2, {"specialeffect2", 14}},
    {I2D_SETFONT, {"setfont", 7}},
    {I2D_BUYINGSTORE, {"buyingstore", 11}},
    {I2D_SEARCHSTORES, {"searchstores", 12}},
    {I2D_SET, {"set", 3}},
    {I2D_INPUT, {"input", 5}},
    {I2D_ANNOUNCE, {"announce", 8}},
    {I2D_CALLFUNC, {"callfunc", 8}},
    {I2D_END, {"end", 3}},
    {I2D_WARP, {"warp", 4}},
    {I2D_PET, {"pet", 3}},
    {I2D_BPET, {"bpet", 4}},
    {I2D_MERCENARY_CREATE, {"mercenary_create", 16}},
    {I2D_MERCENARY_HEAL, {"mercenary_heal", 14}},
    {I2D_MERCENARY_SC_START, {"mercenary_sc_start", 18}},
    {I2D_PRODUCE, {"produce", 7}},
    {I2D_COOKING, {"cooking", 7}},
    {I2D_MAKERUNE, {"makerune", 8}},
    {I2D_GUILDGETEXP, {"guildgetexp", 11}},
    {I2D_GETEXP, {"getexp", 6}},
    {I2D_MONSTER, {"monster", 7}},
    {I2D_HOMEVOLUTION, {"homevolution", 12}},
    {I2D_SETOPTION, {"setoption", 9}},
    {I2D_SETMOUNTING, {"setmounting", 11}},
    {I2D_SETFALCON, {"setfalcon", 9}},
    {I2D_GETGROUPITEM, {"getgroupitem", 12}},
    {I2D_RESETSTATUS, {"resetstatus", 11}},
    {I2D_BONUS_SCRIPT, {"bonus_script", 12}},
    {I2D_PLAYBGM, {"playbgm", 7}},
    {I2D_TRANSFORM, {"transform", 9}},
    {I2D_SC_START2, {"sc_start2", 9}},
    {I2D_PETLOOT, {"petloot", 7}},
    {I2D_PETRECOVERY, {"petrecovery", 11}},
    {I2D_PETSKILLBONUS, {"petskillbonus", 13}},
    {I2D_PETSKILLATTACK, {"petskillattack", 14}},
    {I2D_PETSKILLATTACK2, {"petskillattack2", 15}},
    {I2D_PETSKILLSUPPORT, {"petskillsupport", 15}},
    {I2D_PETHEAL, {"petheal", 7}},
    {I2D_FOR, {"for", 3}},
    {I2D_GETMAPXY, {"getmapxy", 8}},
    {I2D_SPECIALEFFECT, {"specialeffect", 13}},
    {I2D_SHOWSCRIPT, {"showscript", 10}}
};

const char * i2d_block_string[] = {
    "block",
    "expression",
    "statement",
    "if",
    "else"
};

int i2d_block_init(i2d_block ** result, enum i2d_block_type type, i2d_token * tokens, i2d_block * parent) {
    int status = I2D_OK;
    i2d_block * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_buffer_create(&object->buffer, I2D_SIZE)) {
                status = i2d_panic("failed to create buffer object");
            } else {
                object->type = type;
                object->tokens = tokens;
                object->parent = parent;
                object->next = object;
                object->prev = object;
            }
            if(status)
                i2d_block_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_block_deit(i2d_block ** result) {
    i2d_block * object;

    object = *result;
    i2d_deit(object->child, i2d_block_list_deit);
    i2d_deit(object->nodes, i2d_node_deit);
    i2d_deit(object->tokens, i2d_token_list_deit);
    i2d_buffer_destroy(&object->buffer);
    i2d_free(object);
    *result = NULL;
}

void i2d_block_list_deit(i2d_block ** result) {
    i2d_block * object;
    i2d_block * block;

    object = *result;
    if(object) {
        while(object != object->next) {
            block = object->next;
            i2d_block_remove(block);
            i2d_block_deit(&block);
        }
    }
    i2d_deit(object, i2d_block_deit);
    *result = NULL;
}

void i2d_block_append(i2d_block * x, i2d_block * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_block_remove(i2d_block * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

void i2d_block_print(i2d_block * block, int level) {
    int i;

    for(i = 0; i < level; i++)
        fprintf(stdout, "    ");

    if(block->statement)
        fprintf(stdout, "%s [%p]\n", block->statement->name.string, block);
    else
        fprintf(stdout, "%s [%p]\n", i2d_block_string[block->type], block);

    if(block->nodes)
        i2d_node_print(block->nodes, level + 1);
    if(block->child)
        i2d_block_list_print(block->child, level + 1);
}

void i2d_block_list_print(i2d_block * block, int level) {
    i2d_block * iterator;

    iterator = block;
    do {
        i2d_block_print(iterator, level);
        iterator = iterator->next;
    } while(iterator != block);
}

int i2d_parser_init(i2d_parser ** result) {
    int status = I2D_OK;
    i2d_parser * object;

    size_t i;
    size_t size;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_block_init(&object->block_cache, I2D_BLOCK, NULL, NULL)) {
                status = i2d_panic("failed to create block object");
            } else if(i2d_node_init(&object->node_cache, I2D_NODE, NULL)) {
                status = i2d_panic("failed to create node object");
            } else {
                object->node_cache->left = object->node_cache;
                object->node_cache->right = object->node_cache;

                if(i2d_rbt_init(&object->statement_map, i2d_rbt_cmp_str)) {
                    status = i2d_panic("failed to create red black tree object");
                } else {
                    size = i2d_size(statements);
                    for(i = 0; i < size; i++)
                        if(i2d_rbt_insert(object->statement_map, statements[i].name.string, &statements[i]))
                            status = i2d_panic("failed to index statement object");
                }
            }

            if(status)
                i2d_parser_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_parser_deit(i2d_parser ** result) {
    i2d_parser * object;

    object = *result;
    i2d_deit(object->statement_map, i2d_rbt_deit);
    i2d_deit(object->node_cache, i2d_node_list_deit);
    i2d_deit(object->block_cache, i2d_block_list_deit);
    i2d_free(object);
    *result = NULL;
}

int i2d_parser_statement_map(i2d_parser * parser, i2d_lexer * lexer, i2d_block * block) {
    int status = I2D_OK;
    i2d_token * token;
    i2d_string name;

    token = block->tokens->next;
    if(I2D_LITERAL == token->type) {
        if(i2d_token_get_string(token, &name)) {
            status = i2d_panic("failed to get token string");
        } else {
            if(!i2d_rbt_search(parser->statement_map, name.string, (void *) &block->statement)) {
                i2d_token_remove(token);
                i2d_lexer_reset(lexer, &token);
            }
        }
    }

    return status;
}

void i2d_parser_reset(i2d_parser * parser, i2d_lexer * lexer, i2d_block ** result) {
    i2d_block * block;

    block = *result;
    do {
        if(block->child)
            i2d_parser_reset(parser, lexer, &block->child);
        block->parent = NULL;
        i2d_buffer_clear(&block->buffer);
        block->statement = NULL;
        if(block->nodes)
            i2d_parser_node_reset(parser, lexer, &block->nodes);
        if(block->tokens)
            i2d_lexer_reset(lexer, &block->tokens);
        block->type = I2D_BLOCK;
        block = block->next;
    } while(block != *result);
    i2d_block_append(block, parser->block_cache);
    *result = NULL;
}

void i2d_parser_node_reset(i2d_parser * parser, i2d_lexer * lexer, i2d_node ** result) {
    i2d_node * node;

    node = *result;
    i2d_deit(node->logic, i2d_logic_deit);
    i2d_range_destroy(&node->range);
    if(node->left)
        i2d_parser_node_reset(parser, lexer, &node->left);
    if(node->right)
        i2d_parser_node_reset(parser, lexer, &node->right);
    if(node->tokens)
        i2d_lexer_reset(lexer, &node->tokens);
    node->type = I2D_NODE;
    node->left = node;
    node->right = node;
    i2d_node_append(node, parser->node_cache);
    *result = NULL;
}

int i2d_parser_block_init(i2d_parser * parser, i2d_block ** result, enum i2d_block_type type, i2d_token * tokens, i2d_block * parent) {
    int status = I2D_OK;
    i2d_block * block;

    if(parser->block_cache != parser->block_cache->next) {
        block = parser->block_cache->next;
        i2d_block_remove(block);
        block->type = type;
        block->tokens = tokens;
        block->parent = parent;
        *result = block;
    } else {
        status = i2d_block_init(result, type, tokens, parent);
    }

    return status;
}

int i2d_parser_node_init(i2d_parser * parser, i2d_node ** result, enum i2d_node_type type, i2d_token * tokens) {
    int status = I2D_OK;
    i2d_node * node;

    if(parser->node_cache != parser->node_cache->left) {
        node = parser->node_cache->left;
        i2d_node_remove(node);
        node->type = type;
        node->tokens = tokens;
        node->left = NULL;
        node->right = NULL;
        *result = node;
    } else {
        status = i2d_node_init(result, type, tokens);
    }

    return status;
}

int i2d_parser_analysis(i2d_parser * parser, i2d_lexer * lexer, i2d_token * tokens, i2d_block ** result) {
    int status = I2D_OK;

    if(I2D_CURLY_OPEN != tokens->next->type) {
        status = i2d_panic("script must start with a {");
    } else if(I2D_CURLY_CLOSE != tokens->prev->type) {
        status = i2d_panic("script must end with a {");
    } else if(i2d_parser_analysis_recursive(parser, lexer, NULL, result, tokens->next)) {
        status = i2d_panic("failed to parse script");
    }

    return status;
}

int i2d_parser_analysis_recursive(i2d_parser * parser, i2d_lexer * lexer, i2d_block * parent, i2d_block ** result, i2d_token * tokens) {
    int status = I2D_OK;
    i2d_block * root;
    i2d_block * block;
    i2d_block * state;
    i2d_token * anchor;
    i2d_token * token = NULL;
    i2d_string string;
    int parenthesis;

    root = NULL;
    block = NULL;
    anchor = tokens;
    while(I2D_TOKEN != tokens->type && I2D_CURLY_CLOSE != tokens->type && !status) {
        if(I2D_CURLY_OPEN == tokens->type) {
            if(i2d_parser_block_init(parser, &block, I2D_BLOCK, NULL, parent)) {
                status = i2d_panic("failed to create block object");
            } else if(i2d_parser_analysis_recursive(parser, lexer, block, &block->child, tokens->next)) {
                status = i2d_panic("failed to parse script");
            } else if(I2D_CURLY_CLOSE != tokens->next->type) {
                status = i2d_panic("missing } after {");
            } else {
                tokens = tokens->next->next;
                i2d_token_append(anchor->prev, tokens);
                i2d_lexer_reset(lexer, &anchor);
                anchor = tokens;
            }
        } else if(I2D_SEMICOLON == tokens->type) {
            if(anchor == tokens) {
                status = i2d_panic("empty statement");
            } else if(i2d_parser_block_init(parser, &block, I2D_STATEMENT, anchor, parent)) {
                status = i2d_panic("failed to create block object");
            } else {
                i2d_token_append(anchor->prev, tokens);
                tokens = tokens->next;
                anchor = tokens;

                token = tokens->prev;
                i2d_token_remove(token);
                i2d_lexer_reset(lexer, &token);

                if(i2d_lexer_token_init(lexer, &token, I2D_TOKEN)) {
                    status = i2d_panic("failed to create token object");
                } else {
                    i2d_token_append(token, block->tokens);
                    block->tokens = token;
                    token = NULL;
                    if(i2d_parser_statement_map(parser, lexer, block)) {
                        status = i2d_panic("failed to lookup block data object");
                    } else if(block->statement && block->tokens->next->type == I2D_SEMICOLON) {
                        /* support statements without arguments */
                    } else if(i2d_parser_expression_recursive(parser, lexer, block->tokens->next, &block->nodes)) {
                        status = i2d_panic("failed to parse expression");
                    }
                }
            }
        } else if(I2D_LITERAL == tokens->type) {
            if(i2d_token_get_string(tokens, &string)) {
                status = i2d_panic("failed to get token string");
            } else if(!strcmp("if", string.string)) {
                if(i2d_parser_block_init(parser, &block, I2D_IF, anchor, parent)) {
                    status = i2d_panic("failed to create block object");
                } else {
                    tokens = tokens->next;
                    i2d_token_remove(anchor);
                    anchor = tokens;

                    if(I2D_PARENTHESIS_OPEN != tokens->type) {
                        status = i2d_panic("missing ( after if");
                    } else {
                        parenthesis = 1;
                        while(I2D_TOKEN != tokens->type && parenthesis) {
                            tokens = tokens->next;
                            switch(tokens->type) {
                                case I2D_PARENTHESIS_OPEN:  parenthesis++; break;
                                case I2D_PARENTHESIS_CLOSE: parenthesis--; break;
                                default: break;
                            }
                        }
                        if(I2D_PARENTHESIS_CLOSE != tokens->type) {
                            status = i2d_panic("missing ) after (");
                        } else if(anchor->next == tokens) {
                            status = i2d_panic("empty if expression");
                        } else {
                            if(i2d_lexer_token_init(lexer, &token, I2D_TOKEN)) {
                                status = i2d_panic("failed to create token object");
                            } else {
                                tokens = tokens->next;
                                i2d_token_append(anchor->prev, tokens);
                                i2d_token_append(token, anchor);
                                anchor = tokens;

                                if(i2d_parser_expression_recursive(parser, lexer, token->next, &block->nodes))
                                    status = i2d_panic("failed to parse expression");

                                i2d_lexer_reset(lexer, &token);
                            }
                        }
                    }
                }
            } else if(!strcmp("else", string.string)) {
                if(i2d_parser_block_init(parser, &block, I2D_ELSE, anchor, parent)) {
                    status = i2d_panic("failed to create block object");
                } else {
                    tokens = tokens->next;
                    i2d_token_remove(anchor);
                    anchor = tokens;
                }
            } else {
                tokens = tokens->next;
            }
        } else {
            tokens = tokens->next;
        }

        if(block) {
            if(!root) {
                root = block;
            } else {
                if((I2D_IF == state->type || I2D_ELSE == state->type) && !state->child) {
                    state->child = block;
                } else {
                    i2d_block_append(block, root);
                }
            }
            state = block;
            block = NULL;
        }
    }

    if(status) {
        i2d_deit(root, i2d_block_list_deit);
    } else {
        *result = root;
    }

    return status;
}

int i2d_parser_expression_recursive(i2d_parser * parser, i2d_lexer * lexer, i2d_token * tokens, i2d_node ** result) {
    int status = I2D_OK;
    i2d_node * root = NULL;
    i2d_node * iter = NULL;
    i2d_node * node = NULL;

    int parenthesis;
    i2d_token * anchor;
    i2d_token * token = NULL;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        while(tokens->type != I2D_TOKEN && !status) {
            switch(tokens->type) {
                case I2D_PARENTHESIS_OPEN:
                    parenthesis = 1;
                    anchor = tokens;
                    while(I2D_TOKEN != tokens->type && parenthesis) {
                        tokens = tokens->next;
                        switch(tokens->type) {
                            case I2D_PARENTHESIS_OPEN:  parenthesis++; break;
                            case I2D_PARENTHESIS_CLOSE: parenthesis--; break;
                            default: break;
                        }
                    }
                    if(I2D_PARENTHESIS_CLOSE != tokens->type) {
                        status = i2d_panic("missing ) after (");
                    } else if(anchor->next == tokens) {
                        if(i2d_lexer_token_init(lexer, &token, I2D_TOKEN)) {
                            status = i2d_panic("failed to create token object");
                        } else {
                            if(i2d_parser_node_init(parser, &node, I2D_NODE, token)) {
                                status = i2d_panic("failed to create node object");
                            } else {
                                tokens = tokens->next;
                            }

                            if(status) {
                                i2d_lexer_reset(lexer, &token);
                            } else {
                                token = NULL;
                            }
                        }
                    } else {
                        anchor = anchor->next;
                        i2d_token_append(anchor->prev, tokens);
                        tokens = tokens->next;

                        if(i2d_lexer_token_init(lexer, &token, I2D_TOKEN)) {
                            status = i2d_panic("failed to create token object");
                        } else {
                            i2d_token_append(token, anchor);
                            if(i2d_parser_expression_recursive(parser, lexer, anchor, &node))
                                status = i2d_panic("failed to parse expression");

                            i2d_lexer_reset(lexer, &token);
                        }
                    }

                    if(!status && root) {
                        iter = root;
                        while(iter->right)
                            iter = iter->right;

                        if(I2D_VARIABLE == iter->type) {
                            iter->left = node;
                            iter->type = I2D_FUNCTION;
                            node = NULL;
                        }
                    }
                    break;
                case I2D_LITERAL:
                    if(i2d_parser_node_init(parser, &node, I2D_VARIABLE, tokens)) {
                        status = i2d_panic("failed to create node object");
                    } else {
                        tokens = tokens->next;
                        i2d_token_remove(tokens->prev);
                    }
                    break;
                case I2D_NOT:
                case I2D_BIT_NOT:
                    if(i2d_parser_node_init(parser, &node, I2D_UNARY, tokens)) {
                        status = i2d_panic("failed to create node object");
                    } else {
                        tokens = tokens->next;
                        i2d_token_remove(tokens->prev);
                    }
                    break;
                case I2D_ADD:
                case I2D_SUBTRACT:
                    if(root) {
                        iter = root;
                        while(iter->right)
                            iter = iter->right;

                        if(I2D_UNARY == iter->type || I2D_BINARY == iter->type) {
                            if(i2d_parser_node_init(parser, &node, I2D_UNARY, tokens)) {
                                status = i2d_panic("failed to create node object");
                            } else {
                                tokens = tokens->next;
                                i2d_token_remove(tokens->prev);

                                if(I2D_ADD == node->tokens->type) {
                                    node->tokens->type = I2D_ADD_UNARY;
                                } else {
                                    node->tokens->type = I2D_SUBTRACT_UNARY;
                                }
                            }
                        } else {
                            if(i2d_parser_node_init(parser, &node, I2D_BINARY, tokens)) {
                                status = i2d_panic("failed to create node object");
                            } else {
                                tokens = tokens->next;
                                i2d_token_remove(tokens->prev);
                            }
                        }
                    } else {
                        if(i2d_parser_node_init(parser, &node, I2D_UNARY, tokens)) {
                            status = i2d_panic("failed to create node object");
                        } else {
                            tokens = tokens->next;
                            i2d_token_remove(tokens->prev);

                            if(I2D_ADD == node->tokens->type) {
                                node->tokens->type = I2D_ADD_UNARY;
                            } else {
                                node->tokens->type = I2D_SUBTRACT_UNARY;
                            }
                        }
                    }
                    break;
                case I2D_COMMA:
                case I2D_MULTIPLY:
                case I2D_DIVIDE:
                case I2D_MODULUS:
                case I2D_ADD_ASSIGN:
                case I2D_SUBTRACT_ASSIGN:
                case I2D_MULTIPLY_ASSIGN:
                case I2D_DIVIDE_ASSIGN:
                case I2D_MODULUS_ASSIGN:
                case I2D_GREATER:
                case I2D_LESS:
                case I2D_EQUAL:
                case I2D_GREATER_EQUAL:
                case I2D_LESS_EQUAL:
                case I2D_NOT_EQUAL:
                case I2D_RIGHT_SHIFT:
                case I2D_LEFT_SHIFT:
                case I2D_BIT_AND:
                case I2D_BIT_OR:
                case I2D_BIT_XOR:
                case I2D_RIGHT_SHIFT_ASSIGN:
                case I2D_LEFT_SHIFT_ASSIGN:
                case I2D_BIT_AND_ASSIGN:
                case I2D_BIT_OR_ASSIGN:
                case I2D_BIT_XOR_ASSIGN:
                case I2D_AND:
                case I2D_OR:
                case I2D_CONDITIONAL:
                case I2D_COLON:
                case I2D_ASSIGN:
                    if(i2d_parser_node_init(parser, &node, I2D_BINARY, tokens)) {
                        status = i2d_panic("failed to create node object");
                    } else {
                        tokens = tokens->next;
                        i2d_token_remove(tokens->prev);
                    }
                    break;
                default:
                    status = i2d_panic("invalid token - %d", tokens->type);
            }

            if(node) {
                if(!root) {
                    if(I2D_BINARY == node->type) {
                        status = i2d_panic("binary operator without operand");
                    } else {
                        root = node;
                    }
                } else if(i2d_token_precedence[node->tokens->type] < i2d_token_precedence[root->tokens->type]) {
                    if(I2D_BINARY == node->type) {
                        if(!root->right) {
                            status = i2d_panic("binary operator without operand");
                        } else {
                            node->left = root->right;
                            root->right = node;
                        }
                    } else {
                        iter = root;
                        while(iter->right)
                            iter = iter->right;

                        if(I2D_BINARY != iter->type && I2D_UNARY != iter->type) {
                            status = i2d_panic("operand without binary operator");
                        } else {
                            iter->right = node;
                        }
                    }
                } else {
                    if(I2D_BINARY != node->type) {
                        status = i2d_panic("operand without binary operator");
                    } else {
                        node->left = root;
                        root = node;
                    }
                }

                if(status)
                    i2d_node_deit(&node);
                else
                    node = NULL;
            }
        }

        if(!status) {
            if(root && I2D_BINARY == root->type && !root->right) {
                status = i2d_panic("binary operator missing right operand");
            } else if(i2d_lexer_token_init(lexer, &token, I2D_TOKEN)) {
                status = i2d_panic("failed to create token object");
            } else {
                if(i2d_node_init(&node, I2D_NODE, token)) {
                    status = i2d_panic("failed to create node object");
                } else {
                    node->left = root;
                    root = node;
                }

                if(status)
                    i2d_lexer_reset(lexer, &token);
            }
        }

        if(status) {
            i2d_deit(root, i2d_node_deit);
        } else {
            *result = root;
        }
    }

    return status;
}

int i2d_format_create(i2d_format * result, const char * string, size_t length) {
    int status = I2D_OK;

    size_t i;
    char symbol;
    i2d_token * token = NULL;
    i2d_token * state = NULL;
    int curly_level = 0;

    result->tokens = NULL;

    if(i2d_token_init(&result->tokens, I2D_TOKEN)) {
        status = i2d_panic("failed to create token object");
    } else {
        for(i = 0; i < length && !status; i++) {
            symbol = string[i];
            switch(symbol) {
                case '{':
                    if(curly_level) {
                        status = i2d_panic("invalid position level");
                    } else {
                        curly_level++;
                        state = NULL;
                    }
                    break;
                case '}':
                    if(!state) {
                        status = i2d_panic("invalid position token");
                    } else {
                        curly_level--;
                        state = NULL;
                    }
                    break;
                default:
                    if(state) {
                        status = i2d_token_putc(state, symbol);
                    } else {
                        status = i2d_token_init(&token, curly_level ? I2D_POSITION : I2D_LITERAL) ||
                                 i2d_token_putc(token, symbol);
                    }
                    break;
            }

            if(token) {
                i2d_token_append(token, result->tokens);
                state = token;
                token = NULL;
            }
        }
    }

    if(result->tokens == result->tokens->next)
        status = i2d_panic("empty format specification");

    if(status)
        i2d_format_destroy(result);

    return status;
}

int i2d_format_create_json(i2d_format * result, json_t * json) {
    int status = I2D_OK;
    i2d_string format;
    i2d_zero(format);

    if(i2d_object_get_string(json, &format)) {
        status = i2d_panic("failed to get string object");
    } else {
        status = i2d_format_create(result, format.string, format.length);

        i2d_string_destroy(&format);
    }

    return status;
}

void i2d_format_destroy(i2d_format * result) {
    i2d_deit(result->tokens, i2d_token_list_deit);
}

int i2d_format_write(i2d_format * format, i2d_string_stack * stack, i2d_buffer * buffer) {
    int status = I2D_OK;
    i2d_string * list;
    size_t size;

    i2d_token * token;
    i2d_string string;
    long position;

    if(i2d_string_stack_get(stack, &list, &size)) {
        status = i2d_panic("failed to get string stack");
    } else {
        token = format->tokens->next;
        while(token->type != I2D_TOKEN && !status) {
            switch(token->type) {
                case I2D_LITERAL:
                    if(i2d_token_get_string(token, &string)) {
                        status = i2d_panic("failed to get token string");
                    } else if(i2d_buffer_printf(buffer, "%s", string.string)) {
                        status = i2d_panic("failed to write buffer");
                    }
                    break;
                case I2D_POSITION:
                    if(i2d_token_get_constant(token, &position)) {
                        status = i2d_panic("failed to get token constant");
                    } else if(position >= size || position < 0) {
                        status = i2d_panic("invalid position on string stack");
                    } else if(i2d_buffer_printf(buffer, "%s", list[position].string)) {
                        status = i2d_panic("failed to write buffer");
                    }
                    break;
                default:
                    status = i2d_panic("invalid token type -- %d", token->type);
            }
            token = token->next;
        }
    }

    return status;
}

int i2d_data_create(i2d_data * result, const char * key, json_t * json, i2d_constant_db * constant_db) {
    int status = I2D_OK;
    json_t * min;
    json_t * max;
    json_t * description;
    json_t * argument_type;

    min = json_object_get(json, "min");
    max = json_object_get(json, "max");
    description = json_object_get(json, "description");
    argument_type = json_object_get(json, "argument_type");
    i2d_constant_get_by_macro_value(constant_db, key, &result->value);

    if(i2d_string_create(&result->name, key, strlen(key))) {
        status = i2d_panic("failed to copy name string");
    } else if(description && i2d_format_create_json(&result->format, description)) {
        status = i2d_panic("failed to create format object");
    } else if(min && max && i2d_object_get_range(min, max, &result->range)) {
        status = i2d_panic("failed to create range");
    } else if(argument_type && i2d_object_get_string_stack(argument_type, &result->types)) {
        status = i2d_panic("failed to create string stack");
    }

    return status;
}

void i2d_data_destroy(i2d_data * result) {
    i2d_string_stack_destroy(&result->types);
    i2d_range_destroy(&result->range);
    i2d_format_destroy(&result->format);
    i2d_string_destroy(&result->name);
}

int i2d_data_map_init(i2d_data_map ** result, enum i2d_data_map_type type, json_t * json, i2d_constant_db * constant_db) {
    int status = I2D_OK;
    i2d_data_map * object;
    i2d_rbt_cmp cmp;

    size_t i = 0;
    const char * key;
    json_t * value;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            cmp = (type == data_map_by_value) ? i2d_rbt_cmp_long :
                  (type == data_map_by_name) ? i2d_rbt_cmp_str :
                  NULL;

            if(!cmp || i2d_rbt_init(&object->map, cmp)) {
                status = i2d_panic("failed to create red black tree object");
            } else if(i2d_object_get_list(json, sizeof(*object->list), (void **) &object->list, &object->size)) {
                status = i2d_panic("failed to create data array");
            } else {
                json_object_foreach(json, key, value) {
                    if(i2d_data_create(&object->list[i], key, value, constant_db)) {
                        status = i2d_panic("failed to create data object");
                    } else {
                        if((type == data_map_by_value) ? i2d_rbt_insert(object->map, &object->list[i].value, &object->list[i]) :
                           (type == data_map_by_name) ? i2d_rbt_insert(object->map, object->list[i].name.string, &object->list[i]) :
                           1) {
                            status = i2d_panic("failed to map data object");
                        } else {
                            i++;
                        }
                    }
                }
            }

            if(status)
                i2d_data_map_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_data_map_deit(i2d_data_map ** result) {
    i2d_data_map * object;
    size_t i;

    object = *result;
    if(object->list) {
        for(i = 0; i < object->size; i++)
            i2d_data_destroy(&object->list[i]);
        i2d_free(object->list);
    }
    i2d_deit(object->map, i2d_rbt_deit);
    i2d_free(object);
    *result = NULL;
}

int i2d_data_map_get(i2d_data_map * data_map, void * key, i2d_data ** result) {
    return i2d_rbt_search(data_map->map, key, (void **) result);
}

int i2d_context_init(i2d_context ** result) {
    int status = I2D_OK;
    i2d_context * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if( i2d_rbt_init(&object->variables, i2d_rbt_cmp_node) ||
                i2d_string_stack_create(&object->expression_stack, I2D_STACK) ||
                i2d_string_stack_create(&object->predicate_stack, I2D_STACK) ||
                i2d_buffer_create(&object->expression_buffer, I2D_SIZE) ||
                i2d_buffer_create(&object->predicate_buffer, I2D_SIZE) ) {
                status = i2d_panic("failed to create red black tree, string stack, and buffer objects");
            } else {
                object->next = object;
                object->prev = object;
            }

            if(status)
                i2d_context_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_context_deit(i2d_context ** result) {
    i2d_context * object;

    object = *result;
    i2d_buffer_destroy(&object->predicate_buffer);
    i2d_buffer_destroy(&object->expression_buffer);
    i2d_string_stack_destroy(&object->predicate_stack);
    i2d_string_stack_destroy(&object->expression_stack);
    i2d_deit(object->variables, i2d_rbt_deit);
    i2d_free(object);
    *result = NULL;
}

void i2d_context_list_deit(i2d_context ** result) {
    i2d_context * object;
    i2d_context * context;

    object = *result;
    if(object) {
        while(object != object->next) {
            context = object->next;
            i2d_context_remove(context);
            i2d_context_deit(&context);
        }
        i2d_context_deit(&object);
    }
    *result = NULL;
}

void i2d_context_append(i2d_context * x, i2d_context * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_context_remove(i2d_context * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

void i2d_context_reset(i2d_context * context) {
    i2d_rbt_clear(context->variables);
    i2d_context_reset_local(context);
}

void i2d_context_reset_local(i2d_context * context) {
    i2d_string_stack_clear(&context->expression_stack);
    i2d_string_stack_clear(&context->predicate_stack);
    i2d_buffer_clear(&context->expression_buffer);
    i2d_buffer_clear(&context->predicate_buffer);
}

int i2d_context_add_variable(i2d_context * context, i2d_node * node) {
    int status = I2D_OK;
    i2d_node * last;

    if(I2D_VARIABLE != node->type) {
        status = i2d_panic("invalid node type -- %d", node->type);
    } else {
        if(!i2d_rbt_search(context->variables, node, (void **) &last) &&
            i2d_rbt_delete(context->variables, last) ) {
            status = i2d_panic("failed to replace variable");
        } else if(i2d_rbt_insert(context->variables, node, node)) {
            status = i2d_panic("failed to insert variable");
        }
    }

    return status;
}

int i2d_context_get_variable(i2d_context * context, i2d_node * key, i2d_node ** result) {
    return i2d_rbt_search(context->variables, key, (void **) result);
}

int i2d_script_init(i2d_script ** result, i2d_option * option) {
    int status = I2D_OK;
    i2d_script * object;
    json_t * getiteminfo;
    json_t * strcharinfo;
    json_t * functions;
    json_t * blocks;
    json_t * bonus;
    json_t * bonus2;
    size_t i;
    size_t size;

    if(i2d_is_invalid(result) || !option) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_db_init(&object->db, i2d_pre_renewal, &option->source_path)) {
                status = i2d_panic("failed to create database object");
            } else if(i2d_json_create(&object->json, &option->json_path)) {
                status = i2d_panic("failed to create json object");
            } else if(i2d_lexer_init(&object->lexer)) {
                status = i2d_panic("failed to create lexer object");
            } else if(i2d_parser_init(&object->parser)) {
                status = i2d_panic("failed to create parser object");
            } else if(i2d_constant_db_init(&object->constant_db, object->json)) {
                status = i2d_panic("failed to create constant db object");
            } else {
                getiteminfo = json_object_get(object->json, "getiteminfo");
                strcharinfo = json_object_get(object->json, "strcharinfo");
                functions = json_object_get(object->json, "functions");
                blocks = json_object_get(object->json, "blocks");
                if(blocks) {
                    bonus = json_object_get(blocks, "bonus");
                    bonus2 = json_object_get(blocks, "bonus2");
                }
                if(!getiteminfo || i2d_value_map_init(&object->getiteminfo, getiteminfo)) {
                    status = i2d_panic("failed to load getiteminfo");
                } else if(!strcharinfo || i2d_value_map_init(&object->strcharinfo, strcharinfo)) {
                    status = i2d_panic("failed to load strcharinfo");
                } else if(!functions || i2d_data_map_init(&object->functions, data_map_by_name, functions, object->constant_db)) {
                    status = i2d_panic("failed to load functions");
                } else if(!bonus || i2d_data_map_init(&object->bonus, data_map_by_value, bonus, object->constant_db)) {
                    status = i2d_panic("failed to load bonuses");
                } else if(!bonus2 || i2d_data_map_init(&object->bonus2, data_map_by_value, bonus2, object->constant_db)) {
                    status = i2d_panic("failed to load bonuses");
                } else {
                    if(i2d_rbt_init(&object->function_map, i2d_rbt_cmp_str)) {
                        status = i2d_panic("failed to create function map object");
                    } else {
                        size = i2d_size(function_list);
                        for(i = 0; i < size && !status; i++)
                            if(i2d_rbt_insert(object->function_map, function_list[i].name.string, &function_list[i]))
                                status = i2d_panic("failed to map function handler object");
                    }

                    if(i2d_rbt_init(&object->bonus_map, i2d_rbt_cmp_str)) {
                        status = i2d_panic("failed to create bonus map object");
                    } else {
                        size = i2d_size(bonus_list);
                        for(i = 0; i < size && !status; i++)
                            if(i2d_rbt_insert(object->bonus_map, bonus_list[i].name.string, &bonus_list[i]))
                                status = i2d_panic("failed to map bonus handler object");
                    }
                }
            }

            if(status)
                i2d_script_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_script_deit(i2d_script ** result) {
    i2d_script * object;

    object = *result;
    i2d_deit(object->bonus_map, i2d_rbt_deit);
    i2d_deit(object->function_map, i2d_rbt_deit);
    i2d_deit(object->bonus2, i2d_data_map_deit);
    i2d_deit(object->bonus, i2d_data_map_deit);
    i2d_deit(object->functions, i2d_data_map_deit);
    i2d_deit(object->strcharinfo, i2d_value_map_deit);
    i2d_deit(object->getiteminfo, i2d_value_map_deit);
    i2d_deit(object->constant_db, i2d_constant_db_deit);
    i2d_deit(object->contexts, i2d_context_list_deit);
    i2d_deit(object->parser, i2d_parser_deit);
    i2d_deit(object->lexer, i2d_lexer_deit);
    i2d_json_destroy(object->json);
    i2d_deit(object->db, i2d_db_deit);
    i2d_free(object);
    *result = NULL;
}

int i2d_script_context_init(i2d_script * script, i2d_context ** result) {
    int status = I2D_OK;

    if(script->contexts) {
        if(script->contexts == script->contexts->next) {
            *result = script->contexts;
            script->contexts = NULL;
        } else {
            *result = script->contexts->next;
            i2d_context_remove(*result);
        }
    } else {
        status = i2d_context_init(result);
    }

    return status;
}

int i2d_script_context_deit(i2d_script * script, i2d_context ** result) {
    int status = I2D_OK;
    i2d_context * context;

    context = *result;
    i2d_context_reset(context);
    *result = NULL;

    if(script->contexts) {
        i2d_context_append(context, script->contexts);
    } else {
        script->contexts = context;
    }

    return status;
}

int i2d_script_compile(i2d_script * script, i2d_string * source, i2d_string * target) {
    int status = I2D_OK;
    i2d_token * tokens = NULL;
    i2d_block * blocks = NULL;
    i2d_context * context = NULL;

    if(!strcmp("{}", source->string)) {
        status = i2d_string_create(target, "", 0);
    } else if(i2d_lexer_tokenize(script->lexer, source, &tokens)) {
        status = i2d_panic("failed to tokenize -- %s", source->string);
    } else if(i2d_parser_analysis(script->parser, script->lexer, tokens, &blocks)) {
        status = i2d_panic("failed to parse -- %s", source->string);
    } else if(i2d_script_context_init(script, &context)) {
        status = i2d_panic("failed to create context object");
    } else {
        if(i2d_script_translate(script, blocks, context))
            status = i2d_panic("failed to translate -- %s", source->string);

        i2d_script_context_deit(script, &context);
    }

    if(tokens)
        i2d_lexer_reset(script->lexer, &tokens);

    if(blocks)
        i2d_parser_reset(script->parser, script->lexer, &blocks);

    return status;
}

int i2d_script_translate(i2d_script * script, i2d_block * blocks, i2d_context * context) {
        int status = I2D_OK;
    i2d_block * block;

    if(blocks) {
        block = blocks;
        do {
            switch(block->type) {
                case I2D_BLOCK:
                    status = i2d_script_translate(script, block->child, context);
                    break;
                case I2D_STATEMENT:
                    status = i2d_script_statement(script, block, context);
                    break;
                case I2D_IF:
                    status = i2d_script_expression(script, block->nodes, 1, context);
                    break;
                case I2D_ELSE:
                    break;
                default:
                    status = i2d_panic("invalid block type -- %d", block->type);
                    break;
            }
            block = block->next;
        } while(block != blocks && !status);
    }

    return status;
}

int i2d_script_statement(i2d_script * script, i2d_block * block, i2d_context * context) {
    int status = I2D_OK;

    if(block->statement) {
        switch(block->statement->type) {
            case I2D_BONUS:
                status = i2d_script_bonus(script, block, context, script->bonus, 1);
                break;
            case I2D_BONUS2:
                status = i2d_script_bonus(script, block, context, script->bonus2, 2);
                break;
            default:
                /*status = i2d_panic("invalid statement type -- %d", block->statement->type);*/
                break;
        }
    }

    return status;
}

int i2d_script_expression(i2d_script * script, i2d_node * node, int is_conditional, i2d_context * context) {
    int status = I2D_OK;

    if(node->left && i2d_script_expression(script, node->left, is_conditional, context)) {
        status = i2d_panic("failed to evaluate left expression");
    } else if(node->right && i2d_script_expression(script, node->right, is_conditional, context)) {
        status = i2d_panic("failed to evaluate right expression");
    } else {
        switch(node->type) {
            case I2D_NODE:
                status = (node->left) ? i2d_node_copy(node, node->left) : i2d_range_create_add(&node->range, 0, 0);
                break;
            case I2D_VARIABLE:
                status = i2d_script_expression_variable(script, node, context);
                break;
            case I2D_FUNCTION:
                status = i2d_script_expression_function(script, node, context);
                break;
            case I2D_UNARY:
                status = i2d_script_expression_unary(script, node, is_conditional);
                break;
            case I2D_BINARY:
                status = i2d_script_expression_binary(script, node, is_conditional, context);
                break;
            default:
                status = i2d_panic("invalid node type -- %d", node->type);
                break;
        }
    }

    return status;
}

int i2d_script_expression_variable(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    i2d_node * variable;
    long number;
    i2d_string name;

    if(!i2d_context_get_variable(context, node, &variable)) {
        if(i2d_node_copy(node, variable))
            status = i2d_panic("failed to copy variable");
    } else if(i2d_node_get_string(node, &name)) {
        status = i2d_panic("failed to get variable string");
    } else {
        if(isdigit(name.string[0]) && !i2d_token_get_constant(node->tokens, &number)) {
            node->type = I2D_NUMBER;
        } else if(i2d_constant_get_by_macro_value(script->constant_db, name.string, &number)) {
            number = 0;
        }

        if(i2d_range_create_add(&node->range, number, number))
            status = i2d_panic("failed to create range object");
    }

    return status;
}


int i2d_script_expression_function(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    i2d_string name;
    i2d_handler * handler;

    i2d_context_reset_local(context);

    if(i2d_node_get_string(node, &name)) {
        status = i2d_panic("failed to get function string");
    } else if(i2d_rbt_search(script->function_map, name.string, (void **) &handler)) {
        status = i2d_panic("failed to get function handler -- %s", name.string);
    } else {
        status = handler->handler(script, node, context);
    }

    return status;
}

int i2d_script_expression_unary(i2d_script * script, i2d_node * node, int is_conditional) {
    int status = I2D_OK;

    if(!node->right) {
        status = i2d_panic("unary operator missing operand");
    } else {
        switch(node->tokens->type) {
            case I2D_NOT:
                if(is_conditional) {
                    if(i2d_range_not(&node->range, &node->right->range)) {
                        status = i2d_panic("failed to not range object");
                    } else if(node->right->logic && i2d_logic_not(&node->logic, node->right->logic)) {
                        status = i2d_panic("failed to not logic object");
                    }
                } else {
                    status = i2d_range_create_add(&node->range, 0, 1);
                }
                break;
            case I2D_BIT_NOT:
                if(i2d_range_bitnot(&node->range, &node->right->range))
                    status = i2d_panic("failed to bit not range object");
                break;
            case I2D_ADD_UNARY:
                if(i2d_node_copy(node, node->right))
                    status = i2d_panic("failed to copy range object");
                break;
            case I2D_SUBTRACT_UNARY:
                if(i2d_range_negate(&node->range, &node->right->range))
                    status = i2d_panic("failed to negate range object");
                break;
            default:
                status = i2d_panic("invalid node type -- %d", node->tokens->type);
        }
    }

    return status;
}

enum {
    I2D_FLAG_ASSIGN = 0x1
};

int i2d_script_expression_binary_relational(i2d_node * node, int operator, int is_conditional) {
    int status = I2D_OK;
    i2d_string predicate;
    i2d_zero(predicate);

    if(is_conditional) {
        if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, operator)) {
            status = i2d_panic("failed to compute range object");
        } else if(!i2d_node_get_predicate(node, &predicate) && i2d_logic_init(&node->logic, &predicate, &node->range)) {
            status = i2d_panic("failed to create logic object");
        }
    } else {
        status = i2d_range_create_add(&node->range, 0, 1);
    }

    return status;
}

int i2d_script_expression_binary_logical(i2d_node * node, int operator, int is_conditional) {
    int status = I2D_OK;

    if(is_conditional) {
        if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, operator)) {
            status = i2d_panic("failed to compute range object");
        } else if(node->left->logic && node->right->logic && i2d_logic_and(&node->logic, node->left->logic, node->right->logic)) {
            status = i2d_panic("failed to create logic object");
        }
    } else {
        status = i2d_range_create_add(&node->range, 0, 1);
    }

    return status;
}

int i2d_script_expression_binary(i2d_script * script, i2d_node * node, int is_conditional, i2d_context * context) {
    int status = I2D_OK;
    int flag = 0;

    if(!node->left) {
        status = i2d_panic("binary operator missing left operand");
    } else if(!node->right) {
        status = i2d_panic("binary operator missing right operand");
    } else {
        switch(node->tokens->type) {
            case I2D_ADD_ASSIGN:
                flag |= I2D_FLAG_ASSIGN;
            case I2D_ADD:
                if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, '+'))
                    status = i2d_panic("failed to add left and right operand");
                break;
            case I2D_SUBTRACT_ASSIGN:
                flag |= I2D_FLAG_ASSIGN;
            case I2D_SUBTRACT:
                if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, '-'))
                    status = i2d_panic("failed to subtract left and right operand");
                break;
            case I2D_MULTIPLY_ASSIGN:
                flag |= I2D_FLAG_ASSIGN;
            case I2D_MULTIPLY:
                if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, '*'))
                    status = i2d_panic("failed to multiply left and right operand");
                break;
            case I2D_DIVIDE_ASSIGN:
                flag |= I2D_FLAG_ASSIGN;
            case I2D_DIVIDE:
                if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, '/'))
                    status = i2d_panic("failed to divide left and right operand");
                break;
            case I2D_MODULUS_ASSIGN:
                flag |= I2D_FLAG_ASSIGN;
            case I2D_MODULUS:
                if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, '%'))
                    status = i2d_panic("failed to modulus left and right operand");
                break;
            case I2D_COMMA:
                if(i2d_node_copy(node, node->right))
                    status = i2d_panic("failed to copy right operand");
                break;
            case I2D_RIGHT_SHIFT_ASSIGN:
                flag |= I2D_FLAG_ASSIGN;
            case I2D_RIGHT_SHIFT:
                if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, '>' + '>' + 'b'))
                    status = i2d_panic("failed to right shift operand");
                break;
            case I2D_LEFT_SHIFT_ASSIGN:
                flag |= I2D_FLAG_ASSIGN;
            case I2D_LEFT_SHIFT:
                if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, '<' + '<' + 'b'))
                    status = i2d_panic("failed to left shift operand");
                break;
            case I2D_BIT_AND_ASSIGN:
                flag |= I2D_FLAG_ASSIGN;
            case I2D_BIT_AND:
                if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, '&'))
                    status = i2d_panic("failed to bit and left and right operand");
                break;
            case I2D_BIT_OR_ASSIGN:
                flag |= I2D_FLAG_ASSIGN;
            case I2D_BIT_OR:
                if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, '|'))
                    status = i2d_panic("failed to bit or left and right operand");
                break;
            case I2D_BIT_XOR_ASSIGN:
                flag |= I2D_FLAG_ASSIGN;
            case I2D_BIT_XOR:
                if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, '^' + 'b'))
                    status = i2d_panic("failed to bit xor left and right operand");
                break;
            case I2D_GREATER:
                if(i2d_script_expression_binary_relational(node, '>', is_conditional))
                    status = i2d_panic("failed to compute left is greater than right operand");
                break;
            case I2D_GREATER_EQUAL:
                if(i2d_script_expression_binary_relational(node, '>' + '=', is_conditional))
                    status = i2d_panic("failed to compute left is greater or equal than right operand");
                break;
            case I2D_LESS:
                if(i2d_script_expression_binary_relational(node, '<', is_conditional))
                    status = i2d_panic("failed to compute left is less than right operand");
                break;
            case I2D_LESS_EQUAL:
                if(i2d_script_expression_binary_relational(node, '<' + '=', is_conditional))
                    status = i2d_panic("failed to compute left is less or equal than right operand");
                break;
            case I2D_EQUAL:
                if(i2d_script_expression_binary_relational(node, '=' + '=', is_conditional))
                    status = i2d_panic("failed to compute left is equal to right operand");
                break;
            case I2D_NOT_EQUAL:
                if(i2d_script_expression_binary_relational(node, '!' + '=', is_conditional))
                    status = i2d_panic("failed to compute left is equal to right operand");
                break;
            case I2D_AND:
                if(i2d_script_expression_binary_logical(node, '&' + '&', is_conditional))
                    status = i2d_panic("failed to compute left and right operand");
                break;
            case I2D_OR:
                if(i2d_script_expression_binary_logical(node, '|' + '|', is_conditional))
                    status = i2d_panic("failed to compute left or right operand");
                break;
            case I2D_ASSIGN:
                if(i2d_node_copy(node, node->right)) {
                    status = i2d_panic("failed to copy right range object");
                } else {
                    flag |= I2D_FLAG_ASSIGN;
                }
                break;
            default:
                status = i2d_panic("invalid token type -- %d", node->tokens->type);
        }

        if(!status && (flag & I2D_FLAG_ASSIGN)) {
            if(i2d_node_copy(node->left, node)) {
                status = i2d_panic("failed to copy node to left node");
            } else if(i2d_context_add_variable(context, node->left)) {
                status = i2d_panic("failed to add variable to context");
            }
        }
    }

    return status;
}

static int i2d_handler_general(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    i2d_string name;
    i2d_data * data;

    if(i2d_node_get_string(node, &name)) {
        status = i2d_panic("failed to get function string");
    } else if(i2d_data_map_get(script->functions, name.string, &data)) {
        status = i2d_panic("failed to get function data -- %s", name.string);
    } else {
        i2d_buffer_clear(&node->tokens->buffer);

        if(i2d_format_write(&data->format, &context->expression_stack, &node->tokens->buffer)) {
            status = i2d_panic("failed to write function format");
        } else if(i2d_range_copy(&node->range, &data->range)) {
            status = i2d_panic("failed to copy function range");
        }
    }

    return status;
}

static int i2d_handler_readparam(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    i2d_node * arguments[2];
    long value;
    i2d_constant * constant;

    if(i2d_node_get_arguments(node->left, arguments, 1, 1)) {
        status = i2d_panic("failed to get readparam arguments");
    } else if(i2d_node_get_constant(arguments[0], &value)) {
        status = i2d_panic("failed to get paramater number");
    } else if(i2d_constant_get_by_readparam(script->constant_db, &value, &constant)) {
        status = i2d_panic("failed to get constant by paramater number -- %ld", value);
    } else if(i2d_node_set_constant(node, constant)) {
        status = i2d_panic("failed to write paramater");
    }

    return status;
}

static int i2d_handler_getskilllv(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    i2d_node * arguments;
    long value;
    i2d_string name;
    i2d_skill * skill;

    if(i2d_node_get_arguments(node->left, &arguments, 1, 0)) {
        status = i2d_panic("failed to get getskilllv arguments");
    } else if(i2d_node_get_constant(arguments, &value)) {
        status = i2d_panic("failed to get skill id");
    } else if(i2d_skill_db_search_by_id(script->db->skill_db, value, &skill)) {
        if(i2d_node_get_string(arguments, &name)) {
            status = i2d_panic("failed to get skill string");
        } else if(i2d_skill_db_search_by_macro(script->db->skill_db, name.string, &skill)) {
            status = i2d_panic("failed to get skill by id and string -- %ld, %s", value, name.string);
        }
    }

    if(!status) {
        if(i2d_token_set_string(node->tokens, &skill->name)) {
            status = i2d_panic("failed to write skill string");
        } else if(i2d_range_create_add(&node->range, 0, skill->maxlv)) {
            status = i2d_panic("failed to create skill range");
        }
    }

    return status;
}

static int i2d_handler_isequipped(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    i2d_node * arguments[I2D_STACK];
    size_t i;
    size_t size;
    long value;
    i2d_item * item;
    i2d_string string;

    memset(arguments, 0, sizeof(arguments));
    size = i2d_size(arguments);

    if(i2d_node_get_arguments(node->left, arguments, 1, size - 1)) {
        status = i2d_panic("failed to get arguments");
    } else {
        for(i = 0; i < size && arguments[i] && !status; i++) {
            if(i2d_node_get_constant(arguments[i], &value)) {
                status = i2d_panic("failed to get item id");
            } else if(i2d_item_db_search_by_id(script->db->item_db, value, &item)) {
                status = i2d_panic("failed to get item by id -- %ld", value);
            } else {
                if( i ?
                    i2d_buffer_printf(&context->expression_buffer, ", %s", item->name.string) :
                    i2d_buffer_printf(&context->expression_buffer, "%s", item->name.string) )
                    status = i2d_panic("failed to write expression buffer");
            }
        }
    }

    if(!status) {
        i2d_buffer_get(&context->expression_buffer, &string.string, &string.length);

        if(i2d_string_stack_push(&context->expression_stack, string.string, string.length)) {
            status = i2d_panic("failed to push item list");
        } else {
            status = i2d_handler_general(script, node, context);
        }
    }

    return status;
}

static int i2d_handler_countitem(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    i2d_node * arguments;
    long value;
    i2d_string name;
    i2d_item * item;

    if(i2d_node_get_arguments(node->left, &arguments, 1, 0)) {
        status = i2d_panic("failed to get countitem arguments");
    } else if(i2d_node_get_constant(arguments, &value)) {
        status = i2d_panic("failed to get item id");
    } else if(i2d_item_db_search_by_id(script->db->item_db, value, &item)) {
        if(i2d_node_get_string(arguments, &name)) {
            status = i2d_panic("failed to get item string");
        } else if(i2d_item_db_search_by_name(script->db->item_db, name.string, &item)) {
            status = i2d_panic("failed to get item by id and string -- %s (%ld)", name.string, value);
        }
    }

    if(!status) {
        if(i2d_string_stack_push(&context->expression_stack, item->name.string, item->name.length)) {
            status = i2d_panic("failed to push item name");
        } else {
            status = i2d_handler_general(script, node, context);
        }
    }

    return status;
}

static int i2d_handler_gettime(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    i2d_node * arguments;
    long value;
    i2d_constant * constant;

    if(i2d_node_get_arguments(node->left, &arguments, 1, 0)) {
        status = i2d_panic("failed to get gettime arguments");
    } else if(i2d_node_get_constant(arguments, &value)) {
        status = i2d_panic("failed to get tick type");
    } else if(i2d_constant_get_by_gettime(script->constant_db, &value, &constant)) {
        status = i2d_panic("failed to get constant tick type -- %ld", value);
    } else if(i2d_node_set_constant(node, constant)) {
        status = i2d_panic("failed to write tick type");
    }

    return status;
}

static int i2d_handler_strcharinfo(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    i2d_node * arguments[2];
    long value;
    i2d_string string;

    if(i2d_node_get_arguments(node->left, arguments, 1, 1)) {
        status = i2d_panic("failed to get strcharinfo arguments");
    } else if(i2d_node_get_constant(arguments[0], &value)) {
        status = i2d_panic("failed to get type");
    } else if(i2d_value_map_get(script->strcharinfo, &value, &string)) {
        status = i2d_panic("failed to get strcharinfo by type -- %ld", value);
    } else if(i2d_token_set_string(node->tokens, &string)) {
        status = i2d_panic("failed to write strcharinfo string");
    } else if(i2d_range_copy(&node->range, &arguments[0]->range)) {
        status = i2d_panic("failed to copy strcharinfo range");
    }

    return status;
}

static int i2d_handler_getequipid(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    i2d_node * arguments[2];
    long value;
    i2d_constant * constant;

    if(i2d_node_get_arguments(node->left, arguments, 1, 1)) {
        status = i2d_panic("failed to get getequipid arguments");
    } else if(i2d_node_get_constant(arguments[0], &value)) {
        status = i2d_panic("failed to get equipment slot");
    } else if(i2d_constant_get_by_location(script->constant_db, &value, &constant)) {
        status = i2d_panic("failed to get constant by equipment slot -- %ld", value);
    } else if(i2d_node_set_constant(node, constant)) {
        status = i2d_panic("failed to write equipment slot");
    }

    return status;
}

static int i2d_handler_getiteminfo(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    i2d_node * arguments[2];
    long value;
    i2d_string string;
    i2d_item * item;

    if(i2d_node_get_arguments(node->left, arguments, i2d_size(arguments), 0)) {
        status = i2d_panic("failed to get getiteminfo arguments");
    } else {
        if(I2D_FUNCTION == arguments[0]->type) {
            if(i2d_node_get_string(arguments[0], &string)) {
                status = i2d_panic("failed to get function string");
            } else if(i2d_string_stack_push(&context->expression_stack, string.string, string.length)) {
                status = i2d_panic("failed to push function string");
            }
        } else {
            if(i2d_node_get_constant(arguments[0], &value)) {
                status = i2d_panic("failed to get item id");
            } else if(i2d_item_db_search_by_id(script->db->item_db, value, &item)) {
                status = i2d_panic("failed to get item by id -- %ld", value);
            } else if(i2d_string_stack_push(&context->expression_stack, item->name.string, item->name.length)) {
                status = i2d_panic("failed to push function string");
            }
        }

        if(!status) {
            if(i2d_node_get_constant(arguments[1], &value)) {
                status = i2d_panic("failed to get type");
            } else if(i2d_value_map_get(script->getiteminfo, &value, &string)) {
                status = i2d_panic("failed to get getiteminfo by type -- %ld", value);
            } else if(i2d_string_stack_push(&context->expression_stack, string.string, string.length)) {
                status = i2d_panic("failed to push getiteminfo string");
            } else {
                status = i2d_handler_general(script, node, context);
            }
        }
    }

    return status;
}

static int i2d_handler_getmapflag(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    i2d_node * arguments[3];
    long value;
    i2d_string string;
    i2d_constant * constant;

    if(i2d_node_get_arguments(node->left, arguments, 2, 1)) {
        status = i2d_panic("failed to get getmapflag arguments");
    } else {
        if(I2D_FUNCTION == arguments[0]->type) {
            if(i2d_node_get_string(arguments[0], &string)) {
                status = i2d_panic("failed to get function string");
            } else if(i2d_string_stack_push(&context->expression_stack, string.string, string.length)) {
                status = i2d_panic("failed to push function string");
            }
        } else {
            /*
             * to-do:
             * get map name
             */
        }

        if(!status) {
            if(i2d_node_get_constant(arguments[1], &value)) {
                status = i2d_panic("failed to get map flag");
            } else if(i2d_constant_get_by_mapflag(script->constant_db, &value, &constant)) {
                status = i2d_panic("failed to get constant by map flag -- %ld", value);
            } else if(i2d_string_stack_push(&context->expression_stack, constant->name.string, constant->name.length)) {
                status = i2d_panic("failed to push map flag string");
            } else {
                status = i2d_handler_general(script, node, context);
            }
        }
    }

    return status;
}

int i2d_script_bonus(i2d_script * script, i2d_block * block, i2d_context * context, i2d_data_map * bonus_map, int argc) {
    int status = I2D_OK;
    i2d_node * arguments[6];
    long value;
    i2d_data * data;

    size_t i;
    size_t size;
    i2d_string * types;
    i2d_handler * handler;

    if(argc >= i2d_size(arguments)) {
        status = i2d_panic("invalid paramaters");
    } else if(i2d_script_expression(script, block->nodes, 0, context)) {
        status = i2d_panic("failed to evaluate expression");
    } else if(i2d_node_get_arguments(block->nodes, arguments, 1 + argc, 0)) {
        status = i2d_panic("failed to get arguments");
    } else if(i2d_node_get_constant(arguments[0], &value)) {
        status = i2d_panic("failed to get bonus type");
    } else if(i2d_data_map_get(bonus_map, &value, &data)) {
        status = i2d_panic("failed to get bonus type data -- %ld", value);
    } else {
        if(i2d_string_stack_get(&data->types, &types, &size)) {
            status = i2d_panic("failed to get argument type array");
        } else if(!size) {
            status = i2d_panic("empty argument type array");
        } else if(size != argc) {
            status = i2d_panic("mismatch argument type array size and argument count");
        } else {
            i2d_string_stack_clear(&context->expression_stack);
            for(i = 0; i < size && !status; i++) {
                if(i2d_rbt_search(script->bonus_map, types[i].string, (void **) &handler)) {
                    status = i2d_panic("failed to find bonus handler -- %s", types[i].string);
                } else {
                    i2d_buffer_clear(&context->expression_buffer);

                    status = handler->handler(script, arguments[i + 1], context);
                }
            }

            if(!status)
                if(i2d_format_write(&data->format, &context->expression_stack, &block->buffer))
                    status = i2d_panic("failed to write bonus type description");
        }
    }

    return status;
}

static int i2d_bonus_handler_expression(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    i2d_string string;

    i2d_string_stack_clear(&context->predicate_stack);
    i2d_buffer_clear(&context->predicate_buffer);

    if( (node->left  && i2d_node_get_predicate_all(node->left,  &context->predicate_stack)) ||
        (node->right && i2d_node_get_predicate_all(node->right, &context->predicate_stack)) ||
        i2d_string_stack_get_unique(&context->predicate_stack, &context->predicate_buffer) ) {
        status = i2d_panic("failed to get predicate list");
    } else {
        i2d_buffer_get(&context->predicate_buffer, &string.string, &string.length);
        if(string.length && i2d_buffer_printf(&context->expression_buffer, " (%s)", string.string)) {
            status = i2d_panic("failed to write predicates");
        } else {
            i2d_buffer_get(&context->expression_buffer, &string.string, &string.length);
            if(i2d_string_stack_push(&context->expression_stack, string.string, string.length))
                status = i2d_panic("failed to push string on stack");
        }
    }

    return status;
}

static int i2d_bonus_handler_time(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    long min;
    long max;

    char * suffix = NULL;
    long unit = 0;

    i2d_range_get_range(&node->range, &min, &max);

    if (min / 86400000 > 0) {
        suffix = "day";
        unit = 86400000;
    } else if (min / 3600000 > 0) {
        suffix = "hour";
        unit = 3600000;
    } else if (min / 60000 > 0) {
        suffix = "minute";
        unit = 60000;
    } else if (min / 1000 > 0) {
        suffix = "second";
        unit = 1000;
    } else {
        suffix = "millisecond";
        unit = 1;
    }

    min /= unit;
    max /= unit;

    if( min == max ?
        i2d_buffer_printf(&context->expression_buffer, "%ld %s%s", min, suffix, min > 1 ? "s" : "") :
        i2d_buffer_printf(&context->expression_buffer, "%ld ~ %ld %s%s", min, max, suffix, max > 1 ? "s" : "") ) {
        status = i2d_panic("failed to write time range");
    } else if(i2d_bonus_handler_expression(script, node, context)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_bonus_handler_regen(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    long constant;

    if(i2d_node_get_constant(node, &constant)) {
        status = i2d_panic("failed to get constant");
    } else {
        switch(constant) {
            case 1: status = i2d_buffer_printf(&context->expression_buffer, "HP"); break;
            case 2: status = i2d_buffer_printf(&context->expression_buffer, "SP"); break;
            default: status = i2d_panic("unsupported regen value -- %ld", constant); break;
        }
        if(status) {
            status = i2d_panic("failed to write regen");
        } else if(i2d_bonus_handler_expression(script, node, context)) {
            status = i2d_panic("failed to write expression");
        }
    }

    return status;
}

static int i2d_bonus_handler_splash(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    long min;
    long max;

    i2d_range_get_range(&node->range, &min, &max);
    min = min * 2 + 1;
    max = max * 2 + 1;

    if( min == max ?
        i2d_buffer_printf(&context->expression_buffer, "%ld x %ld", min, min) :
        i2d_buffer_printf(&context->expression_buffer, "%ld x %ld ~ %ld x %ld", min, min, max, max) ) {
        status = i2d_panic("failed to write splash range");
    } else if(i2d_bonus_handler_expression(script, node, context)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_bonus_handler_elements(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    long value;
    i2d_constant * constant;

    if(i2d_node_get_constant(node, &value)) {
        status = i2d_panic("failed to get element value");
    } else if(i2d_constant_get_by_element(script->constant_db, &value, &constant)) {
        status = i2d_panic("failed to get element -- %ld", value);
    } else if(i2d_string_stack_push(&context->expression_stack, constant->name.string, constant->name.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_bonus_handler_races(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    long value;
    i2d_constant * constant;

    if(i2d_node_get_constant(node, &value)) {
        status = i2d_panic("failed to get race value");
    } else if(i2d_constant_get_by_race(script->constant_db, &value, &constant)) {
        status = i2d_panic("failed to get race -- %ld", value);
    } else if(i2d_string_stack_push(&context->expression_stack, constant->name.string, constant->name.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_bonus_handler_classes(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    long value;
    i2d_constant * constant;

    if(i2d_node_get_constant(node, &value)) {
        status = i2d_panic("failed to get class value");
    } else if(i2d_constant_get_by_class(script->constant_db, &value, &constant)) {
        status = i2d_panic("failed to get class -- %ld", value);
    } else if(i2d_string_stack_push(&context->expression_stack, constant->name.string, constant->name.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_bonus_handler_integer(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    long min;
    long max;

    i2d_range_get_range(&node->range, &min, &max);
    if( min == max ?
        i2d_buffer_printf(&context->expression_buffer, "%+ld", min) :
        i2d_buffer_printf(&context->expression_buffer, "%+ld ~ %+ld", min, max) ) {
        status = i2d_panic("failed to write integer range");
    } else if(i2d_bonus_handler_expression(script, node, context)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_bonus_handler_percent(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    long min;
    long max;

    i2d_range_get_range(&node->range, &min, &max);
    if( min == max ?
        i2d_buffer_printf(&context->expression_buffer, "%+ld%%", min) :
        i2d_buffer_printf(&context->expression_buffer, "%+ld%% ~ %+ld%%", min, max) ) {
        status = i2d_panic("failed to write percent range");
    } else if(i2d_bonus_handler_expression(script, node, context)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_bonus_handler_percent_invert(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    long min;
    long max;

    i2d_range_get_range(&node->range, &min, &max);

    min *= -1;
    max *= -1;

    if( min == max ?
        i2d_buffer_printf(&context->expression_buffer, "%+ld%%", min) :
        i2d_buffer_printf(&context->expression_buffer, "%+ld%% ~ %+ld%%", max, min) ) {
        status = i2d_panic("failed to write percent range");
    } else if(i2d_bonus_handler_expression(script, node, context)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_bonus_handler_percent100(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    long min;
    long max;

    i2d_range_get_range(&node->range, &min, &max);

    min /= 100;
    max /= 100;

    if( min == max ?
        i2d_buffer_printf(&context->expression_buffer, "%+ld%%", min) :
        i2d_buffer_printf(&context->expression_buffer, "%+ld%% ~ %+ld%%", max, min) ) {
        status = i2d_panic("failed to write percent range");
    } else if(i2d_bonus_handler_expression(script, node, context)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_bonus_handler_ignore(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;

    if(i2d_string_stack_push(&context->expression_stack, "ignore", 6))
        status = i2d_panic("failed to push string on stack");

    return status;
}

static int i2d_bonus_handler_sizes(i2d_script * script, i2d_node * node, i2d_context * context) {
    int status = I2D_OK;
    long value;
    i2d_constant * constant;

    if(i2d_node_get_constant(node, &value)) {
        status = i2d_panic("failed to get size value");
    } else if(i2d_constant_get_by_size(script->constant_db, &value, &constant)) {
        status = i2d_panic("failed to get size -- %ld", value);
    } else if(i2d_string_stack_push(&context->expression_stack, constant->name.string, constant->name.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}
