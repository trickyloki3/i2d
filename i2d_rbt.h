#ifndef i2d_rbt_h
#define i2d_rbt_h

#include "i2d_util.h"

typedef struct i2d_rbt i2d_rbt;

int i2d_rbt_init(i2d_rbt **);
void i2d_rbt_deit(i2d_rbt **);
int i2d_rbt_insert(i2d_rbt *, int, void *);
int i2d_rbt_delete(i2d_rbt *, int);
int i2d_rbt_search(i2d_rbt *, int, void **);
#endif
