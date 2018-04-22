#include "i2d_script.h"

static int i2d_bonus_type_description_tokenize(i2d_bonus_type *, const char *, size_t);
static int i2d_bonus_type_description_load(i2d_bonus_type *, json_t *);
static int i2d_bonus_type_argument_type_map(enum i2d_bonus_argument_type *, json_t *);
static int i2d_bonus_type_argument_type_load(i2d_bonus_type *, json_t *);

static int i2d_translator_bonus_type_load(i2d_translator *, i2d_json *);
static int i2d_translator_const_load(i2d_translator *, i2d_json *);
static int i2d_translator_bonus_type_remap(i2d_translator *);

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
    "!",
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
            if(i2d_buf_init(&object->buffer, 64)) {
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
    i2d_deit(object->buffer, i2d_buf_deit);
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
    }
    i2d_deit(object, i2d_token_deit);
    *result = NULL;
}

int i2d_token_write(i2d_token * token, void * data, size_t size) {
    int status = I2D_OK;

    if( token->buffer->offset == token->buffer->length &&
        i2d_buf_fit(token->buffer, token->buffer->length * 2) ) {
        status = I2D_FAIL;
    } else if(i2d_buf_binary(token->buffer, data, size)) {
        status = i2d_panic("failed to write buffer object");
    }

    return status;
}

int i2d_token_get_literal(i2d_token * token, i2d_str * result) {
    int status = I2D_OK;

    if(I2D_LITERAL != token->type && I2D_POSITION != token->type) {
        status = I2D_FAIL;
    } else if(i2d_buf_add_null(token->buffer)) {
        status = i2d_panic("failed to write buffer object");
    } else {
        result->string = (char *) token->buffer->buffer;
        result->length = token->buffer->offset;
    }

    return status;
}

