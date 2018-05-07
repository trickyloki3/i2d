#ifndef i2d_constant_h
#define i2d_constant_h

#include "i2d_util.h"
#include "i2d_range.h"
#include "i2d_rbt.h"
#include "i2d_json.h"

struct i2d_constant {
    i2d_string macro;
    i2d_string name;
    i2d_range range;
    long value;
};

typedef struct i2d_constant i2d_constant;

int i2d_constant_create(i2d_constant *, const char *, json_t *);
void i2d_constant_destroy(i2d_constant *);
#endif
