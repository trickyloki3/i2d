#include "i2d_script.h"

static void i2d_lexer_reset(i2d_lexer *);
static int i2d_lexer_add_symbol(i2d_lexer *, enum i2d_token_type);
static int i2d_lexer_add_string(i2d_lexer *, enum i2d_token_type, char *, size_t);

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

static void i2d_lexer_reset(i2d_lexer * lexer) {
    lexer->size = 0;
    lexer->array->offset = 0;
    lexer->buffer->offset = 0;
}

static int i2d_lexer_add_symbol(i2d_lexer * lexer, enum i2d_token_type type) {
    int status = I2D_OK;
    i2d_token * token = NULL;

    if(i2d_buf_object(lexer->array, sizeof(*token), (void **) &token)) {
        status = i2d_panic("failed to create token object");
    } else {
        token->type = type;

        if(!status)
            lexer->size++;
    }

    return status;
}

static int i2d_lexer_add_string(i2d_lexer * lexer, enum i2d_token_type type, char * string, size_t length) {
    int status = I2D_OK;
    i2d_token * token = NULL;

    if(!string || !length) {
        status = i2d_panic("invalid paramater");
    } else if(i2d_lexer_add_symbol(lexer, type)) {
        status = i2d_panic("failed to add symbol");
    } else {
        token = &lexer->tokens[lexer->size - 1];
        if(i2d_buf_object(lexer->buffer, length + 1, (void **) &token->string)) {
            status = i2d_panic("failed to create string object");
        } else {
            memcpy(token->string, string, length);
            token->length = length;
            token->string[token->length] = 0;
        }
    }

    return status;
}

#if i2d_debug
int i2d_lexer_test(void) {
    int status = I2D_OK;
    i2d_lexer * lexer = NULL;

    assert(!i2d_lexer_init(&lexer));
    assert(!i2d_lexer_add_string(lexer, I2D_IDENTIFIER, "variable", 8));
    assert(!i2d_lexer_add_string(lexer, I2D_IDENTIFIER, "identifier", 10));
    assert(I2D_IDENTIFIER == lexer->tokens[0].type);
    assert(!strcmp("variable", lexer->tokens[0].string));
    assert(8 == lexer->tokens[0].length);
    assert(I2D_IDENTIFIER == lexer->tokens[1].type);
    assert(!strcmp("identifier", lexer->tokens[1].string));
    assert(10 == lexer->tokens[1].length);
    i2d_lexer_reset(lexer);
    i2d_lexer_deit(&lexer);

    return status;
}
#endif
