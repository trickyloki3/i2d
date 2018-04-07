#ifndef i2d_json_h
#define i2d_json_h

#include "i2d_util.h"
#include "i2d_rbt.h"
#include "jansson.h"

struct i2d_block_data {
    i2d_str * name;
    struct i2d_block_data * next;
    struct i2d_block_data * prev;
};

typedef struct i2d_block_data i2d_block_data;

int i2d_block_data_init(i2d_block_data **, json_t *);
void i2d_block_data_deit(i2d_block_data **);
void i2d_block_data_list_deit(i2d_block_data **);
void i2d_block_data_append(i2d_block_data *, i2d_block_data *);
void i2d_block_data_remove(i2d_block_data *);

struct i2d_json {
    i2d_block_data * block_data_list;
    i2d_rbt * block_data_index;
};

typedef struct i2d_json i2d_json;

int i2d_json_init(i2d_json **, i2d_str *);
void i2d_json_deit(i2d_json **);

#if i2d_debug
int i2d_json_test();
#endif
#endif
