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
    }
    i2d_deit(object, i2d_token_deit);
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

int i2d_data_init(i2d_data ** result, const char * key, json_t * json, i2d_constant_db * constant_db) {
    int status = I2D_OK;
    i2d_data * object;
    json_t * min;
    json_t * max;
    json_t * description;
    json_t * argument_type;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            min = json_object_get(json, "min");
            max = json_object_get(json, "max");
            description = json_object_get(json, "description");
            argument_type = json_object_get(json, "argument_type");

            if(i2d_constant_get_by_macro_value(constant_db, key, &object->value)) {
                status = i2d_panic("failed to search for constant -- %s", key);
            } else if(i2d_string_create(&object->name, key, strlen(key))) {
                status = i2d_panic("failed to copy name string");
            } else if(description && i2d_format_create_json(&object->format, description)) {
                status = i2d_panic("failed to create format object");
            } else if(min && max && i2d_object_get_range(min, max, &object->range)) {
                status = i2d_panic("failed to create range");
            } else if(argument_type && i2d_object_get_string_stack(argument_type, &object->types)) {
                status = i2d_panic("failed to create string stack");
            }

            if(status)
                i2d_data_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_data_deit(i2d_data ** result) {
    i2d_data * object;

    object = *result;
    i2d_string_stack_destroy(&object->types);
    i2d_range_destroy(&object->range);
    i2d_format_destroy(&object->format);
    i2d_string_destroy(&object->name);
    i2d_free(object);
    *result = NULL;
}

int i2d_script_init(i2d_script ** result, i2d_option * option) {
    int status = I2D_OK;
    i2d_script * object;
    json_t * getiteminfo;
    json_t * strcharinfo;

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
                if(!getiteminfo || i2d_value_map_init(&object->getiteminfo, getiteminfo)) {
                    status = i2d_panic("failed to load getiteminfo");
                } else if(!strcharinfo || i2d_value_map_init(&object->strcharinfo, strcharinfo)) {
                    status = i2d_panic("failed to load strcharinfo");
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
    i2d_deit(object->strcharinfo, i2d_value_map_deit);
    i2d_deit(object->getiteminfo, i2d_value_map_deit);
    i2d_deit(object->constant_db, i2d_constant_db_deit);
    i2d_deit(object->parser, i2d_parser_deit);
    i2d_deit(object->lexer, i2d_lexer_deit);
    i2d_json_destroy(object->json);
    i2d_deit(object->db, i2d_db_deit);
    i2d_free(object);
    *result = NULL;
}

int i2d_script_compile(i2d_script * script, i2d_string * source, i2d_string * target) {
    int status = I2D_OK;
    i2d_token * tokens = NULL;
    i2d_block * blocks = NULL;

    if(!strcmp("{}", source->string)) {
        status = i2d_string_create(target, "", 0);
    } else if(i2d_lexer_tokenize(script->lexer, source, &tokens)) {
        status = i2d_panic("failed to tokenize -- %s", source->string);
    } else if(i2d_parser_analysis(script->parser, script->lexer, tokens, &blocks)) {
        status = i2d_panic("failed to parse -- %s", source->string);
    } else {

    }

    if(tokens)
        i2d_lexer_reset(script->lexer, &tokens);

    if(blocks)
        i2d_parser_reset(script->parser, script->lexer, &blocks);

    return status;
}
