#ifndef i2d_json_h
#define i2d_json_h

#include "i2d_util.h"
#include "i2d_rbt.h"
#include "jansson.h"

struct i2d_keywords {
    i2d_str * list;
    size_t size;
    i2d_rbt * index;
};

typedef struct i2d_keywords i2d_keywords;

int i2d_keywords_init(i2d_keywords **, json_t *);
void i2d_keywords_deit(i2d_keywords **);

struct i2d_json {
    i2d_keywords * keywords;
};

typedef struct i2d_json i2d_json;

int i2d_json_init(i2d_json **, i2d_str *);
void i2d_json_deit(i2d_json **);
#endif
