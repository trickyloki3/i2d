#include "i2d_script.h"

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
        result->string = token->buffer->buffer;
        result->length = token->buffer->offset;
    }

    return status;
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
            if(i2d_token_init(&object->list, I2D_HEAD))
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
    i2d_lexer_reset(object);
    i2d_deit(object->list, i2d_token_deit);
    i2d_free(object);
    *result = NULL;
}

void i2d_lexer_reset(i2d_lexer * lexer) {
    i2d_token * token = NULL;

    if(lexer->list) {
        while(lexer->list != lexer->list->next) {
            token = lexer->list->next;
            i2d_token_remove(token);
            i2d_token_deit(&token);
        }
    }
}

int i2d_lexer_tokenize(i2d_lexer * lexer, i2d_str * script) {
    int status = I2D_OK;
    size_t i;
    char symbol;
    i2d_token * token = NULL;
    i2d_token * state = NULL;

    i2d_lexer_reset(lexer);

    for(i = 0; i < script->length && !status; i++) {
        symbol = script->string[i];
        token = NULL;

        switch(symbol) {
            case  '{': status = i2d_token_init(&token, I2D_CURLY_OPEN); break;
            case  '}': status = i2d_token_init(&token, I2D_CURLY_CLOSE); break;
            case  '(': status = i2d_token_init(&token, I2D_PARENTHESIS_OPEN); break;
            case  ')': status = i2d_token_init(&token, I2D_PARENTHESIS_CLOSE); break;
            case  ',': status = i2d_token_init(&token, I2D_COMMA); break;
            case  ';': status = i2d_token_init(&token, I2D_SEMICOLON); break;
            case  '$': status = i2d_token_init(&token, I2D_PERMANENT_GLOBAL); break;
            case  '.': status = i2d_token_init(&token, I2D_TEMPORARY_NPC); break;
            case '\'': status = i2d_token_init(&token, I2D_TEMPORARY_INSTANCE); break;
            case  '@':
                if(state && I2D_PERMANENT_GLOBAL == state->type) {
                    state->type = I2D_TEMPORARY_GLOBAL;
                } else if(state && I2D_TEMPORARY_NPC == state->type) {
                    state->type = I2D_TEMPORARY_SCOPE;
                } else {
                    status = i2d_token_init(&token, I2D_TEMPORARY_CHARACTER); break;
                }
                break;
            case  '#':
                if(state && I2D_PERMANENT_ACCOUNT_LOCAL == state->type) {
                    state->type = I2D_PERMANENT_ACCOUNT_GLOBAL;
                } else {
                    status = i2d_token_init(&token, I2D_PERMANENT_ACCOUNT_LOCAL);
                }
                break;
            case  '+': status = i2d_token_init(&token, I2D_ADD); break;
            case  '-': status = i2d_token_init(&token, I2D_SUBTRACT); break;
            case  '*': status = i2d_token_init(&token, I2D_MULTIPLY); break;
            case  '/': status = i2d_token_init(&token, I2D_DIVIDE); break;
            case  '%': status = i2d_token_init(&token, I2D_MODULUS); break;
            case  '>': status = i2d_token_init(&token, I2D_GREATER); break;
            case  '<': status = i2d_token_init(&token, I2D_LESS); break;
            case  '!': status = i2d_token_init(&token, I2D_NOT); break;
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
                        default: status = i2d_token_init(&token, I2D_ASSIGN); break;
                    }
                } else {
                    status = i2d_token_init(&token, I2D_ASSIGN);
                }
                break;
            default:
                if('_' == symbol || isalpha(symbol) || isdigit(symbol)) {
                    if(state && I2D_LITERAL == state->type) {
                        status = i2d_token_write(state, &symbol, sizeof(symbol));
                    } else {
                        status = i2d_token_init(&token, I2D_LITERAL) ||
                                 i2d_token_write(token, &symbol, sizeof(symbol));
                    }
                } else if(isspace(symbol)) {
                    state = NULL;
                }
                break;
        }

        if(token) {
            i2d_token_append(token, lexer->list);
            state = token;
        }
    }

    return status;
}

#if i2d_debug
int i2d_lexer_test(void) {
    int status = I2D_OK;
    i2d_lexer * lexer = NULL;
    i2d_str * script = NULL;
    i2d_token * token = NULL;

    int i = 0;
    enum i2d_token_type sequence[] = {
        I2D_CURLY_OPEN,
        I2D_CURLY_CLOSE,
        I2D_PARENTHESIS_OPEN,
        I2D_PARENTHESIS_CLOSE,
        I2D_COMMA,
        I2D_SEMICOLON,
        I2D_LITERAL,
        I2D_LITERAL,
        I2D_LITERAL,
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
        I2D_ASSIGN
    };

    assert(!i2d_lexer_init(&lexer));
    assert(!i2d_str_copy(&script, "{}(),; _var1 var2 1234 0x11 @ $ $@ . .@ ' # ## + - * / % += -= *= /= %= > < ! == >= <= != =", 91));
    assert(!i2d_lexer_tokenize(lexer, script));
    token = lexer->list->next;
    while(token != lexer->list) {
        assert(token->type == sequence[i++]);
        token = token->next;
    }
    i2d_str_deit(&script);
    i2d_lexer_deit(&lexer);

    return status;
}
#endif
