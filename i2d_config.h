#ifndef i2d_config_h
#define i2d_config_h

#include "i2d_util.h"
#include "i2d_json.h"

struct i2d_config {
    i2d_string source_path;
    i2d_string data_path;
    int renewal;
    long item_id;
};

typedef struct i2d_config i2d_config;

int i2d_config_init(i2d_config **, i2d_string *);
void i2d_config_deit(i2d_config  **);
#endif