char i2d_token_get_last_symbol(i2d_token * token) {
    i2d_buf * buffer = token->buffer;
    return (buffer->offset > 0) ? buffer->buffer[buffer->offset - 1] : 0;
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
    i2d_str literal;
    char space;

    iterator = token;
    do {
        space = iterator->next == token ? '\n' : ' ';
        if(!i2d_token_get_literal(iterator, &literal)) {
            fprintf(stdout, "%s(%s)%c", i2d_token_string[iterator->type], literal.string, space);
        } else {
            fprintf(stdout, "%s%c", i2d_token_string[iterator->type], space);
        }
        iterator = iterator->next;
    } while(iterator != token);
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
        i2d_buf_zero(token->buffer);
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

int i2d_lexer_tokenize(i2d_lexer * lexer, i2d_str * script, i2d_token ** result) {
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
                    if('"' == symbol && '\\' != i2d_token_get_last_symbol(state)) {
                        state->type = I2D_LITERAL;
                    } else {
                        status = i2d_token_write(state, &symbol, sizeof(symbol));
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
                        status = i2d_token_write(state, &symbol, sizeof(symbol));
                    } else if('*' == i2d_token_get_last_symbol(state)) {
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
                    if(state && I2D_LITERAL == state->type && '$' != i2d_token_get_last_symbol(state)) {
                        status = i2d_token_write(state, "$", 1);
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
                                    status = i2d_token_write(state, &symbol, sizeof(symbol));
                                    break;
                                case I2D_TEMPORARY_CHARACTER:
                                    status = i2d_token_write(state, "@", 1) ||
                                             i2d_token_write(state, &symbol, sizeof(symbol));
                                    state->type = I2D_LITERAL;
                                    break;
                                case I2D_PERMANENT_GLOBAL:
                                    status = i2d_token_write(state, "$", 1) ||
                                             i2d_token_write(state, &symbol, sizeof(symbol));
                                    state->type = I2D_LITERAL;
                                    break;
                                case I2D_TEMPORARY_GLOBAL:
                                    status = i2d_token_write(state, "$@", 2) ||
                                             i2d_token_write(state, &symbol, sizeof(symbol));
                                    state->type = I2D_LITERAL;
                                    break;
                                case I2D_TEMPORARY_NPC:
                                    status = i2d_token_write(state, ".", 1) ||
                                             i2d_token_write(state, &symbol, sizeof(symbol));
                                    state->type = I2D_LITERAL;
                                    break;
                                case I2D_TEMPORARY_SCOPE:
                                    status = i2d_token_write(state, ".@", 2) ||
                                             i2d_token_write(state, &symbol, sizeof(symbol));
                                    state->type = I2D_LITERAL;
                                    break;
                                case I2D_TEMPORARY_INSTANCE:
                                    status = i2d_token_write(state, "'", 1) ||
                                             i2d_token_write(state, &symbol, sizeof(symbol));
                                    state->type = I2D_LITERAL;
                                    break;
                                case I2D_PERMANENT_ACCOUNT_LOCAL:
                                    status = i2d_token_write(state, "#", 1) ||
                                             i2d_token_write(state, &symbol, sizeof(symbol));
                                    state->type = I2D_LITERAL;
                                    break;
                                case I2D_PERMANENT_ACCOUNT_GLOBAL:
                                    status = i2d_token_write(state, "##", 2) ||
                                             i2d_token_write(state, &symbol, sizeof(symbol));
                                    state->type = I2D_LITERAL;
                                    break;
                                default:
                                    status = i2d_lexer_token_init(lexer, &token, I2D_LITERAL) ||
                                             i2d_token_write(token, &symbol, sizeof(symbol));
                                    break;
                            }
                        } else {
                            status = i2d_lexer_token_init(lexer, &token, I2D_LITERAL) ||
                                     i2d_token_write(token, &symbol, sizeof(symbol));
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
    i2d_deit(object->range, i2d_range_list_deit);
    i2d_deit(object->tokens, i2d_token_list_deit);
    i2d_deit(object->right, i2d_node_deit);
    i2d_deit(object->left, i2d_node_deit);
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

    i2d_token_print(node->tokens);

    if(node->range) {
        for(i = 0; i < level; i++)
            fprintf(stdout, "    ");

        i2d_range_list_print(node->range, "range");
    }

    if(node->left)
        i2d_node_print(node->left, level + 1);
    if(node->right)
        i2d_node_print(node->right, level + 1);
}

int i2d_node_cmp_literal(void * left, void * right) {
    int status;
    i2d_str literal_left;
    i2d_str literal_right;

    if( i2d_token_get_literal(left, &literal_left) ||
        i2d_token_get_literal(right, &literal_right) ) {
        status = i2d_panic("failed to get literal");
    } else {
        status = strcmp(literal_left.string, literal_right.string);
    }

    return status;
}

const char * i2d_statement_string[] = {
    "start",
    "bonus",
    "bonus2",
    "bonus3",
    "bonus4",
    "bonus5",
    "autobonus",
    "autobonus2",
    "autobonus3",
    "heal",
    "percentheal",
    "itemheal",
    "skill",
    "itemskill",
    "unitskilluseid",
    "sc_start",
    "sc_start4",
    "sc_end",
    "getitem",
    "rentitem",
    "delitem",
    "getrandgroupitem",
    "skilleffect",
    "specialeffect2",
    "setfont",
    "buyingstore",
    "searchstores",
    "set",
    "input",
    "announce",
    "callfunc",
    "end",
    "warp",
    "pet",
    "bpet",
    "mercenary_create",
    "mercenary_heal",
    "mercenary_sc_start",
    "produce",
    "cooking",
    "makerune",
    "guildgetexp",
    "getexp",
    "monster",
    "homevolution",
    "setoption",
    "setmounting",
    "setfalcon",
    "getgroupitem",
    "resetstatus",
    "bonus_script",
    "playbgm",
    "transform",
    "sc_start2",
    "petloot",
    "petrecovery",
    "petskillbonus",
    "petskillattack",
    "petskillattack2",
    "petskillsupport",
    "petheal",
    "for",
    "getmapxy",
    "specialeffect",
    "showscript",
    "end"
};

int i2d_statement_init(i2d_statement ** result, enum i2d_statement_type type) {
    int status = I2D_OK;
    i2d_statement * object = NULL;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            object->type = type;
            if(i2d_str_init(&object->name, i2d_statement_string[type], strlen(i2d_statement_string[type]))) {
                status = i2d_panic("failed to create string object");
            } else {
                object->next = object;
                object->prev = object;
            }

            if(status)
                i2d_statement_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_statement_deit(i2d_statement ** result) {
    i2d_statement * object;

    object = *result;
    i2d_deit(object->name, i2d_str_deit);
    i2d_free(object);
    *result = NULL;
}

void i2d_statement_list_deit(i2d_statement ** result) {
    i2d_statement * object;
    i2d_statement * statement;

    object = *result;
    if(object) {
        while(object != object->next) {
            statement = object->next;
            i2d_statement_remove(statement);
            i2d_statement_deit(&statement);
        }
    }
    i2d_deit(object, i2d_statement_deit);
    *result = NULL;
}

void i2d_statement_append(i2d_statement * x, i2d_statement * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_statement_remove(i2d_statement * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

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
            if(i2d_buf_init(&object->buffer, 4096)) {
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
    i2d_deit(object->buffer, i2d_buf_deit);
    i2d_deit(object->child, i2d_block_list_deit);
    i2d_deit(object->nodes, i2d_node_deit);
    i2d_deit(object->tokens, i2d_token_list_deit);
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
        fprintf(stdout, "%s [%p]\n", block->statement->name->string, block);
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

                if(i2d_parser_statement_load(object))
                    status = i2d_panic("failed to load statement objects");
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
    i2d_deit(object->statement_index, i2d_rbt_deit);
    i2d_deit(object->statement_list, i2d_statement_list_deit);
    i2d_deit(object->node_cache, i2d_node_list_deit);
    i2d_deit(object->block_cache, i2d_block_list_deit);
    i2d_free(object);
    *result = NULL;
}

int i2d_parser_statement_load(i2d_parser * parser) {
    int status = I2D_OK;
    size_t i;
    i2d_statement * statement;

    if(i2d_statement_init(&parser->statement_list, I2D_STATEMENT_START)) {
        status = i2d_panic("failed to create statement object");
    } else if(i2d_rbt_init(&parser->statement_index, i2d_rbt_cmp_str)) {
        status = i2d_panic("failed to create red black tree object");
    } else {
        for(i = I2D_STATEMENT_START + 1; i < I2D_STATEMENT_END && !status; i++) {
            statement = NULL;
            if(i2d_statement_init(&statement, i)) {
                status = i2d_panic("failed to create statement object");
            } else {
                i2d_statement_append(statement, parser->statement_list);
                if(i2d_rbt_insert(parser->statement_index, statement->name, statement))
                    status = i2d_panic("failed to index statement object");
            }
        }
    }

    return status;
}

int i2d_parser_statement_map(i2d_parser * parser, i2d_lexer * lexer, i2d_block * block) {
    int status = I2D_OK;
    i2d_token * token;
    i2d_str name;
    i2d_zero(name);

    token = block->tokens->next;
    if(I2D_LITERAL == token->type) {
        if(i2d_token_get_literal(token, &name)) {
            status = i2d_panic("failed to get literal");
        } else {
            if(!i2d_rbt_search(parser->statement_index, &name, (void *) &block->statement)) {
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
        i2d_buf_zero(block->buffer);
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
    i2d_deit(node->range, i2d_range_list_deit);
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
    i2d_token * anchor;
    i2d_token * token = NULL;
    i2d_str literal;
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
            if(i2d_token_get_literal(tokens, &literal)) {
                status = i2d_panic("failed to get literal");
            } else if(!strcmp("if", literal.string)) {
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
            } else if(!strcmp("else", literal.string)) {
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
                i2d_block_append(block, root);
            }
            block = NULL;

            if(parent && (I2D_IF == parent->type))
                break;
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

static int i2d_bonus_type_description_tokenize(i2d_bonus_type * bonus_type, const char * string, size_t length) {
    int status = I2D_OK;
    size_t i;
    char symbol;
    i2d_token * token = NULL;
    i2d_token * state = NULL;
    int curly_level = 0;

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
                    status = i2d_token_write(state, &symbol, sizeof(symbol));
                } else {
                    status = i2d_token_init(&token, curly_level ? I2D_POSITION : I2D_LITERAL) ||
                             i2d_token_write(token, &symbol, sizeof(symbol));
                }
                break;
        }

        if(token) {
            i2d_token_append(token, bonus_type->tokens);
            state = token;
            token = NULL;
        }
    }

    return status;
}

static int i2d_bonus_type_description_load(i2d_bonus_type * bonus_type, json_t * json) {
    int status = I2D_OK;
    json_t * description;
    const char * string;
    size_t length;

    description = json_object_get(json, "description");
    if(!description) {
        status = i2d_panic("failed to get description key value");
    } else {
        string = json_string_value(description);
        if(!string) {
            status = i2d_panic("failed to get description string");
        } else {
            length = json_string_length(description);
            if(!length) {
                status = i2d_panic("failed on empty description string");
            } else {
                if(i2d_token_init(&bonus_type->tokens, I2D_TOKEN)) {
                    status = i2d_panic("failed to create token object");
                } else if(i2d_bonus_type_description_tokenize(bonus_type, string, length)) {
                    status = i2d_panic("failed to tokenize description string");
                }
            }
        }
    }

    return status;
}

static int i2d_bonus_type_argument_type_map(enum i2d_bonus_argument_type * type, json_t * json) {
    int status = I2D_OK;
    const char * string;
    size_t length;

    string = json_string_value(json);
    if(!string) {
        status = i2d_panic("failed to get argument type string");
    } else {
        length = json_string_length(json);
        if(!length) {
            status = i2d_panic("failed on empty argument type string");
        } else {
            if(!strcmp(string, "elemental")) {
                *type = I2D_ELEMENTAL;
            } else {
                status = i2d_panic("unsupported argument type -- %s", string);
            }
        }
    }

    return status;
}

static int i2d_bonus_type_argument_type_load(i2d_bonus_type * bonus_type, json_t * json) {
    int status = I2D_OK;
    size_t i;
    size_t size;
    json_t * argument_type_array;
    json_t * argument_type;

    argument_type_array = json_object_get(json, "argument_type");
    if(!argument_type_array) {
        status = i2d_panic("failed to get argument type key value");
    } else {
        size = json_array_size(argument_type_array);
        if(!size) {
            status = i2d_panic("failed on empty argument type array");
        } else if(size > i2d_size(bonus_type->type)) {
            status = i2d_panic("failed on insufficient bonus type array");
        } else {
            for(i = 0; i < size && !status; i++) {
                argument_type = json_array_get(argument_type_array, i);
                if(!argument_type) {
                    status = i2d_panic("failed to get argument type string");
                } else if(i2d_bonus_type_argument_type_map(&bonus_type->type[i], argument_type)) {
                    status = i2d_panic("failed to map argument type");
                }
            }
        }
    }

    return status;
}

int i2d_bonus_type_init(i2d_bonus_type ** result, const char * name, json_t * json) {
    int status = I2D_OK;
    i2d_bonus_type * object;
    json_t * argument_type;

    if(i2d_is_invalid(result) || !name) {
        status = i2d_panic("invalid paramaters");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_str_init(&object->name, name, strlen(name))) {
                status = i2d_panic("failed to create string object");
            } else if(i2d_bonus_type_description_load(object, json)) {
                status = i2d_panic("failed to load bonus type description");
            } else if(i2d_bonus_type_argument_type_load(object, json)) {
                status = i2d_panic("failed to load argument type");
            }

            if(status)
                i2d_bonus_type_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_bonus_type_deit(i2d_bonus_type ** result) {
    i2d_bonus_type * object;

    object = *result;
    i2d_deit(object->tokens, i2d_token_list_deit);
    i2d_deit(object->name, i2d_str_deit);
    i2d_free(object);
    *result = NULL;
}

int i2d_bonus_type_format(i2d_bonus_type * bonus_type, i2d_str ** list, size_t size, i2d_buf * buffer) {
    int status = I2D_OK;
    i2d_token * token;
    i2d_str literal;
    long index;

    if(!bonus_type->tokens) {
        status = i2d_panic("failed on empty bonus type description");
    } else {
        token = bonus_type->tokens->next;
        while(token->type != I2D_TOKEN && !status) {
            switch(token->type) {
                case I2D_LITERAL:
                    if(i2d_token_get_literal(token, &literal)) {
                        status = i2d_panic("failed to get literal");
                    } else if(i2d_buf_format(buffer, "%s", literal.string)) {
                        status = i2d_panic("failed to format buffer");
                    }
                    break;
                case I2D_POSITION:
                    if(i2d_token_get_literal(token, &literal)) {
                        status = i2d_panic("failed to get literal");
                    } else if(i2d_strtol(&index, literal.string, literal.length, 10)) {
                        status = i2d_panic("failed to convert literal to number");
                    } else if(index >= size) {
                        status = i2d_panic("failed on insufficient string list");
                    } else if(i2d_buf_format(buffer, "%s", list[index]->string)) {
                        status = i2d_panic("failed to format buffer");
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

int i2d_const_init(i2d_const ** result, const char * name, json_t * json) {
    int status = I2D_OK;
    i2d_const * object;

    if(i2d_is_invalid(result) || !name || !json) {
        status = i2d_panic("invalid paramaters");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_str_init(&object->name, name, strlen(name))) {
                status = i2d_panic("failed to create string object");
            } else {
                object->value = json_integer_value(json);
            }
            if(status)
                i2d_const_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_const_deit(i2d_const ** result) {
    i2d_const * object;

    object = *result;
    i2d_deit(object->name, i2d_str_deit);
    i2d_free(object);
    *result = NULL;
}

int i2d_str_map_load(i2d_str_map * str_map, json_t * json) {
    int status = I2D_OK;
    size_t i = 0;
    const char * key;
    json_t * value;

    const char * name;
    size_t length;

    str_map->size = json_object_size(json) * 2;
    if(!str_map->size) {
        status = i2d_panic("failed on empty map object");
    } else {
        str_map->list = calloc(str_map->size, sizeof(*str_map->list));
        if(!str_map->list) {
            status = i2d_panic("out of memory");
        } else {
            json_object_foreach(json, key, value) {
                if(i2d_str_init(&str_map->list[i], key, strlen(key))) {
                    status = i2d_panic("failed to create string object");
                } else if(!json_string_value(value) || !json_string_length(value)) {
                    status = i2d_panic("failed on invalid string object");
                } else if(i2d_str_init(&str_map->list[i + 1], json_string_value(value), json_string_length(value))) {
                    status = i2d_panic("failed to create string object");
                } else if(i2d_rbt_insert(str_map->map, str_map->list[i], str_map->list[i + 1])) {
                    status = i2d_panic("failed to map string object");
                } else {
                    i += 2;
                }
            }
        }
    }

    return status;
}

int i2d_str_map_init(i2d_str_map ** result, const char * key, json_t * json) {
    int status = I2D_OK;
    i2d_str_map * object;
    json_t * map;

    if(i2d_is_invalid(result) || !key || !json) {
        status = i2d_panic("invalid paramaters");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_rbt_init(&object->map, i2d_rbt_cmp_str)) {
                status = i2d_panic("failed to create red black tree object");
            } else {
                map = json_object_get(json, key);
                if(!map) {
                    status = i2d_panic("failed to get %s key value", key);
                } else if(i2d_str_map_load(object, map)) {
                    status = i2d_panic("failed to load string map object");
                }
            }
            if(status)
                i2d_str_map_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_str_map_deit(i2d_str_map ** result) {
    i2d_str_map * object;
    size_t i;

    object = *result;
    if(object->list) {
        for(i = 0; i < object->size; i++)
            i2d_deit(object->list[i], i2d_str_deit);
        i2d_free(object->list);
    }
    i2d_deit(object->map, i2d_rbt_deit);
    i2d_free(object);
    *result = NULL;
}

int i2d_str_map_map(i2d_str_map * str_map, i2d_str * key, i2d_str ** result) {
    int status = I2D_OK;
    i2d_str * value;

    if(i2d_rbt_search(str_map->map, key, (void **) &value)) {
        status = I2D_FAIL;
    } else if(i2d_str_init(result, value->string, value->length)) {
        status = i2d_panic("failed to create string object");
    }

    return status;
}

static int i2d_translator_bonus_type_load(i2d_translator * translator, i2d_json * json) {
    int status = I2D_OK;
    json_t * bonus = NULL;
    size_t i = 0;
    const char * key;
    json_t * value;

    if(i2d_json_block_map(json, "bonus", &bonus)) {
        status = i2d_panic("failed to get bonus block key value");
    } else {
        translator->bonus_size = json_object_size(bonus);
        if(!translator->bonus_size) {
            status = i2d_panic("failed on empty bonus type array");
        } else {
            translator->bonus_list = calloc(translator->bonus_size, sizeof(*translator->bonus_list));
            if(!translator->bonus_list) {
                status = i2d_panic("out of memory");
            } else {
                json_object_foreach(bonus, key, value) {
                    if(i2d_bonus_type_init(&translator->bonus_list[i], key, value)) {
                        status = i2d_panic("failed to create bonus type object");
                    } else {
                        i++;
                    }
                }
            }
        }
    }

    if(!status) {
        if(i2d_rbt_init(&translator->bonus_map, i2d_rbt_cmp_str)) {
            status = i2d_panic("failed to create red black tree object");
        } else {
            for(i = 0; i < translator->bonus_size; i++)
                if(i2d_rbt_insert(translator->bonus_map, translator->bonus_list[i]->name, translator->bonus_list[i]))
                    status = i2d_panic("failed to index bonus type object");
        }
    }

    return status;
}

static int i2d_translator_const_load(i2d_translator * translator, i2d_json * json) {
    int status = I2D_OK;
    json_t * consts;
    size_t i = 0;
    const char * key;
    json_t * value;

    consts = json_object_get(json->object, "consts");
    if(!consts) {
        status = i2d_panic("failed to get consts key value");
    } else {
        translator->const_size = json_object_size(consts);
        if(!translator->const_size) {
            status = i2d_panic("failed on empty const array");
        } else {
            translator->const_list = calloc(translator->const_size, sizeof(*translator->const_list));
            if(!translator->const_list) {
                status = i2d_panic("out of memory");
            } else {
                json_object_foreach(consts, key, value) {
                    if(i2d_const_init(&translator->const_list[i], key, value)) {
                        status = i2d_panic("failed to create const object");
                    } else {
                        i++;
                    }
                }
            }
        }
    }

    if(!status) {
        if(i2d_rbt_init(&translator->const_map, i2d_rbt_cmp_str)) {
            status = i2d_panic("failed to create red black tree object");
        } else {
            for(i = 0; i < translator->const_size; i++)
                if(i2d_rbt_insert(translator->const_map, translator->const_list[i]->name, translator->const_list[i]))
                    status = i2d_panic("failed to index const object");
        }
    }

    return status;
}

static int i2d_translator_bonus_type_remap(i2d_translator * translator) {
    int status = I2D_OK;
    size_t i;
    i2d_rbt * bonus_map = NULL;

    if(i2d_rbt_init(&bonus_map, i2d_rbt_cmp_long)) {
        status = i2d_panic("failed to create red black tree object");
    } else {
        for(i = 0; i < translator->bonus_size; i++) {
            if(i2d_translator_const_map(translator, translator->bonus_list[i]->name, &translator->bonus_list[i]->value)) {
                status = i2d_panic("failed to remap bonus type -- %s", translator->bonus_list[i]->name->string);
            } else {
                if(i2d_rbt_insert(bonus_map, &translator->bonus_list[i]->value, translator->bonus_list[i]))
                    status = i2d_panic("failed to index bonus type object");
            }
        }

        if(status) {
            i2d_rbt_deit(&bonus_map);
        } else {
            i2d_rbt_deit(&translator->bonus_map);
            translator->bonus_map = bonus_map;
        }
    }

    return status;
}

int i2d_translator_init(i2d_translator ** result, i2d_json * json) {
    int status = I2D_OK;
    i2d_translator * object;
    size_t i;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_translator_bonus_type_load(object, json)) {
                status = i2d_panic("failed to load bonus type");
            } else if(i2d_translator_const_load(object, json)) {
                status = i2d_panic("failed to load consts");
            } else if(i2d_translator_bonus_type_remap(object)) {
                status = i2d_panic("failed to remap bonus type");
            } else if(i2d_str_map_init(&object->elements, "elements", json->object)) {
                status = i2d_panic("failed to load element map object");
            }

            if(status)
                i2d_translator_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_translator_deit(i2d_translator ** result) {
    i2d_translator * object;
    size_t i;

    object = *result;
    i2d_deit(object->elements, i2d_str_map_deit);
    if(object->const_list) {
        for(i = 0; i < object->const_size; i++)
            i2d_deit(object->const_list[i], i2d_const_deit);
        i2d_free(object->const_list);
    }
    i2d_deit(object->const_map, i2d_rbt_deit);
    if(object->bonus_list) {
        for(i = 0; i < object->bonus_size; i++)
            i2d_deit(object->bonus_list[i], i2d_bonus_type_deit);
        i2d_free(object->bonus_list);
    }
    i2d_deit(object->bonus_map, i2d_rbt_deit);
    i2d_free(object);
    *result = NULL;
}

int i2d_translator_bonus_map(i2d_translator * translator, long * key, i2d_bonus_type ** result) {
    return i2d_rbt_search(translator->bonus_map, key, (void **) result);
}

int i2d_translator_const_map(i2d_translator * translator, i2d_str * key, long * result) {
    int status = I2D_OK;
    i2d_const * constant;

    if(i2d_rbt_search(translator->const_map, key, (void **) &constant)) {
        status = I2D_FAIL;
    } else {
        *result = constant->value;
    }

    return status;
}

int i2d_translator_bonus_type(i2d_translator * translator, enum i2d_bonus_argument_type type, i2d_node * node, i2d_str ** result) {
    int status = I2D_OK;
    i2d_str literal;

    switch(type) {
        case I2D_ELEMENTAL:
            if(I2D_VARIABLE != node->type) {
                status = i2d_panic("invalid node type -- %d", node->type);
            } else if(i2d_token_get_literal(node->tokens, &literal)) {
                status = i2d_panic("failed to get literal");
            } else if(i2d_str_map_map(translator->elements, &literal, result)) {
                status = i2d_panic("failed to map element -- %s", literal.string);
            }
            break;
        default:
            status = i2d_panic("invalid block argument type -- %d", type);
            break;
    }

    return status;
}

int i2d_block_get_arguments(i2d_block * block, i2d_node ** nodes, size_t size) {
    int status = I2D_OK;
    size_t i;
    i2d_node * node;

    if(!block->nodes->left) {
        status = i2d_panic("failed on empty argument list");
    } else {
        i = size - 1;
        node = block->nodes->left;
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

    return status;
}

int i2d_node_get_constant(i2d_node * node, long * result) {
    int status = I2D_OK;
    long min;
    long max;

    i2d_range_list_get_range(node->range, &min, &max);

    if(min != max) {
        status = i2d_panic("failed on invalid range");
    } else {
        *result = min;
    }

    return status;
}

int i2d_node_get_string(i2d_node * node, i2d_str * result) {
    int status = I2D_OK;

    if(I2D_VARIABLE != node->type) {
        status = i2d_panic("failed on invalid node type -- %d", node->type);
    } else if(i2d_token_get_literal(node->tokens, result)) {
        status = i2d_panic("failed to get literal");
    }

    return status;
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
            if(i2d_rbt_init(&object->variables, i2d_node_cmp_literal)) {
                status = i2d_panic("failed to create red black tree object");
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

int i2d_context_reset(i2d_context * context) {
    int status = I2D_OK;

    if(i2d_rbt_clear(context->variables))
        status = i2d_panic("failed to clear variable map");

    return status;
}

int i2d_context_insert_variable(i2d_context * context, i2d_node * node) {
    int status = I2D_OK;
    i2d_node * last;

    if(I2D_VARIABLE != node->type) {
        status = i2d_panic("invalid node type -- %d", node->type);
    } else {
        if( !i2d_rbt_search(context->variables, node->tokens, (void **) &last) &&
            i2d_rbt_delete(context->variables, last) ) {
            status = i2d_panic("failed to replace existing variable");
        } else if(i2d_rbt_insert(context->variables, node->tokens, node)) {
            status = i2d_panic("failed to map variable");
        }
    }

    return status;
}

int i2d_context_search_variable(i2d_context * context, i2d_node * node, i2d_node ** result) {
    int status = I2D_OK;

    if(I2D_VARIABLE != node->type) {
        status = i2d_panic("invalid node type -- %d", node->type);
    } else {
        status = i2d_rbt_search(context->variables, node->tokens, (void **) result);
    }

    return status;
}

int i2d_script_init(i2d_script ** result, i2d_str * path) {
    int status = I2D_OK;
    i2d_script * object;

    if(i2d_is_invalid(result) || !path) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_json_init(&object->json, path)) {
                status = i2d_panic("failed to create json object");
            } else if(i2d_lexer_init(&object->lexer)) {
                status = i2d_panic("failed to create lexer object");
            } else if(i2d_parser_init(&object->parser)) {
                status = i2d_panic("failed to create parser object");
            } else if(i2d_translator_init(&object->translator, object->json)) {
                status = i2d_panic("failed to create translator object");
            } else if(i2d_context_init(&object->context)) {
                status = i2d_panic("failed to create context object");
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
    i2d_deit(object->context, i2d_context_list_deit);
    i2d_deit(object->translator, i2d_translator_deit);
    i2d_deit(object->parser, i2d_parser_deit);
    i2d_deit(object->lexer, i2d_lexer_deit);
    i2d_deit(object->json, i2d_json_deit);
    i2d_free(object);
    *result = NULL;
}

int i2d_script_compile(i2d_script * script, i2d_str * source, i2d_str ** target) {
    int status = I2D_OK;
    i2d_token * tokens = NULL;
    i2d_block * blocks = NULL;

    if(!strcmp("{}", source->string)) {
        status = i2d_str_init(target, "", 0);
    } else if(i2d_context_reset(script->context)) {
        status = i2d_panic("failed to reset context object");
    } else if(i2d_lexer_tokenize(script->lexer, source, &tokens)) {
        status = i2d_panic("failed to lex -- %s", source->string);
    } else if(i2d_parser_analysis(script->parser, script->lexer, tokens, &blocks)) {
        status = i2d_panic("failed to parse -- %s", source->string);
    } else if(i2d_script_translate(script, blocks)) {
        status = i2d_panic("failed to translate -- %s", source->string);
    }

    if(tokens)
        i2d_lexer_reset(script->lexer, &tokens);

    if(blocks)
        i2d_parser_reset(script->parser, script->lexer, &blocks);

    return status;
}

int i2d_script_translate(i2d_script * script, i2d_block * list) {
    int status = I2D_OK;
    i2d_block * block;

    if(list) {
        block = list;
        do {
            switch(block->type) {
                case I2D_BLOCK:
                    status = i2d_script_translate(script, block->child);
                    break;
                case I2D_STATEMENT:
                    status = i2d_script_statement(script, block);
                    break;
                case I2D_IF:
                    break;
                case I2D_ELSE:
                    break;
                default:
                    status = i2d_panic("invalid block type -- %d", block->type);
                    break;
            }
            block = block->next;
        } while(block != list && !status);
    }

    return status;
}

int i2d_script_statement(i2d_script * script, i2d_block * block) {
    int status = I2D_OK;
    json_t * block_data = NULL;

    if(block->statement) {
        switch(block->statement->type) {
            case I2D_BONUS:
                status = i2d_script_bonus(script, block);
                break;
        }
    }

    return status;
}

int i2d_script_bonus(i2d_script * script, i2d_block * block) {
    int status = I2D_OK;
    i2d_node * arguments[2];
    long bonus_id;
    i2d_bonus_type * bonus_type;
    i2d_str * string = NULL;

    if(i2d_script_expression(script, block->nodes, 0)) {
        status = i2d_panic("failed to translate expression");
    } else if(i2d_block_get_arguments(block, arguments, i2d_size(arguments))) {
        status = i2d_panic("failed to get arguments");
    } else if(i2d_node_get_constant(arguments[0], &bonus_id)) {
        status = i2d_panic("failed to get bonus type value");
    } else if(i2d_translator_bonus_map(script->translator, &bonus_id, &bonus_type)) {
        status = i2d_panic("failed to map bonus type value");
    } else if(i2d_translator_bonus_type(script->translator, bonus_type->type[0], arguments[1], &string)) {
        status = i2d_panic("failed to translate bonus arguments");
    } else {
        if(i2d_bonus_type_format(bonus_type, &string, 1, block->buffer))
            status = i2d_panic("failed to format bonus type");
        i2d_str_deit(&string);
    }

    return status;
}

int i2d_script_expression(i2d_script * script, i2d_node * node, int is_conditional) {
    int status = I2D_OK;

    if(node->left && i2d_script_expression(script, node->left, is_conditional)) {
        status = i2d_panic("failed to evaluate left expression");
    } else if(node->right && i2d_script_expression(script, node->right, is_conditional)) {
        status = i2d_panic("failed to evaluate right expression");
    } else {
        switch(node->type) {
            case I2D_NODE:
                break;
            case I2D_VARIABLE:
                status = i2d_script_expression_variable(script, node);
                break;
            case I2D_FUNCTION:
                break;
            case I2D_UNARY:
                status = i2d_script_expression_unary(script, node, is_conditional);
                break;
            case I2D_BINARY:
                status = i2d_script_expression_binary(script, node, is_conditional);
                break;
            default:
                status = i2d_panic("invalid node type -- %d", node->type);
                break;
        }
    }

    return status;
}

int i2d_script_expression_variable(i2d_script * script, i2d_node * node) {
    int status = I2D_OK;
    i2d_node * variable;
    int base;
    long number;
    i2d_str literal;
    i2d_zero(literal);

    if(i2d_token_get_literal(node->tokens, &literal)) {
        status = i2d_panic("failed to get literal");
    } else {
        if(!i2d_context_search_variable(script->context, node, &variable)) {
            if(i2d_range_list_copy(&node->range, variable->range))
                status = i2d_panic("failed to copy range list object");
        } else {
            if(isdigit(literal.string[0])) {
                if(!strncmp(literal.string, "0x", 2) && literal.length > 2) {
                    base = 16;
                } else if(literal.string[0] == '0' && literal.length > 1) {
                    base = 8;
                } else {
                    base = 10;
                }

                if(i2d_strtol(&number, literal.string, literal.length, base))
                    status = i2d_panic("failed to parse hexadecimal number -- %s", literal.string);
            } else if(i2d_translator_const_map(script->translator, &literal, &number)) {
                number = 0;
            }

            if(i2d_range_list_init(&node->range)) {
                status = i2d_panic("failed to create range list object");
            } else if(i2d_range_list_add(node->range, number, number)) {
                status = i2d_panic("failed to add range to range list");
            }
        }
    }

    return status;
}

int i2d_script_expression_unary(i2d_script * script, i2d_node * node, int is_conditional) {
    int status = I2D_OK;

    if(!node->right || !node->right->range) {
        status = i2d_panic("unary operator missing operand");
    } else {
        switch(node->tokens->type) {
            case I2D_NOT:
                if(is_conditional) {
                    status = i2d_range_list_not(&node->range, node->right->range);
                } else {
                    status = i2d_range_list_init(&node->range) || i2d_range_list_add(node->range, 0, 1);
                }
                break;
            case I2D_ADD_UNARY:
                status = i2d_range_list_copy(&node->range, node->right->range);
                break;
            case I2D_SUBTRACT_UNARY:
                status = i2d_range_list_negate(&node->range, node->right->range);
                break;
            default:
                status = i2d_panic("invalid node type -- %d", node->tokens->type);
        }
    }

    return status;
}

int i2d_script_expression_binary(i2d_script * script, i2d_node * node, int is_conditional) {
    int status = I2D_OK;

    if(!node->left || !node->left->range) {
        status = i2d_panic("binary operator missing left operand");
    } else if(!node->right || !node->right->range) {
        status = i2d_panic("binary operator missing right operand");
    } else {
        switch(node->tokens->type) {
            case I2D_ADD:
            case I2D_ADD_ASSIGN:
                status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '+');
                break;
            case I2D_SUBTRACT:
            case I2D_SUBTRACT_ASSIGN:
                status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '-');
                break;
            case I2D_MULTIPLY:
            case I2D_MULTIPLY_ASSIGN:
                status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '*');
                break;
            case I2D_DIVIDE:
            case I2D_DIVIDE_ASSIGN:
                status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '/');
                break;
            case I2D_MODULUS:
            case I2D_MODULUS_ASSIGN:
                status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '%');
                break;
            case I2D_COMMA:
                status = i2d_range_list_copy(&node->range, node->right->range);
                break;
            case I2D_RIGHT_SHIFT:
            case I2D_RIGHT_SHIFT_ASSIGN:
                status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '>' + '>' + 'b');
                break;
            case I2D_LEFT_SHIFT:
            case I2D_LEFT_SHIFT_ASSIGN:
                status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '<' + '<' + 'b');
                break;
            case I2D_BIT_AND:
            case I2D_BIT_AND_ASSIGN:
                status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '&');
                break;
            case I2D_BIT_OR:
            case I2D_BIT_OR_ASSIGN:
                status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '|');
                break;
            case I2D_BIT_XOR:
            case I2D_BIT_XOR_ASSIGN:
                status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '^' + 'b');
                break;
            case I2D_GREATER:
                if(is_conditional) {
                    status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '>');
                } else {
                    status = i2d_range_list_init(&node->range) || i2d_range_list_add(node->range, 0, 1);
                }
                break;
            case I2D_GREATER_EQUAL:
                if(is_conditional) {
                    status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '>' + '=');
                } else {
                    status = i2d_range_list_init(&node->range) || i2d_range_list_add(node->range, 0, 1);
                }
                break;
            case I2D_LESS:
                if(is_conditional) {
                    status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '<');
                } else {
                    status = i2d_range_list_init(&node->range) || i2d_range_list_add(node->range, 0, 1);
                }
                break;
            case I2D_LESS_EQUAL:
                if(is_conditional) {
                    status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '<' + '=');
                } else {
                    status = i2d_range_list_init(&node->range) || i2d_range_list_add(node->range, 0, 1);
                }
                break;
            case I2D_EQUAL:
                if(is_conditional) {
                    status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '=' + '=');
                } else {
                    status = i2d_range_list_init(&node->range) || i2d_range_list_add(node->range, 0, 1);
                }
                break;
            case I2D_NOT_EQUAL:
                if(is_conditional) {
                    status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '!' + '=');
                } else {
                    status = i2d_range_list_init(&node->range) || i2d_range_list_add(node->range, 0, 1);
                }
                break;
            case I2D_AND:
                if(is_conditional) {
                    status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '&' + '&');
                } else {
                    status = i2d_range_list_init(&node->range) || i2d_range_list_add(node->range, 0, 1);
                }
                break;
            case I2D_OR:
                if(is_conditional) {
                    status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '|' + '|');
                } else {
                    status = i2d_range_list_init(&node->range) || i2d_range_list_add(node->range, 0, 1);
                }
                break;
            case I2D_ASSIGN:
                status = i2d_range_list_copy(&node->range, node->right->range);
                break;
            default:
                status = i2d_panic("invalid token type -- %d", node->tokens->type);
                break;
        }

        if(!status) {
            switch(node->tokens->type) {
                case I2D_ADD_ASSIGN:
                case I2D_SUBTRACT_ASSIGN:
                case I2D_MULTIPLY_ASSIGN:
                case I2D_DIVIDE_ASSIGN:
                case I2D_MODULUS_ASSIGN:
                case I2D_RIGHT_SHIFT_ASSIGN:
                case I2D_LEFT_SHIFT_ASSIGN:
                case I2D_BIT_AND_ASSIGN:
                case I2D_BIT_OR_ASSIGN:
                case I2D_BIT_XOR_ASSIGN:
                case I2D_ASSIGN:
                    i2d_deit(node->left->range, i2d_range_list_deit);

                    if(i2d_range_list_copy(&node->left->range, node->range)) {
                        status = i2d_panic("failed to copy range list object");
                    } else if(i2d_context_insert_variable(script->context, node->left)) {
                        status = i2d_panic("failed to map variable");
                    }
                    break;
            }
        }
    }

    return status;
}

