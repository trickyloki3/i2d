#include "i2d_script.h"

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
    "//"
    "/**/",
    "\"\""
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

    if(I2D_LITERAL != token->type) {
        status = i2d_panic("invalid token type");
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
        if(I2D_LITERAL == iterator->type && !i2d_token_get_literal(iterator, &literal)) {
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
    i2d_deit(object->list, i2d_token_list_deit);
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

int i2d_lexer_tokenize(i2d_lexer * lexer, i2d_str * script) {
    int status = I2D_OK;
    size_t i;
    char symbol;
    i2d_token * token = NULL;
    i2d_token * state = NULL;

    if(lexer->list)
        i2d_lexer_reset(lexer, &lexer->list);

    if(i2d_lexer_token_init(lexer, &lexer->list, I2D_TOKEN)) {
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
                i2d_token_append(token, lexer->list);
                state = token;
            }
        }
    }

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
        putc('\t', stdout);

    i2d_token_print(node->tokens);

    if(node->left)
        i2d_node_print(node->left, level + 1);
    if(node->right)
        i2d_node_print(node->right, level + 1);
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
            object->type = type;
            object->tokens = tokens;
            object->parent = parent;
            object->next = object;
            object->prev = object;
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
        putc('\t', stdout);

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

int i2d_block_token_augment(i2d_block * block, i2d_lexer * lexer) {
    int status = I2D_OK;
    i2d_token * token = NULL;

    if(i2d_lexer_token_init(lexer, &token, I2D_TOKEN)) {
        status = i2d_panic("failed to create token object");
    } else {
        i2d_token_append(token, block->tokens);
        block->tokens = token;
    }

    return status;
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
    i2d_deit(object->block_list, i2d_block_list_deit);
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

int i2d_parser_analysis(i2d_parser * parser, i2d_lexer * lexer, i2d_json * json) {
    int status = I2D_OK;

    if(parser->block_list)
        i2d_parser_reset(parser, lexer, &parser->block_list);

    if(I2D_CURLY_OPEN != lexer->list->next->type) {
        status = i2d_panic("script must start with a {");
    } else if(I2D_CURLY_CLOSE != lexer->list->prev->type) {
        status = i2d_panic("script must end with a {");
    } else if(i2d_parser_analysis_recursive(parser, lexer, json, NULL, &parser->block_list, lexer->list->next)) {
        status = i2d_panic("failed to parse script");
    }

    return status;
}

int i2d_parser_analysis_recursive(i2d_parser * parser, i2d_lexer * lexer, i2d_json * json, i2d_block * parent, i2d_block ** result, i2d_token * tokens) {
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
            } else if(i2d_parser_analysis_recursive(parser, lexer, json, block, &block->child, tokens->next)) {
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
            if(i2d_parser_block_init(parser, &block, I2D_STATEMENT, anchor, parent)) {
                status = i2d_panic("failed to create block object");
            } else {
                tokens = tokens->next;
                i2d_token_append(anchor->prev, tokens);
                anchor = tokens;

                if(i2d_block_token_augment(block, lexer)) {
                    status = i2d_panic("failed to create token object");
                } else if(i2d_parser_statement_map(parser, lexer, block)) {
                    status = i2d_panic("failed to lookup block data object");
                } else if(block->statement && block->tokens->next->type == I2D_SEMICOLON) {
                    /* support statements without arguments */
                } else if(i2d_parser_statement_recursive(parser, lexer, block, &block->child, block->tokens->next)) {
                    status = i2d_panic("failed to parse statement");
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

int i2d_parser_statement_recursive(i2d_parser * parser, i2d_lexer * lexer, i2d_block * parent, i2d_block ** result, i2d_token * tokens) {
    int status = I2D_OK;
    i2d_block * root;
    i2d_block * block;
    i2d_token * anchor;
    int parenthesis = 0;

    root = NULL;
    block = NULL;
    anchor = tokens;
    while(tokens->type != I2D_TOKEN && !status) {
        if(I2D_PARENTHESIS_OPEN == tokens->type) {
            parenthesis++;
        } else if(I2D_PARENTHESIS_CLOSE == tokens->type) {
            parenthesis--;
        } else if(!parenthesis) {
            if(I2D_COMMA == tokens->type) {
                if(anchor == tokens) {
                    status = i2d_panic("empty expression");
                } else if(i2d_parser_block_init(parser, &block, I2D_EXPRESSION, anchor, parent)) {
                        status = i2d_panic("failed to create block object");
                } else {
                    i2d_token_append(anchor->prev, tokens);
                    anchor = tokens->next;

                    if(i2d_block_token_augment(block, lexer)) {
                        status = i2d_panic("failed to create token object");
                    } else if(i2d_parser_expression_recursive(parser, lexer, block->tokens->next, &block->nodes)) {
                        status = i2d_panic("failed to parse expression");
                    }
                }
            } else if(I2D_SEMICOLON == tokens->type) {
                if(anchor == tokens) {
                    status = i2d_panic("empty statement");
                } else {
                    if(i2d_parser_block_init(parser, &block, I2D_EXPRESSION, anchor, parent)) {
                        status = i2d_panic("failed to create block object");
                    } else {
                        i2d_token_append(anchor->prev, tokens);
                        anchor = tokens->next;

                        if(i2d_block_token_augment(block, lexer)) {
                            status = i2d_panic("failed to create token object");
                        } else if(i2d_parser_expression_recursive(parser, lexer, block->tokens->next, &block->nodes)) {
                            status = i2d_panic("failed to parse expression");
                        }
                    }
                }
            }
        }
        tokens = tokens->next;

        if(block) {
            if(!root) {
                root = block;
            } else {
                i2d_block_append(block, root);
            }
            block = NULL;
        }
    }

    if(!status) {
        if(parenthesis) {
            status = i2d_panic("statement is missing parenthesises");
        } else if(I2D_SEMICOLON != tokens->prev->type) {
            status = i2d_panic("statement is missing semicolon");
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
            if(i2d_lexer_token_init(lexer, &token, I2D_TOKEN)) {
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

int i2d_translator_init(i2d_translator ** result) {
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
    i2d_free(object);
    *result = NULL;
}

int i2d_translator_translate(i2d_translator * translator, i2d_block * list) {
    int status = I2D_OK;
    i2d_block * block;

    if(list) {
        block = list;
        do {
            switch(block->type) {
                case I2D_BLOCK:
                    status = i2d_translator_translate(translator, block->child);
                    break;
                case I2D_EXPRESSION:
                    status = i2d_translator_expression(translator, block);
                    break;
                case I2D_STATEMENT:
                    status = i2d_translator_statement(translator, block);
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

int i2d_translator_statement(i2d_translator * translator, i2d_block * block) {
    int status = I2D_OK;

    return status;
}

int i2d_translator_expression(i2d_translator * translator, i2d_block * block) {
    int status = I2D_OK;

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
            } else if(i2d_translator_init(&object->translator)) {
                status = i2d_panic("failed to create translator object");
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
    i2d_deit(object->translator, i2d_translator_deit);
    i2d_deit(object->parser, i2d_parser_deit);
    i2d_deit(object->lexer, i2d_lexer_deit);
    i2d_deit(object->json, i2d_json_deit);
    i2d_free(object);
    *result = NULL;
}

int i2d_script_compile(i2d_script * script, i2d_str * source, i2d_str ** target) {
    int status = I2D_OK;

    if(!strcmp("{}", source->string)) {
        status = i2d_str_init(target, "", 0);
    } else if(i2d_lexer_tokenize(script->lexer, source)) {
        status = i2d_panic("failed to lex -- %s", source->string);
    } else if(i2d_parser_analysis(script->parser, script->lexer, script->json)) {
        status = i2d_panic("failed to parse -- %s", source->string);
    } else if(i2d_translator_translate(script->translator, script->parser->block_list)) {
        status = i2d_panic("failed to translate -- %s", source->string);
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

    int i;
    int j;
    enum i2d_token_type sequence[] = { I2D_LITERAL, I2D_CURLY_OPEN, I2D_CURLY_CLOSE, I2D_PARENTHESIS_OPEN, I2D_PARENTHESIS_CLOSE, I2D_COMMA, I2D_SEMICOLON, I2D_LITERAL, I2D_LITERAL, I2D_LITERAL, I2D_LITERAL, I2D_TEMPORARY_CHARACTER, I2D_PERMANENT_GLOBAL, I2D_TEMPORARY_GLOBAL, I2D_TEMPORARY_NPC, I2D_TEMPORARY_SCOPE, I2D_TEMPORARY_INSTANCE, I2D_PERMANENT_ACCOUNT_LOCAL, I2D_PERMANENT_ACCOUNT_GLOBAL, I2D_ADD, I2D_SUBTRACT, I2D_MULTIPLY, I2D_DIVIDE, I2D_MODULUS, I2D_ADD_ASSIGN, I2D_SUBTRACT_ASSIGN, I2D_MULTIPLY_ASSIGN, I2D_DIVIDE_ASSIGN, I2D_MODULUS_ASSIGN, I2D_GREATER, I2D_LESS, I2D_NOT, I2D_EQUAL, I2D_GREATER_EQUAL, I2D_LESS_EQUAL, I2D_NOT_EQUAL, I2D_RIGHT_SHIFT, I2D_LEFT_SHIFT, I2D_BIT_AND, I2D_BIT_OR, I2D_BIT_XOR, I2D_BIT_NOT, I2D_RIGHT_SHIFT_ASSIGN, I2D_LEFT_SHIFT_ASSIGN, I2D_BIT_AND_ASSIGN, I2D_BIT_OR_ASSIGN, I2D_BIT_XOR_ASSIGN, I2D_AND, I2D_OR, I2D_CONDITIONAL, I2D_COLON, I2D_UNIQUE_NAME, I2D_ASSIGN };

    assert(!i2d_lexer_init(&lexer));
    assert(!i2d_str_init(&script, "//\n\"QUOTE\"/*123*/{}(),; _var1 var2 1234 0x11 @ $ $@ . .@ ' # ## + - * / % += -= *= /= %= > < ! == >= <= != >> <<  & | ^ ~ >>= <<= &= |= ^= && || ? : :: =", 147));
    for(j = 0; j < 2; j++) {
        assert(!i2d_lexer_tokenize(lexer, script));
        i = 0;
        token = lexer->list->next;
        while(token != lexer->list) {
            assert(token->type == sequence[i++]);
            token = token->next;
        }
    }
    i2d_str_deit(&script);
    i2d_lexer_deit(&lexer);

    assert(!i2d_lexer_init(&lexer));
    assert(!i2d_str_init(&script, "@var $var $@var .var .@var 'var #var ##var @var$ $var$ $@var$ .var$ .@var$ 'var$ #var$ ##var$", 93));
    assert(!i2d_lexer_tokenize(lexer, script));
    token = lexer->list->next;
    while(token != lexer->list) {
        assert(token->type == I2D_LITERAL);
        token = token->next;
    }
    i2d_str_deit(&script);
    i2d_lexer_deit(&lexer);

    return status;
}
#endif
