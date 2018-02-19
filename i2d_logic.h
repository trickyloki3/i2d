#ifndef i2d_logic_h
#define i2d_logic_h

#include "i2d_range.h"

struct i2d_logic {
    int type;
    i2d_str * name;
    i2d_range_list * range;
    struct i2d_logic * parent;
    struct i2d_logic * left;
    struct i2d_logic * right;
};

typedef struct i2d_logic i2d_logic;

int i2d_logic_init(i2d_logic **, i2d_str *, i2d_range_list *);
void i2d_logic_deit(i2d_logic **);
void i2d_logic_print(i2d_logic *, int);
#endif
