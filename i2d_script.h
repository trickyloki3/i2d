#ifndef i2d_script_h
#define i2d_script_h

#include "i2d_util.h"

enum i2d_token_type {
    I2D_CURLY_OPEN,
    I2D_CURLY_CLOSE
};

struct i2d_token {
    enum i2d_token_type type;
    char * string;
    size_t length;
};

typedef struct i2d_token i2d_token;

struct i2d_lexer {
    i2d_token * tokens;
    size_t size;
    i2d_buf * array;
    i2d_buf * buffer;
};

typedef struct i2d_lexer i2d_lexer;

int i2d_lexer_init(i2d_lexer **);
void i2d_lexer_deit(i2d_lexer **);
#endif
