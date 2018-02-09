#ifndef i2d_util_h
#define i2d_util_h

#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "string.h"

#define I2D_OK      0
#define I2D_FAIL    1

#define i2d_panic(format, ...) i2d_panic_func("%s (%s:%zu): " format ".\n", __FILE__, __func__, __LINE__, ## __VA_ARGS__)
int i2d_panic_func(const char *, ...);

#define i2d_is_invalid(ptr) (!(ptr) || *(ptr))
#define i2d_free(ptr) if(ptr) { free(ptr); (ptr) = NULL; }

struct i2d_str {
    char * string;
    size_t length;
};

typedef struct i2d_str i2d_str;

int i2d_str_init(i2d_str **, const char *, size_t);
void i2d_str_deit(i2d_str **);
#endif