#ifndef i2d_json_h
#define i2d_json_h

#include "i2d_util.h"

struct i2d_json {
    struct {
        i2d_str * list;
        size_t size;
    } keywords;
};

typedef struct i2d_json i2d_json;

int i2d_json_init(i2d_json **, i2d_str *);
void i2d_json_deit(i2d_json **);
#endif
