#include "i2d_script.h"

static int i2d_lexer_add_token(i2d_lexer *, enum i2d_token_type, char *, size_t);

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
            if( i2d_buf_init(&object->array, 4096) ||
                i2d_buf_init(&object->buffer, 4096) ) {
                status = i2d_panic("failed to create buffer objects");
        } else {
            object->tokens = (i2d_token *) object->array->buffer;
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
    i2d_deit(object->buffer, i2d_buf_deit);
    i2d_deit(object->array, i2d_buf_deit);
    i2d_free(object);
    *result = NULL;
}

static int i2d_lexer_add_token(i2d_lexer * lexer, enum i2d_token_type type, char * string, size_t length) {
    int status = I2D_OK;
    i2d_token * token = NULL;

    if(i2d_buf_object(lexer->array, sizeof(*token), (void **) &token)) {
        status = i2d_panic("failed to create token object");
    } else {
        token->type = type;

        if(string && length) {
            if(i2d_buf_object(lexer->buffer, length + 1, (void **) &token->string)) {
                status = i2d_panic("failed to create string object");
            } else {
                memcpy(token->string, string, length);
                token->length = length;
                token->string[token->length] = 0;
            }
        }
    }

    return status;
}
