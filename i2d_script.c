#include "i2d_script.h"

static int i2d_bonus_handler_time(i2d_script *, i2d_node *, i2d_str_stack *);
static int i2d_bonus_handler_regen(i2d_script *, i2d_node *, i2d_str_stack *);
static int i2d_bonus_handler_splash(i2d_script *, i2d_node *, i2d_str_stack *);
static int i2d_bonus_handler_elements(i2d_script *, i2d_node *, i2d_str_stack *);
static int i2d_bonus_handler_races(i2d_script *, i2d_node *, i2d_str_stack *);
static int i2d_bonus_handler_classes(i2d_script *, i2d_node *, i2d_str_stack *);
static int i2d_bonus_handler_integer(i2d_script *, i2d_node *, i2d_str_stack *);
static int i2d_bonus_handler_percent(i2d_script *, i2d_node *, i2d_str_stack *);
static int i2d_bonus_handler_percent_invert(i2d_script *, i2d_node *, i2d_str_stack *);
static int i2d_bonus_handler_percent__div100(i2d_script *, i2d_node *, i2d_str_stack *);
static int i2d_bonus_handler_ignore(i2d_script *, i2d_node *, i2d_str_stack *);
static int i2d_function_handler_generic(i2d_script *, i2d_node *);
static int i2d_function_handler_readparam(i2d_script *, i2d_node *);
static int i2d_function_handler_getskilllv(i2d_script *, i2d_node *);
static int i2d_function_handler_isequipped(i2d_script *, i2d_node *);
static int i2d_function_handler_countitem(i2d_script *, i2d_node *);
static int i2d_function_handler_gettime(i2d_script *, i2d_node *);
static int i2d_function_handler_strcharinfo(i2d_script *, i2d_node *);
static int i2d_function_handler_getequipid(i2d_script *, i2d_node *);
static int i2d_function_handler_getiteminfo(i2d_script *, i2d_node *);

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

