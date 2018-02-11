#include "i2d_script.h"

int i2d_token_init(i2d_lexer * lexer, i2d_token ** result, enum i2d_token_type type) {
    int status = I2D_OK;
    i2d_token * object = NULL;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        if(i2d_buf_object(lexer->tokens, sizeof(*object), (void **) &object)) {
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
                lexer->tokens->offset -= sizeof(*object);
            } else {
                *result = object;
                lexer->size++;
            }
        }
    }

    return status;
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
            if(i2d_buf_init(&object->tokens, 4096)) {
                status = i2d_panic("failed to create buffer object");
            } else if(i2d_token_init(object, &object->list, I2D_HEAD)) {
                status = i2d_panic("failed to create token object");
            }

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
    i2d_deit(object->tokens, i2d_buf_deit);
    i2d_free(object);
    *result = NULL;
}

void i2d_lexer_reset(i2d_lexer * lexer) {
    size_t i;
    i2d_token * tokens;

    tokens = (i2d_token *) lexer->tokens->buffer;
    for(i = 0; i < lexer->size; i++)
        i2d_deit(tokens[i].buffer, i2d_buf_deit);

    lexer->list = NULL;
    lexer->size = 0;
    lexer->tokens->offset = 0;
}

int i2d_lexer_tokenize(i2d_lexer * lexer, i2d_str * script) {
    int status = I2D_OK;
    size_t i;
    i2d_token * token = NULL;
    i2d_token * state = NULL;

    if(i2d_buf_fit(lexer->tokens, sizeof(i2d_token) * script->length)) {
        status = I2D_FAIL;
    } else {
        for(i = 0; i < script->length && !status; i++) {
            token = NULL;

            switch(script->string[i]) {
                case '{': status = i2d_token_init(lexer, &token, I2D_CURLY_OPEN); break;
                case '}': status = i2d_token_init(lexer, &token, I2D_CURLY_CLOSE); break;
                case '(': status = i2d_token_init(lexer, &token, I2D_PARENTHESIS_OPEN); break;
                case ')': status = i2d_token_init(lexer, &token, I2D_PARENTHESIS_CLOSE); break;
                case ',': status = i2d_token_init(lexer, &token, I2D_COMMA); break;
                case ';': status = i2d_token_init(lexer, &token, I2D_SEMICOLON); break;
            }

            if(token) {
                i2d_token_append(token, lexer->list);
                state = token;
            }
        }
    }

    return status;
}

#if i2d_debug
int i2d_lexer_test(void) {
    int status = I2D_OK;
    i2d_lexer * lexer = NULL;
    i2d_str * script = NULL;
    i2d_token * tokens = NULL;

    assert(!i2d_lexer_init(&lexer));
    assert(!i2d_str_copy(&script, "{}(),; _var1 var2", 6));
    assert(!i2d_lexer_tokenize(lexer, script));
    tokens = (i2d_token *) lexer->tokens->buffer;
    assert(tokens[1].type == I2D_CURLY_OPEN);
    assert(tokens[2].type == I2D_CURLY_CLOSE);
    assert(tokens[3].type == I2D_PARENTHESIS_OPEN);
    assert(tokens[4].type == I2D_PARENTHESIS_CLOSE);
    assert(tokens[5].type == I2D_COMMA);
    assert(tokens[6].type == I2D_SEMICOLON);
    i2d_str_deit(&script);
    i2d_lexer_deit(&lexer);

    return status;
}
#endif