#if i2d_debug
int i2d_script_test(i2d_script * script, i2d_item * item) {
    int status = I2D_OK;
    i2d_str * description = NULL;

    if(i2d_script_compile(script, item->script, &description)) {
        status = i2d_panic("failed to translate script for item %ld", item->id);
    } else {
        i2d_deit(description, i2d_str_deit);
    }

    if(i2d_script_compile(script, item->onequip_script, &description)) {
        status = i2d_panic("failed to translate script for item %ld", item->id);
    } else {
        i2d_deit(description, i2d_str_deit);
    }

    if(i2d_script_compile(script, item->onunequip_script, &description)) {
        status = i2d_panic("failed to translate script for item %ld", item->id);
    } else {
        i2d_deit(description, i2d_str_deit);
    }

    return status;
}

int i2d_lexer_test(void) {
    int status = I2D_OK;
    i2d_lexer * lexer = NULL;
    i2d_str * script = NULL;
    i2d_token * token = NULL;
    i2d_token * tokens = NULL;

    int i;
    int j;
    enum i2d_token_type sequence[] = { I2D_LITERAL, I2D_CURLY_OPEN, I2D_CURLY_CLOSE, I2D_PARENTHESIS_OPEN, I2D_PARENTHESIS_CLOSE, I2D_COMMA, I2D_SEMICOLON, I2D_LITERAL, I2D_LITERAL, I2D_LITERAL, I2D_LITERAL, I2D_TEMPORARY_CHARACTER, I2D_PERMANENT_GLOBAL, I2D_TEMPORARY_GLOBAL, I2D_TEMPORARY_NPC, I2D_TEMPORARY_SCOPE, I2D_TEMPORARY_INSTANCE, I2D_PERMANENT_ACCOUNT_LOCAL, I2D_PERMANENT_ACCOUNT_GLOBAL, I2D_ADD, I2D_SUBTRACT, I2D_MULTIPLY, I2D_DIVIDE, I2D_MODULUS, I2D_ADD_ASSIGN, I2D_SUBTRACT_ASSIGN, I2D_MULTIPLY_ASSIGN, I2D_DIVIDE_ASSIGN, I2D_MODULUS_ASSIGN, I2D_GREATER, I2D_LESS, I2D_NOT, I2D_EQUAL, I2D_GREATER_EQUAL, I2D_LESS_EQUAL, I2D_NOT_EQUAL, I2D_RIGHT_SHIFT, I2D_LEFT_SHIFT, I2D_BIT_AND, I2D_BIT_OR, I2D_BIT_XOR, I2D_BIT_NOT, I2D_RIGHT_SHIFT_ASSIGN, I2D_LEFT_SHIFT_ASSIGN, I2D_BIT_AND_ASSIGN, I2D_BIT_OR_ASSIGN, I2D_BIT_XOR_ASSIGN, I2D_AND, I2D_OR, I2D_CONDITIONAL, I2D_COLON, I2D_UNIQUE_NAME, I2D_ASSIGN };

    assert(!i2d_lexer_init(&lexer));
    assert(!i2d_str_init(&script, "//\n\"QUOTE\"/*123*/{}(),; _var1 var2 1234 0x11 @ $ $@ . .@ ' # ## + - * / % += -= *= /= %= > < ! == >= <= != >> <<  & | ^ ~ >>= <<= &= |= ^= && || ? : :: =", 147));
    for(j = 0; j < 2; j++) {
        assert(!i2d_lexer_tokenize(lexer, script, &tokens));
        i = 0;
        token = tokens->next;
        while(token != tokens) {
            assert(token->type == sequence[i++]);
            token = token->next;
        }
        i2d_lexer_reset(lexer, &tokens);
    }
    i2d_str_deit(&script);
    i2d_lexer_deit(&lexer);

    assert(!i2d_lexer_init(&lexer));
    assert(!i2d_str_init(&script, "@var $var $@var .var .@var 'var #var ##var @var$ $var$ $@var$ .var$ .@var$ 'var$ #var$ ##var$", 93));
    assert(!i2d_lexer_tokenize(lexer, script, &tokens));
    token = tokens->next;
    while(token != tokens) {
        assert(token->type == I2D_LITERAL);
        token = token->next;
    }
    i2d_lexer_reset(lexer, &tokens);
    i2d_str_deit(&script);
    i2d_lexer_deit(&lexer);

    return status;
}
#endif
