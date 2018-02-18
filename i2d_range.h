#ifndef i2d_range_h
#define i2d_range_h

#include "i2d_util.h"

struct i2d_range {
    long min;
    long max;
    struct i2d_range * next;
    struct i2d_range * prev;
};

typedef struct i2d_range i2d_range;

int i2d_range_init(i2d_range **, long, long);
void i2d_range_deit(i2d_range **);
void i2d_range_append(i2d_range *, i2d_range *);
void i2d_range_remove(i2d_range *);
int i2d_range_add(i2d_range *, long, long);

struct i2d_range_list {
    i2d_range * list;
};

typedef struct i2d_range_list i2d_range_list;

int i2d_range_list_init(i2d_range_list **);
void i2d_range_list_deit(i2d_range_list **);
void i2d_range_list_print(i2d_range_list *);
int i2d_range_list_add(i2d_range_list *, long, long);
int i2d_range_list_copy(i2d_range_list **, i2d_range_list *);
int i2d_range_list_negate(i2d_range_list **, i2d_range_list *);

typedef int (*i2d_range_merge_cb)(i2d_range_list *, i2d_range **, i2d_range *);
int i2d_range_list_merge(i2d_range_list *, i2d_range *, i2d_range *, i2d_range_merge_cb);
int i2d_range_list_merge_or(i2d_range_list *, i2d_range **, i2d_range *);
int i2d_range_list_merge_and(i2d_range_list *, i2d_range **, i2d_range *);

int i2d_range_list_or(i2d_range_list *, i2d_range_list *, i2d_range_list **);
int i2d_range_list_and(i2d_range_list *, i2d_range_list *, i2d_range_list **);
#endif
