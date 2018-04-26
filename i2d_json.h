#ifndef i2d_json_h
#define i2d_json_h

#include "i2d_util.h"
#include "i2d_rbt.h"
#include "jansson.h"

struct i2d_json {
    json_t * object;
};

typedef struct i2d_json i2d_json;

int i2d_json_init(i2d_json **, i2d_str *);
void i2d_json_deit(i2d_json **);
int i2d_json_block_map(i2d_json *, const char *, json_t **);

int i2d_json_get_str(json_t *, const char *, i2d_str_const *);
int i2d_json_get_int(json_t *, const char *, json_int_t *);

struct i2d_str_list {
    i2d_str ** list;
    size_t size;
};

typedef struct i2d_str_list i2d_str_list;

int i2d_str_list_init(i2d_str_list **, const char *, json_t *);
void i2d_str_list_deit(i2d_str_list **);

#if i2d_debug
int i2d_json_test();
#endif
#endif
