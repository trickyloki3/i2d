#ifndef i2d_item_h
#define i2d_item_h

#include "i2d_util.h"

struct i2d_item_db {

};

typedef struct i2d_item_db i2d_item_db;

int i2d_item_db_init(i2d_item_db **, i2d_str *);
void i2d_item_db_deit(i2d_item_db **);
#endif