int i2d_token_assign_literal(i2d_token * token, i2d_str * result) {
    int status = I2D_OK;

    i2d_buf_zero(token->buffer);

    if(i2d_buf_format(token->buffer, "%s", result->string))
        status = i2d_panic("failed to format buffer object");

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
        if(!i2d_token_get_literal(iterator, &literal)) {
            fprintf(stdout, "%s(%s) ", i2d_token_string[iterator->type], literal.string);
        } else {
            fprintf(stdout, "%s ", i2d_token_string[iterator->type]);
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
    i2d_deit(object->logic, i2d_logic_deit);
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

int i2d_node_copy(i2d_node * source, i2d_node * target) {
    int status = I2D_OK;

    i2d_deit(source->range, i2d_range_list_deit);
    i2d_deit(source->logic, i2d_logic_deit);

    if(i2d_range_list_copy(&source->range, target->range)) {
        status = i2d_panic("failed to copy range list object");
    } else if(target->logic) {
        if(i2d_logic_copy(&source->logic, target->logic))
            status = i2d_panic("failed to copy logic object");
    }

    return status;
}

void i2d_node_print(i2d_node * node, int level) {
    int i;

    for(i = 0; i < level; i++)
        fprintf(stdout, "    ");

    fprintf(stdout, "(%s) ", i2d_node_string[node->type]);

    i2d_token_print(node->tokens);

    if(node->range) {
        i2d_range_list_print(node->range, NULL);
    } else {
        fprintf(stdout, "\n");
    }

    if(node->logic)
        i2d_logic_print(node->logic, level + 1);

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

    if(I2D_VARIABLE != node->type && I2D_FUNCTION != node->type) {
        status = i2d_panic("failed on invalid node type -- %d", node->type);
    } else if(i2d_token_get_literal(node->tokens, result)) {
        status = i2d_panic("failed to get literal");
    }

    return status;
}

int i2d_node_get_predicate(i2d_node * node, i2d_str * result) {
    int status = I2D_FAIL;

    if(I2D_VARIABLE == node->type) {
        status = i2d_token_get_literal(node->tokens, result);
    } else if(I2D_FUNCTION == node->type) {
        status = i2d_token_get_literal(node->tokens, result);
    } else {
        if(node->left)
            status = i2d_node_get_predicate(node->left, result);
        if(node->right && status)
            status = i2d_node_get_predicate(node->right, result);
    }

    return status;
}

int i2d_node_get_all_predicate(i2d_node * node, i2d_buf * buffer) {
    int status = I2D_OK;
    i2d_str literal;

    if( I2D_VARIABLE == node->type ||
        I2D_FUNCTION == node->type ) {
        if(i2d_token_get_literal(node->tokens, &literal)) {
            status = i2d_panic("failed to get literal");
        } else {
            status = buffer->offset ?
                i2d_buf_format(buffer, ", %s", literal.string) :
                i2d_buf_format(buffer, "%s", literal.string);
        }
    } else {
        if(node->left)
            status = i2d_node_get_all_predicate(node->left, buffer);
        if(node->right)
            status = i2d_node_get_all_predicate(node->right, buffer);
    }

    return status;
}

static i2d_statement statements[] = {
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
            if(i2d_buf_init(&object->buffer, 4096)) {
                status = i2d_panic("failed to create buffer object");
            } else if(i2d_str_stack_init(&object->stack, 16)) {
                status = i2d_panic("failed to create string stack object");
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
    i2d_deit(object->stack, i2d_str_stack_deit);
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
                        if(i2d_rbt_insert(object->statement_map, &statements[i].name, &statements[i]))
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
    i2d_str name;
    i2d_zero(name);

    token = block->tokens->next;
    if(I2D_LITERAL == token->type) {
        if(i2d_token_get_literal(token, &name)) {
            status = i2d_panic("failed to get literal");
        } else {
            if(!i2d_rbt_search(parser->statement_map, &name, (void *) &block->statement)) {
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
        i2d_str_stack_clear(block->stack);
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
    i2d_block * state;
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

int i2d_const_init(void ** result, const char * name, json_t * json, i2d_rbt * rbt, void * context) {
    int status = I2D_OK;
    i2d_const * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramaters");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_str_copy(&object->name, name, strlen(name))) {
                status = i2d_panic("failed to create string object");
            } else {
                object->value = json_integer_value(json);
                if(i2d_rbt_insert(rbt, &object->name, object))
                    status = i2d_panic("failed to map const object");
            }
            if(status)
                i2d_const_deit((void **) &object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_const_deit(void ** result) {
    i2d_const * object;

    object = *result;
    i2d_free(object->name.string);
    i2d_free(object);
    *result = NULL;
}

int i2d_description_init(i2d_description ** result, i2d_str_const * description) {
    int status = I2D_OK;
    i2d_description * object;

    if(i2d_is_invalid(result) || !description) {
        status = i2d_panic("invalid paramaters");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_token_init(&object->tokens, I2D_TOKEN)) {
                status = i2d_panic("failed to create token object");
            } else if(i2d_description_tokenize(object, description->string, description->length)) {
                status = i2d_panic("failed to tokenize description");
            }

            if(status)
                i2d_description_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_description_deit(i2d_description ** result) {
    i2d_description * object;

    object = *result;
    i2d_deit(object->tokens, i2d_token_list_deit);
    i2d_free(object);
    *result = NULL;
}

int i2d_description_tokenize(i2d_description * description, const char * string, size_t length) {
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
            i2d_token_append(token, description->tokens);
            state = token;
            token = NULL;
        }
    }

    return status;
}

int i2d_description_format(i2d_description * description, i2d_str_stack * stack, i2d_buf * buffer) {
    int status = I2D_OK;
    i2d_str * list;
    size_t size;
    i2d_token * token;
    i2d_str literal;
    long index;

    if(i2d_str_stack_get_list(stack, &list, &size)) {
        status = i2d_panic("failed to get stack of string");
    } else if(!description->tokens) {
        status = i2d_panic("failed on empty bonus type description");
    } else {
        token = description->tokens->next;
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
                    } else if(i2d_buf_format(buffer, "%s", list[index].string)) {
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

int i2d_function_init(void ** result, const char * name, json_t * json, i2d_rbt * rbt, void * context) {
    int status = I2D_OK;
    i2d_function * object;
    json_int_t min;
    json_int_t max;
    i2d_str_const description;
    i2d_zero(description);

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_str_copy(&object->name, name, strlen(name))) {
                status = i2d_panic("failed to create string object");
            } else if(i2d_json_get_int(json, "min", &min) || i2d_json_get_int(json, "max", &max)) {
                status = i2d_panic("failed to get min and max value");
            } else if(i2d_range_list_init2(&object->range, min, max)) {
                status = i2d_panic("failed to create range list object");
            } else if(i2d_json_get_str(json, "description", &description)) {
                status = i2d_panic("failed to get description string");
            } else if(i2d_description_init(&object->description, &description)) {
                status = i2d_panic("failed to create description object");
            } else if(i2d_rbt_insert(rbt, &object->name, object)) {
                status = i2d_panic("failed to map function object");
            }

            if(status)
                i2d_function_deit((void **) &object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_function_deit(void ** result) {
    i2d_function * object;

    object = *result;
    i2d_deit(object->range, i2d_range_list_deit);
    i2d_deit(object->description, i2d_description_deit);
    i2d_free(object->name.string);
    i2d_free(object);
    *result = NULL;
}

int i2d_bonus_type_init(void ** result, const char * name, json_t * json, i2d_rbt * rbt, void * context) {
    int status = I2D_OK;
    i2d_bonus_type * object;
    json_t * argument_type;
    i2d_str_const description;
    i2d_zero(description);

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramaters");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_str_copy(&object->name, name, strlen(name))) {
                status = i2d_panic("failed to create string object");
            } else if(i2d_json_get_str(json, "description", &description)) {
                status = i2d_panic("failed to get description string");
            } else if(i2d_description_init(&object->description, &description)) {
                status = i2d_panic("failed to create description object");
            } else if(i2d_str_list_init(&object->argument_type, "argument_type", json)) {
                status = i2d_panic("failed to create string list object");
            } else if(i2d_translator_const_map(context, &object->name, &object->value)) {
                status = i2d_panic("failed to get bonus type value -- %s", object->name.string);
            } else if(i2d_rbt_insert(rbt, &object->value, object)) {
                status = i2d_panic("failed to map bonus type object");
            }

            if(status)
                i2d_bonus_type_deit((void **) &object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_bonus_type_deit(void ** result) {
    i2d_bonus_type * object;

    object = *result;
    i2d_deit(object->argument_type, i2d_str_list_deit);
    i2d_deit(object->description, i2d_description_deit);
    i2d_free(object->name.string);
    i2d_free(object);
    *result = NULL;
}

int i2d_readparam_init(void ** result, const char * key, json_t * json, i2d_rbt * rbt, void * context) {
    int status = I2D_OK;
    i2d_readparam * object;
    i2d_str_const search;
    i2d_str_const name;
    long min;
    long max;
    json_t * value;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_json_get_str(json, "name", &name)) {
                status = i2d_panic("failed to get string object");
            } else if(i2d_str_copy(&object->name, name.string, name.length)) {
                status = i2d_panic("failed to create string object");
            } else {
                value = json_object_get(json, "min");
                if(value) {
                    min = json_integer_value(value);
                    value = json_object_get(json, "max");
                    if(value) {
                        max = json_integer_value(value);
                        if(i2d_range_list_init2(&object->range, min, max))
                            status = i2d_panic("failed to create range list object");
                    }
                } else if(i2d_range_list_init2(&object->range, 0, 0)) {
                    status = i2d_panic("failed to create range list object");
                }

                if(!status) {
                    search.string = key;
                    search.length = strlen(key);
                    if(i2d_translator_const_map(context, (i2d_str *) &search, &object->key)) {
                        status = i2d_panic("failed to get readparam value -- %s", search.string);
                    } else if(i2d_rbt_insert(rbt, &object->key, object)) {
                        status = i2d_panic("failed to map readparam object");
                    }
                }
            }

            if(status)
                i2d_readparam_deit((void **) &object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_readparam_deit(void ** result) {
    i2d_readparam * object;

    object = *result;
    i2d_deit(object->range, i2d_range_list_deit);
    i2d_free(object->name.string);
    i2d_free(object);
    *result = NULL;
}

int i2d_str_map_init(void ** result, const char * key, json_t * json, i2d_rbt * rbt, void * context) {
    int status = I2D_OK;
    i2d_str_map * object;
    const char * string;
    size_t length;

    i2d_str_const consts;
    consts.string = key;
    consts.length = strlen(key);

    if(i2d_is_invalid(result) || !key || !json) {
        status = i2d_panic("invalid paramaters");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            string = json_string_value(json);
            if(!string) {
                status = i2d_panic("invalid string on %s key value", key);
            } else {
                length = json_string_length(json);
                if(!length) {
                    status = i2d_panic("empty string on %s key value", key);
                } else if(i2d_translator_const_map(context, (i2d_str *) &consts, &object->key)) {
                    status = i2d_panic("failed to get constant value -- %s", consts.string);
                } else if(i2d_str_copy(&object->value, string, length)) {
                    status = i2d_panic("failed to create string object");
                } else if(i2d_rbt_insert(rbt, &object->key, object)) {
                    status = i2d_panic("failed to map string object");
                }
            }

            if(status)
                i2d_str_map_deit((void **) &object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_str_map_deit(void ** result) {
    i2d_str_map * object;

    object = *result;
    i2d_free(object->value.string);
    i2d_free(object);
    *result = NULL;
}

int i2d_str_long_map_init(void ** result, const char * key, json_t * json, i2d_rbt * rbt, void * context) {
    int status = I2D_OK;
    i2d_str_long_map * object;
    const char * string;
    size_t length;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_strtol(&object->key, key, strlen(key), 10)) {
                status = i2d_panic("failed to convert literal to number");
            } else {
                string = json_string_value(json);
                if(!string) {
                    status = i2d_panic("failed on invalid string");
                } else {
                    length = json_string_length(json);
                    if(!length) {
                        status = i2d_panic("failed on empty string");
                    } else if(i2d_str_copy(&object->value, string, length)) {
                        status = i2d_panic("failed to create string object");
                    } else if(i2d_rbt_insert(rbt, &object->key, object)) {
                        status = i2d_panic("failed to map string long object");
                    }
                }
            }
            if(status)
                i2d_str_long_map_deit((void **) &object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_str_long_map_deit(void ** result) {
    i2d_str_long_map * object;

    object = *result;
    i2d_free(object->value.string);
    i2d_free(object);
    *result = NULL;
}

int i2d_translator_init(i2d_translator ** result, i2d_json * json) {
    int status = I2D_OK;
    i2d_translator * object;
    size_t i;
    size_t size;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_object_init(&object->consts, json->object, "consts", i2d_const_init, i2d_const_deit, i2d_rbt_cmp_str, object)) {
                status = i2d_panic("failed to create consts object");
            } else if(i2d_object_init(&object->functions, json->object, "functions", i2d_function_init, i2d_function_deit, i2d_rbt_cmp_str, object)) {
                status = i2d_panic("failed to create functions object");
            } else if(i2d_object_init(&object->bonus_types, json->blocks, "bonus", i2d_bonus_type_init, i2d_bonus_type_deit, i2d_rbt_cmp_long, object)) {
                status = i2d_panic("failed to create bonus types object");
            } else if(i2d_object_init(&object->elements, json->object, "elements", i2d_str_map_init, i2d_str_map_deit, i2d_rbt_cmp_long, object)) {
                status = i2d_panic("failed to create elements object");
            } else if(i2d_object_init(&object->races, json->object, "races", i2d_str_map_init, i2d_str_map_deit, i2d_rbt_cmp_long, object)) {
                status = i2d_panic("failed to create races object");
            } else if(i2d_object_init(&object->classes, json->object, "classes", i2d_str_map_init, i2d_str_map_deit, i2d_rbt_cmp_long, object)) {
                status = i2d_panic("failed to create classes object");
            } else if(i2d_object_init(&object->locations, json->object, "locations", i2d_str_map_init, i2d_str_map_deit, i2d_rbt_cmp_long, object)) {
                status = i2d_panic("failed to create locations object");
            } else if(i2d_object_init(&object->getiteminfo, json->object, "getiteminfo", i2d_str_long_map_init, i2d_str_long_map_deit, i2d_rbt_cmp_long, object)) {
                status = i2d_panic("failed to create getiteminfo object");
            } else if(i2d_object_init(&object->strcharinfo, json->object, "strcharinfo", i2d_str_long_map_init, i2d_str_long_map_deit, i2d_rbt_cmp_long, object)) {
                status = i2d_panic("failed to create strcharinfo object");
            } else if(i2d_object_init(&object->gettimes, json->object, "gettimes", i2d_readparam_init, i2d_readparam_deit, i2d_rbt_cmp_long, object)) {
                status = i2d_panic("failed to create gettimes object");
            } else if(i2d_object_init(&object->readparam, json->object, "readparam", i2d_readparam_init, i2d_readparam_deit, i2d_rbt_cmp_long, object)) {
                status = i2d_panic("failed to create readparams object");
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
    i2d_deit(object->readparam, i2d_object_deit);
    i2d_deit(object->gettimes, i2d_object_deit);
    i2d_deit(object->strcharinfo, i2d_object_deit);
    i2d_deit(object->getiteminfo, i2d_object_deit);
    i2d_deit(object->locations, i2d_object_deit);
    i2d_deit(object->classes, i2d_object_deit);
    i2d_deit(object->races, i2d_object_deit);
    i2d_deit(object->elements, i2d_object_deit);
    i2d_deit(object->bonus_types, i2d_object_deit);
    i2d_deit(object->functions, i2d_object_deit);
    i2d_deit(object->consts, i2d_object_deit);
    i2d_free(object);
    *result = NULL;
}

int i2d_translator_const_map(i2d_translator * translator, i2d_str * key, long * value) {
    int status = I2D_OK;
    i2d_const * constant;

    if(i2d_object_map(translator->consts, key, (void **) &constant)) {
        status = I2D_FAIL;
    } else {
        *value = constant->value;
    }

    return status;
}

int i2d_translator_bonus_map(i2d_translator * translator, long * key, i2d_bonus_type ** result) {
    return i2d_object_map(translator->bonus_types, key, (void **) result);
}

int i2d_translator_function_map(i2d_translator * translator, i2d_str * key, i2d_function ** result) {
    return i2d_object_map(translator->functions, key, (void **) result);
}

int i2d_translator_elements_map(i2d_translator * translator, long * key, i2d_str * result) {
    int status = I2D_OK;
    i2d_str_map * str_map;

    if(i2d_object_map(translator->elements, key, (void **) &str_map)) {
        status = I2D_FAIL;
    } else {
        *result = str_map->value;
    }

    return status;
}

int i2d_translator_races_map(i2d_translator * translator, long * key, i2d_str * result) {
    int status = I2D_OK;
    i2d_str_map * str_map;

    if(i2d_object_map(translator->races, key, (void **) &str_map)) {
        status = I2D_FAIL;
    } else {
        *result = str_map->value;
    }

    return status;
}

int i2d_translator_classes_map(i2d_translator * translator, long * key, i2d_str * result) {
    int status = I2D_OK;
    i2d_str_map * str_map;

    if(i2d_object_map(translator->classes, key, (void **) &str_map)) {
        status = I2D_FAIL;
    } else {
        *result = str_map->value;
    }

    return status;
}

int i2d_translator_locations_map(i2d_translator * translator, long * key, i2d_str * result) {
    int status = I2D_OK;
    i2d_str_map * str_map;

    if(i2d_object_map(translator->locations, key, (void **) &str_map)) {
        status = I2D_FAIL;
    } else {
        *result = str_map->value;
    }

    return status;
}

int i2d_translator_getiteminfo_map(i2d_translator * translator, long * key, i2d_str * result) {
    int status = I2D_OK;
    i2d_str_long_map * str_long_map;

    if(i2d_object_map(translator->getiteminfo, key, (void **) &str_long_map)) {
        status = I2D_FAIL;
    } else {
        *result = str_long_map->value;
    }

    return status;
}

int i2d_translator_strcharinfo_map(i2d_translator * translator, long * key, i2d_str * result) {
    int status = I2D_OK;
    i2d_str_long_map * str_long_map;

    if(i2d_object_map(translator->strcharinfo, key, (void **) &str_long_map)) {
        status = I2D_FAIL;
    } else {
        *result = str_long_map->value;
    }

    return status;
}

int i2d_translator_gettimes_map(i2d_translator * translator, long * key, i2d_readparam ** result) {
    return i2d_object_map(translator->gettimes, key, (void **) result);
}

int i2d_translator_readparam_map(i2d_translator * translator, long * key, i2d_readparam ** result) {
    return i2d_object_map(translator->readparam, key, (void **) result);
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
                if( i2d_buf_init(&object->predicates, 4096) ||
                    i2d_buf_init(&object->expression, 4096) ) {
                    status = i2d_panic("failed to create buffer objects");
                } else if(i2d_str_stack_init(&object->stack, 16)) {
                    status = i2d_panic("failed to create string stack object");
                } else {
                    object->next = object;
                    object->prev = object;
                }
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
    i2d_deit(object->stack, i2d_str_stack_deit);
    i2d_deit(object->expression, i2d_buf_deit);
    i2d_deit(object->predicates, i2d_buf_deit);
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
    i2d_str_stack_clear(context->stack);
    i2d_context_reset_buffer(context);
}

void i2d_context_reset_buffer(i2d_context * context) {
    i2d_buf_zero(context->expression);
    i2d_buf_zero(context->predicates);
    i2d_rbt_clear(context->variables);
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

static struct i2d_bonus_handler {
    i2d_str name;
    int (*handler)(i2d_script *, i2d_node *, i2d_str_stack *);
} bonus_list[] = {
    { {"time", 4}, i2d_bonus_handler_time },
    { {"regen", 5}, i2d_bonus_handler_regen },
    { {"splash", 6}, i2d_bonus_handler_splash },
    { {"elements", 8}, i2d_bonus_handler_elements },
    { {"races", 4}, i2d_bonus_handler_races },
    { {"classes", 7}, i2d_bonus_handler_classes },
    { {"integer", 7}, i2d_bonus_handler_integer },
    { {"percent", 7}, i2d_bonus_handler_percent },
    { {"percent_invert", 14}, i2d_bonus_handler_percent_invert },
    { {"percent_div100", 14}, i2d_bonus_handler_percent__div100 },
    { {"ignore", 6}, i2d_bonus_handler_ignore }
};

typedef struct i2d_bonus_handler i2d_bonus_handler;

static i2d_bonus_handler_expression(i2d_script * script, i2d_node * node, i2d_str_stack * stack) {
    int status = I2D_OK;
    i2d_str predicates;
    i2d_str expression;

    if(i2d_node_get_all_predicate(node, script->context->predicates)) {
        status = i2d_panic("failed to get all predicate");
    } else {
        i2d_buf_get_str(script->context->predicates, &predicates);
        if(predicates.length && i2d_buf_format(script->context->expression, " (%s)", predicates.string)) {
            status = i2d_panic("failed to write predicates");
        } else {
            i2d_buf_get_str(script->context->expression, &expression);
            if(i2d_str_stack_push(stack, &expression))
                status = i2d_panic("failed to push string on stack");
        }
    }

    return status;
}

static int i2d_bonus_handler_time(i2d_script * script, i2d_node * node, i2d_str_stack * stack) {
    int status = I2D_OK;
    long min;
    long max;

    char * suffix = NULL;
    long unit = 0;

    i2d_range_list_get_range(node->range, &min, &max);

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
        i2d_buf_format(script->context->expression, "%ld %s%s", min, suffix, min > 1 ? "s" : "") :
        i2d_buf_format(script->context->expression, "%ld ~ %ld %s%s", min, max, suffix, max > 1 ? "s" : "") ) {
        status = i2d_panic("failed to write time range");
    } else if(i2d_bonus_handler_expression(script, node, stack)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_bonus_handler_regen(i2d_script * script, i2d_node * node, i2d_str_stack * stack) {
    int status = I2D_OK;
    long constant;

    if(i2d_node_get_constant(node, &constant)) {
        status = i2d_panic("failed to get constant");
    } else {
        switch(constant) {
            case 1: status = i2d_buf_format(script->context->expression, "HP"); break;
            case 2: status = i2d_buf_format(script->context->expression, "SP"); break;
            default: status = i2d_panic("unsupported regen value -- %ld", constant); break;
        }
        if(status) {
            status = i2d_panic("failed to write regen");
        } else if(i2d_bonus_handler_expression(script, node, stack)) {
            status = i2d_panic("failed to write expression");
        }
    }

    return status;
}

static int i2d_bonus_handler_splash(i2d_script * script, i2d_node * node, i2d_str_stack * stack) {
    int status = I2D_OK;
    long min;
    long max;

    i2d_range_list_get_range(node->range, &min, &max);
    min = min * 2 + 1;
    max = max * 2 + 1;

    if( min == max ?
        i2d_buf_format(script->context->expression, "%ld x %ld", min, min) :
        i2d_buf_format(script->context->expression, "%ld x %ld ~ %ld x %ld", min, min, max, max) ) {
        status = i2d_panic("failed to write splash range");
    } else if(i2d_bonus_handler_expression(script, node, stack)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_bonus_handler_elements(i2d_script * script, i2d_node * node, i2d_str_stack * stack) {
    int status = I2D_OK;
    long constant;
    i2d_str element;

    if(i2d_node_get_constant(node, &constant)) {
        status = i2d_panic("failed to get constant");
    } else if(i2d_translator_elements_map(script->translator, &constant, &element)) {
        status = i2d_panic("failed to get element -- %ld", constant);
    } else if(i2d_str_stack_push(stack, &element)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_bonus_handler_races(i2d_script * script, i2d_node * node, i2d_str_stack * stack) {
    int status = I2D_OK;
    long constant;
    i2d_str race;

    if(i2d_node_get_constant(node, &constant)) {
        status = i2d_panic("failed to get constant");
    } else if(i2d_translator_races_map(script->translator, &constant, &race)) {
        status = i2d_panic("failed to get race -- %ld", constant);
    } else if(i2d_str_stack_push(stack, &race)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_bonus_handler_classes(i2d_script * script, i2d_node * node, i2d_str_stack * stack) {
    int status = I2D_OK;
    long constant;
    i2d_str class;

    if(i2d_node_get_constant(node, &constant)) {
        status = i2d_panic("failed to get constant");
    } else if(i2d_translator_classes_map(script->translator, &constant, &class)) {
        status = i2d_panic("failed to get class -- %ld", constant);
    } else if(i2d_str_stack_push(stack, &class)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_bonus_handler_integer(i2d_script * script, i2d_node * node, i2d_str_stack * stack) {
    int status = I2D_OK;
    long min;
    long max;

    i2d_range_list_get_range(node->range, &min, &max);
    if( min == max ?
        i2d_buf_format(script->context->expression, "%+ld", min) :
        i2d_buf_format(script->context->expression, "%+ld ~ %+ld", min, max) ) {
        status = i2d_panic("failed to write integer range");
    } else if(i2d_bonus_handler_expression(script, node, stack)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_bonus_handler_percent(i2d_script * script, i2d_node * node, i2d_str_stack * stack) {
    int status = I2D_OK;
    long min;
    long max;

    i2d_range_list_get_range(node->range, &min, &max);
    if( min == max ?
        i2d_buf_format(script->context->expression, "%+ld%%", min) :
        i2d_buf_format(script->context->expression, "%+ld%% ~ %+ld%%", min, max) ) {
        status = i2d_panic("failed to write percent range");
    } else if(i2d_bonus_handler_expression(script, node, stack)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_bonus_handler_percent_invert(i2d_script * script, i2d_node * node, i2d_str_stack * stack) {
    int status = I2D_OK;
    long min;
    long max;

    i2d_range_list_get_range(node->range, &min, &max);

    min *= -1;
    max *= -1;

    if( min == max ?
        i2d_buf_format(script->context->expression, "%+ld%%", min) :
        i2d_buf_format(script->context->expression, "%+ld%% ~ %+ld%%", max, min) ) {
        status = i2d_panic("failed to write percent range");
    } else if(i2d_bonus_handler_expression(script, node, stack)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_bonus_handler_percent__div100(i2d_script * script, i2d_node * node, i2d_str_stack * stack) {
    int status = I2D_OK;
    long min;
    long max;

    i2d_range_list_get_range(node->range, &min, &max);

    min /= 100;
    max /= 100;

    if( min == max ?
        i2d_buf_format(script->context->expression, "%+ld%%", min) :
        i2d_buf_format(script->context->expression, "%+ld%% ~ %+ld%%", max, min) ) {
        status = i2d_panic("failed to write percent range");
    } else if(i2d_bonus_handler_expression(script, node, stack)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_bonus_handler_ignore(i2d_script * script, i2d_node * node, i2d_str_stack * stack) {
    int status = I2D_OK;
    i2d_str ignore;

    ignore.string = "ignore";
    ignore.length = strlen(ignore.string);

    if(i2d_str_stack_push(stack, &ignore))
        status = i2d_panic("failed to push string on stack");

    return status;
}

static struct i2d_function_handler {
    i2d_str name;
    int (*handler)(i2d_script *, i2d_node *);
} function_list[] = {
    { {"getrefine", 9}, i2d_function_handler_generic },
    { {"readparam", 9}, i2d_function_handler_readparam },
    { {"getskilllv", 10}, i2d_function_handler_getskilllv },
    { {"isequipped", 10}, i2d_function_handler_isequipped },
    { {"getpartnerid", 12}, i2d_function_handler_generic },
    { {"checkmadogear", 12}, i2d_function_handler_generic },
    { {"eaclass", 7}, i2d_function_handler_generic },
    { {"countitem", 9}, i2d_function_handler_countitem },
    { {"gettime", 7}, i2d_function_handler_gettime },
    { {"strcharinfo", 11}, i2d_function_handler_strcharinfo },
    { {"getequipid", 10}, i2d_function_handler_getequipid },
    { {"getiteminfo", 11}, i2d_function_handler_getiteminfo }
};

typedef struct i2d_function_handler i2d_function_handler;

static int i2d_function_handler_generic(i2d_script * script, i2d_node * node) {
    int status = I2D_OK;
    i2d_str literal;
    i2d_function * function;

    if(i2d_node_get_string(node, &literal)) {
        status = i2d_panic("failed to get string");
    } else if(i2d_translator_function_map(script->translator, &literal, &function)) {
        status = i2d_panic("failed to get function -- %s", literal.string);
    } else {
        i2d_buf_zero(node->tokens->buffer);
        if(i2d_description_format(function->description, script->context->stack, node->tokens->buffer)) {
            status = i2d_panic("failed to write description");
        } else if(i2d_range_list_copy(&node->range, function->range)) {
            status = i2d_panic("failed to copy range list object");
        }
    }

    return status;
}

static int i2d_function_handler_readparam(i2d_script * script, i2d_node * node) {
    int status = I2D_OK;
    i2d_node * argument;
    long constant;
    i2d_readparam * readparam;

    if(i2d_node_get_arguments(node->left, &argument, 1, 0)) {
        status = i2d_panic("failed to get arguments");
    } else if(i2d_node_get_constant(argument->left, &constant)) {
        status = i2d_panic("failed to get constant");
    } else if(i2d_translator_readparam_map(script->translator, &constant, &readparam)) {
        status = i2d_panic("failed to get readparam -- %ld", constant);
    } else if(i2d_token_assign_literal(node->tokens, &readparam->name)) {
        status = i2d_panic("failed to assign literal to token");
    } else if(i2d_range_list_copy(&node->range, readparam->range)) {
        status = i2d_panic("failed to copy range list object");
    }

    return status;
}

static int i2d_function_handler_getskilllv(i2d_script * script, i2d_node * node) {
    int status = I2D_OK;
    i2d_node * argument;
    long constant;
    i2d_str literal;
    i2d_skill * skill;

    if(i2d_node_get_arguments(node->left, &argument, 1, 0)) {
        status = i2d_panic("failed to get arguments");
    } else if(i2d_node_get_constant(argument->left, &constant)) {
        status = i2d_panic("failed to get constant");
    } else if(i2d_skill_db_search_by_id(script->db->skill_db, constant, &skill)) {
        if(i2d_node_get_string(argument->left, &literal)) {
            status = i2d_panic("failed to get string");
        } else if(i2d_skill_db_search_by_macro(script->db->skill_db, &literal, &skill)) {
            status = i2d_panic("failed to get skill by id and macro -- %s (%ld)", literal.string, constant);
        }
    }

    if(!status) {
        if(i2d_token_assign_literal(node->tokens, &skill->name)) {
            status = i2d_panic("failed to assign literal to token");
        } else if(i2d_range_list_init2(&node->range, 0, skill->maxlv)) {
            status = i2d_panic("failed to create range list object");
        }
    }

    return status;
}

static int i2d_function_handler_isequipped(i2d_script * script, i2d_node * node) {
    int status = I2D_OK;
    i2d_str literal;
    i2d_function * function;
    i2d_node * arguments[I2D_CAP];
    size_t i;
    size_t size;
    long item_id;
    i2d_item * item;

    memset(arguments, 0, sizeof(arguments));
    size = i2d_size(arguments);

    if(i2d_node_get_string(node, &literal)) {
        status = i2d_panic("failed to get string");
    } else if(i2d_translator_function_map(script->translator, &literal, &function)) {
        status = i2d_panic("failed to get function -- %s", literal.string);
    } else if(i2d_node_get_arguments(node->left->left, arguments, 1, size - 1)) {
        status = i2d_panic("failed to get arguments");
    } else {
        i2d_buf_zero(node->tokens->buffer);
        for(i = 0; i < size && arguments[i] && !status; i++) {
            if(i2d_node_get_constant(arguments[i], &item_id)) {
                status = i2d_panic("failed to get item id");
            } else if(i2d_item_db_search_by_id(script->db->item_db, item_id, &item)) {
                status = i2d_panic("failed to get item by id -- %ld", item_id);
            } else {
                if( i ?
                    i2d_buf_format(node->tokens->buffer, ", %s", item->name.string) :
                    i2d_buf_format(node->tokens->buffer, "%s", item->name.string) )
                    status = i2d_panic("failed to write buffer object");
            }
        }

        if(!status) {
            if(i2d_node_get_string(node, &literal)) {
                status = i2d_panic("failed to get string");
            } else if(i2d_str_stack_push(script->context->stack, &literal)) {
                status = i2d_panic("failed to push string on stack");
            } else {
                i2d_buf_zero(node->tokens->buffer);
                if(i2d_description_format(function->description, script->context->stack, node->tokens->buffer)) {
                    status = i2d_panic("failed to format bonus type");
                } else if(i2d_range_list_copy(&node->range, function->range)) {
                    status = i2d_panic("failed to copy range list object");
                }
            }
        }
    }

    return status;
}

static int i2d_function_handler_countitem(i2d_script * script, i2d_node * node) {
    int status = I2D_OK;
    i2d_str literal;
    i2d_function * function;
    long item_id;
    i2d_item * item;

    if(i2d_node_get_string(node, &literal)) {
        status = i2d_panic("failed to get string");
    } else if(i2d_translator_function_map(script->translator, &literal, &function)) {
        status = i2d_panic("failed to get function -- %s", literal.string);
    } else if(i2d_node_get_constant(node->left, &item_id)) {
        status = i2d_panic("failed to get item id");
    } else if(i2d_item_db_search_by_id(script->db->item_db, item_id, &item)) {
        status = i2d_panic("failed to get item by id -- %ld", item_id);
    } else if(i2d_str_stack_push(script->context->stack, &item->name)) {
        status = i2d_panic("failed to push string on stack");
    } else {
        i2d_buf_zero(node->tokens->buffer);
        if(i2d_description_format(function->description, script->context->stack, node->tokens->buffer)) {
            status = i2d_panic("failed to format bonus type");
        } else if(i2d_range_list_copy(&node->range, function->range)) {
            status = i2d_panic("failed to copy range list object");
        }
    }

    return status;
}

static int i2d_function_handler_gettime(i2d_script * script, i2d_node * node) {
    int status = I2D_OK;
    long constant;
    i2d_readparam * param;

    if(i2d_node_get_constant(node->left, &constant)) {
        status = i2d_panic("failed to get constant");
    } else if(i2d_translator_gettimes_map(script->translator, &constant, &param)) {
        status = i2d_panic("failed to get gettime -- %ld", constant);
    } else if(i2d_token_assign_literal(node->tokens, &param->name)) {
        status = i2d_panic("failed to assign literal to token");
    } else if(i2d_range_list_copy(&node->range, param->range)) {
        status = i2d_panic("failed to copy range list object");
    }

    return status;
}

static int i2d_function_handler_strcharinfo(i2d_script * script, i2d_node * node) {
    int status = I2D_OK;
    long constant;
    i2d_str value;

    if(i2d_node_get_constant(node->left, &constant)) {
        status = i2d_panic("failed to get constant");
    } else if(i2d_translator_strcharinfo_map(script->translator, &constant, &value)) {
        status = i2d_panic("failed to get strcharinfo -- %ld", constant);
    } else if(i2d_token_assign_literal(node->tokens, &value)) {
        status = i2d_panic("failed to assign literal to token");
    } else if(i2d_range_list_copy(&node->range, node->left->range)) {
        status = i2d_panic("failed to create range list object");
    }

    return status;
}

static int i2d_function_handler_getequipid(i2d_script * script, i2d_node * node) {
    int status = I2D_OK;
    long constant;
    i2d_str value;

    if(i2d_node_get_constant(node->left, &constant)) {
        status = i2d_panic("failed to get constant");
    } else if(i2d_translator_locations_map(script->translator, &constant, &value)) {
        status = i2d_panic("failed to get getequipid -- %ld", constant);
    } else if(i2d_token_assign_literal(node->tokens, &value)) {
        status = i2d_panic("failed to assign literal to token");
    } else if(i2d_range_list_copy(&node->range, node->left->range)) {
        status = i2d_panic("failed to create range list object");
    }

    return status;
}

static int i2d_function_handler_getiteminfo(i2d_script * script, i2d_node * node) {
    int status = I2D_OK;
    i2d_str literal;
    i2d_function * function;
    i2d_node * arguments[2];
    long item_id;
    i2d_item * item;
    long info_type;
    i2d_str item_info;

    memset(arguments, 0, sizeof(arguments));

    if(i2d_node_get_string(node, &literal)) {
        status = i2d_panic("failed to get string");
    } else if(i2d_translator_function_map(script->translator, &literal, &function)) {
        status = i2d_panic("failed to get function -- %s", literal.string);
    } else if(i2d_node_get_arguments(node->left->left, arguments, i2d_size(arguments), 0)) {
        status = i2d_panic("failed to get arguments");
    } else {
        if(I2D_FUNCTION == arguments[0]->type) {
            if(i2d_node_get_string(arguments[0], &literal)) {
                status = i2d_panic("failed to get string");
            } else if(i2d_str_stack_push(script->context->stack, &literal)) {
                status = i2d_panic("failed to push string on stack");
            }
        } else {
            if(i2d_node_get_constant(arguments[0], &item_id)) {
                status = i2d_panic("failed to get item id");
            } else if(i2d_item_db_search_by_id(script->db->item_db, item_id, &item)) {
                status = i2d_panic("failed to get item by id -- %ld", item_id);
            } else if(i2d_str_stack_push(script->context->stack, &item->name)) {
                status = i2d_panic("failed to push string on stack");
            }
        }

        if(!status) {
            if(i2d_node_get_constant(arguments[0], &info_type)) {
                status = i2d_panic("failed to get info type");
            } else if(i2d_translator_getiteminfo_map(script->translator, &info_type, &item_info)) {
                status = i2d_panic("failed to get getiteminfo -- %ld", info_type);
            } else if(i2d_str_stack_push(script->context->stack, &item_info)) {
                status = i2d_panic("failed to push string on stack");
            } else {
                i2d_buf_zero(node->tokens->buffer);
                if(i2d_description_format(function->description, script->context->stack, node->tokens->buffer)) {
                    status = i2d_panic("failed to format bonus type");
                } else if(i2d_range_list_copy(&node->range, function->range)) {
                    status = i2d_panic("failed to create range list object");
                }
            }
        }
    }

    return status;
}

int i2d_script_init(i2d_script ** result, i2d_option * option) {
    int status = I2D_OK;
    i2d_script * object;
    size_t i;
    size_t size;

    if(i2d_is_invalid(result) || !option) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_db_init(&object->db, i2d_db_pre_renewal, option->source_path)) {
                status = i2d_panic("failed to create database object");
            } else if(i2d_json_init(&object->json, option->json_path)) {
                status = i2d_panic("failed to create json object");
            } else if(i2d_lexer_init(&object->lexer)) {
                status = i2d_panic("failed to create lexer object");
            } else if(i2d_parser_init(&object->parser)) {
                status = i2d_panic("failed to create parser object");
            } else if(i2d_translator_init(&object->translator, object->json)) {
                status = i2d_panic("failed to create translator object");
            } else if(i2d_context_init(&object->context)) {
                status = i2d_panic("failed to create context object");
            } else {
                if(i2d_rbt_init(&object->functions_handlers, i2d_rbt_cmp_str)) {
                    status = i2d_panic("failed to create red black tree object");
                } else {
                    size = i2d_size(function_list);
                    for(i = 0; i < size && !status; i++)
                        if(i2d_rbt_insert(object->functions_handlers, &function_list[i].name, &function_list[i]))
                            status = i2d_panic("failed to map function handler object");
                }

                if(i2d_rbt_init(&object->bonus_handlers, i2d_rbt_cmp_str)) {
                    status = i2d_panic("failed to create red black tree object");
                } else {
                    size = i2d_size(bonus_list);
                    for(i = 0; i < size && !status; i++)
                        if(i2d_rbt_insert(object->bonus_handlers, &bonus_list[i].name, &bonus_list[i]))
                            status = i2d_panic("failed to map bonus handler object");
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
    i2d_deit(object->bonus_handlers, i2d_rbt_deit);
    i2d_deit(object->functions_handlers, i2d_rbt_deit);
    i2d_deit(object->context, i2d_context_list_deit);
    i2d_deit(object->translator, i2d_translator_deit);
    i2d_deit(object->parser, i2d_parser_deit);
    i2d_deit(object->lexer, i2d_lexer_deit);
    i2d_deit(object->json, i2d_json_deit);
    i2d_deit(object->db, i2d_db_deit);
    i2d_free(object);
    *result = NULL;
}

int i2d_script_compile(i2d_script * script, i2d_str * source, i2d_str ** target) {
    int status = I2D_OK;
    i2d_token * tokens = NULL;
    i2d_block * blocks = NULL;

    i2d_context_reset(script->context);

    if(!strcmp("{}", source->string)) {
        status = i2d_str_init(target, "", 0);
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
                    status = i2d_script_expression(script, block->nodes, 1);
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
            default:
                break;
        }
    }

    return status;
}

int i2d_script_bonus_handler(i2d_script * script, i2d_str * argument_type, i2d_node * node, i2d_str_stack * stack) {
    int status = I2D_OK;
    i2d_bonus_handler * handler;

    if(i2d_rbt_search(script->bonus_handlers, argument_type, (void **) &handler)) {
        status = i2d_panic("failed to search to bonus handler -- %s", argument_type->string);
    } else {
        i2d_str_stack_clear(script->context->stack);
        i2d_buf_zero(script->context->expression);
        i2d_buf_zero(script->context->predicates);
        status = handler->handler(script, node, stack);
    }

    return status;
}

int i2d_script_bonus(i2d_script * script, i2d_block * block) {
    int status = I2D_OK;
    i2d_node * arguments[2];
    long bonus_id;
    i2d_bonus_type * bonus_type;

    if(i2d_script_expression(script, block->nodes, 0)) {
        status = i2d_panic("failed to translate expression");
    } else if(i2d_node_get_arguments(block->nodes->left, arguments, 2, 0)) {
        status = i2d_panic("failed to get arguments");
    } else if(i2d_node_get_constant(arguments[0], &bonus_id)) {
        status = i2d_panic("failed to get bonus type value");
    } else if(i2d_translator_bonus_map(script->translator, &bonus_id, &bonus_type)) {
        status = i2d_panic("failed to map bonus type value");
    } else if(i2d_script_bonus_handler(script, bonus_type->argument_type->list[0], arguments[1], block->stack)) {
        status = i2d_panic("failed to translate bonus arguments");
    } else if(i2d_description_format(bonus_type->description, block->stack, block->buffer)) {
        status = i2d_panic("failed to format bonus type");
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
                if(node->left) {
                    status = i2d_node_copy(node, node->left);
                } else {
                    status = i2d_range_list_init2(&node->range, 0, 0);
                }
                break;
            case I2D_VARIABLE:
                status = i2d_script_expression_variable(script, node);
                break;
            case I2D_FUNCTION:
                status = i2d_script_expression_function(script, node);
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
    i2d_readparam * readparam;
    int base;
    long number;
    i2d_str literal;
    i2d_zero(literal);

    if(i2d_token_get_literal(node->tokens, &literal)) {
        status = i2d_panic("failed to get literal");
    } else {
        if(!i2d_context_search_variable(script->context, node, &variable)) {
            if(i2d_node_copy(node, variable))
                status = i2d_panic("failed to copy node object");
        } else {
            if(isdigit(literal.string[0])) {
                if(!strncmp(literal.string, "0x", 2) && literal.length > 2) {
                    base = 16;
                } else if(literal.string[0] == '0' && literal.length > 1) {
                    base = 8;
                } else {
                    base = 10;
                }

                if(i2d_strtol(&number, literal.string, literal.length, base)) {
                    status = i2d_panic("failed to parse hexadecimal number -- %s", literal.string);
                } else {
                    node->type = I2D_NUMBER;
                }
            } else if(i2d_translator_const_map(script->translator, &literal, &number)) {
                number = 0;
            }

            if(i2d_range_list_init2(&node->range, number, number))
                status = i2d_panic("failed to create range list object");
        }
    }

    return status;
}

int i2d_script_expression_function(i2d_script * script, i2d_node * node) {
    int status = I2D_OK;
    i2d_str literal;
    i2d_function_handler * function;

    i2d_context_reset_buffer(script->context);
    if(i2d_token_get_literal(node->tokens, &literal)) {
        status = i2d_panic("failed to get literal");
    } else if(!i2d_rbt_search(script->functions_handlers, &literal, (void **) &function)) {
        status = function->handler(script, node);
    } else {
        status = i2d_panic("unsupported function -- %s", literal.string);
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
                    if(i2d_range_list_not(&node->range, node->right->range)) {
                        status = i2d_panic("failed to compute range object");
                    } else if(node->right->logic) {
                        if(i2d_logic_not(&node->logic, node->right->logic))
                            status = i2d_panic("failed to create logic object");
                    }

                } else {
                    status = i2d_range_list_init2(&node->range, 0, 1);
                }
                break;
            case I2D_BIT_NOT:
                status = i2d_range_list_bitnot(&node->range, node->right->range);
                break;
            case I2D_ADD_UNARY:
                status = i2d_node_copy(node, node->right);
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
    int is_assign = 0;
    i2d_str predicate;
    i2d_zero(predicate);

    if(!node->left || !node->left->range) {
        status = i2d_panic("binary operator missing left operand");
    } else if(!node->right || !node->right->range) {
        status = i2d_panic("binary operator missing right operand");
    } else {
        switch(node->tokens->type) {
            case I2D_ADD_ASSIGN:
                is_assign = 1;
            case I2D_ADD:
                status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '+');
                break;
            case I2D_SUBTRACT_ASSIGN:
                is_assign = 1;
            case I2D_SUBTRACT:
                status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '-');
                break;
            case I2D_MULTIPLY_ASSIGN:
                is_assign = 1;
            case I2D_MULTIPLY:
                status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '*');
                break;
            case I2D_DIVIDE_ASSIGN:
                is_assign = 1;
            case I2D_DIVIDE:
                status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '/');
                break;
            case I2D_MODULUS_ASSIGN:
                is_assign = 1;
            case I2D_MODULUS:
                status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '%');
                break;
            case I2D_COMMA:
                status = i2d_node_copy(node, node->right);
                break;
            case I2D_RIGHT_SHIFT_ASSIGN:
                is_assign = 1;
            case I2D_RIGHT_SHIFT:
                status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '>' + '>' + 'b');
                break;
            case I2D_LEFT_SHIFT_ASSIGN:
                is_assign = 1;
            case I2D_LEFT_SHIFT:
                status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '<' + '<' + 'b');
                break;
            case I2D_BIT_AND_ASSIGN:
                is_assign = 1;
            case I2D_BIT_AND:
                status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '&');
                break;
            case I2D_BIT_OR_ASSIGN:
                is_assign = 1;
            case I2D_BIT_OR:
                status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '|');
                break;
            case I2D_BIT_XOR_ASSIGN:
                is_assign = 1;
            case I2D_BIT_XOR:
                status = i2d_range_list_compute(&node->range, node->left->range, node->right->range, '^' + 'b');
                break;
            case I2D_GREATER:
                if(is_conditional) {
                    if(i2d_range_list_compute(&node->range, node->left->range, node->right->range, '>')) {
                        status = i2d_panic("failed to compute range object");
                    } else if(!i2d_node_get_predicate(node, &predicate)) {
                        if(i2d_logic_init(&node->logic, &predicate, node->range))
                            status = i2d_panic("failed to create logic object");
                    }
                } else {
                    status = i2d_range_list_init2(&node->range, 0, 1);
                }
                break;
            case I2D_GREATER_EQUAL:
                if(is_conditional) {
                    if(i2d_range_list_compute(&node->range, node->left->range, node->right->range, '>' + '=')) {
                        status = i2d_panic("failed to compute range object");
                    } else if(!i2d_node_get_predicate(node, &predicate)) {
                        if(i2d_logic_init(&node->logic, &predicate, node->range))
                            status = i2d_panic("failed to create logic object");
                    }
                } else {
                    status = i2d_range_list_init2(&node->range, 0, 1);
                }
                break;
            case I2D_LESS:
                if(is_conditional) {
                    if(i2d_range_list_compute(&node->range, node->left->range, node->right->range, '<')) {
                        status = i2d_panic("failed to compute range object");
                    } else if(!i2d_node_get_predicate(node, &predicate)) {
                        if(i2d_logic_init(&node->logic, &predicate, node->range))
                            status = i2d_panic("failed to create logic object");
                    }
                } else {
                    status = i2d_range_list_init2(&node->range, 0, 1);
                }
                break;
            case I2D_LESS_EQUAL:
                if(is_conditional) {
                    if(i2d_range_list_compute(&node->range, node->left->range, node->right->range, '<' + '=')) {
                        status = i2d_panic("failed to compute range object");
                    } else if(!i2d_node_get_predicate(node, &predicate)) {
                        if(i2d_logic_init(&node->logic, &predicate, node->range))
                            status = i2d_panic("failed to create logic object");
                    }
                } else {
                    status = i2d_range_list_init2(&node->range, 0, 1);
                }
                break;
            case I2D_EQUAL:
                if(is_conditional) {
                    if(i2d_range_list_compute(&node->range, node->left->range, node->right->range, '=' + '=')) {
                        status = i2d_panic("failed to compute range object");
                    } else if(!i2d_node_get_predicate(node, &predicate)) {
                        if(i2d_logic_init(&node->logic, &predicate, node->range))
                            status = i2d_panic("failed to create logic object");
                    }
                } else {
                    status = i2d_range_list_init2(&node->range, 0, 1);
                }
                break;
            case I2D_NOT_EQUAL:
                if(is_conditional) {
                    if(i2d_range_list_compute(&node->range, node->left->range, node->right->range, '!' + '=')) {
                        status = i2d_panic("failed to compute range object");
                    } else if(!i2d_node_get_predicate(node, &predicate)) {
                        if(i2d_logic_init(&node->logic, &predicate, node->range))
                            status = i2d_panic("failed to create logic object");
                    }
                } else {
                    status = i2d_range_list_init2(&node->range, 0, 1);
                }
                break;
            case I2D_AND:
                if(is_conditional) {
                    if(i2d_range_list_compute(&node->range, node->left->range, node->right->range, '&' + '&')) {
                        status = i2d_panic("failed to compute range object");
                    } else if(node->left->logic && node->right->logic) {
                        if(i2d_logic_and(&node->logic, node->left->logic, node->right->logic))
                            status = i2d_panic("failed to create logic object");
                    }
                } else {
                    status = i2d_range_list_init2(&node->range, 0, 1);
                }
                break;
            case I2D_OR:
                if(is_conditional) {
                    if(i2d_range_list_compute(&node->range, node->left->range, node->right->range, '|' + '|')) {
                        status = i2d_panic("failed to compute range object");
                    } else if(node->left->logic && node->right->logic) {
                        if(i2d_logic_and(&node->logic, node->left->logic, node->right->logic))
                            status = i2d_panic("failed to create logic object");
                    }
                } else {
                    status = i2d_range_list_init2(&node->range, 0, 1);
                }
                break;
            case I2D_ASSIGN:
                is_assign = 1;

                status = i2d_node_copy(node, node->right);
                break;
            default:
                status = i2d_panic("invalid token type -- %d", node->tokens->type);
                break;
        }

        if(!status && is_assign) {
            if(i2d_node_copy(node->left, node)) {
                status = i2d_panic("failed to copy range list object");
            } else if(i2d_context_insert_variable(script->context, node->left)) {
                status = i2d_panic("failed to map variable");
            }
        }
    }

    return status;
}

#if i2d_debug
int i2d_script_test(i2d_script * script, i2d_item * item) {
    int status = I2D_OK;
    i2d_str * description = NULL;

    if(i2d_script_compile(script, &item->script, &description)) {
        status = i2d_panic("failed to translate script for item %ld", item->id);
    } else {
        i2d_deit(description, i2d_str_deit);
    }

    if(i2d_script_compile(script, &item->onequip_script, &description)) {
        status = i2d_panic("failed to translate script for item %ld", item->id);
    } else {
        i2d_deit(description, i2d_str_deit);
    }

    if(i2d_script_compile(script, &item->onunequip_script, &description)) {
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
