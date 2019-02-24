#ifndef i2d_range_h
#define i2d_range_h

#include "i2d_util.h"

struct i2d_range_node {
    long min;
    long max;
    struct i2d_range_node * next;
    struct i2d_range_node * prev;
};

typedef struct i2d_range_node i2d_range_node;

int i2d_range_node_init(i2d_range_node **, long, long);
void i2d_range_node_deit(i2d_range_node **);
void i2d_range_node_append(i2d_range_node *, i2d_range_node *);
void i2d_range_node_remove(i2d_range_node *);

struct i2d_range {
    i2d_range_node * list;
    long min;
    long max;
};

typedef struct i2d_range i2d_range;

int i2d_range_create(i2d_range *);
int i2d_range_create_add(i2d_range *, long, long);
void i2d_range_destroy(i2d_range *);
void i2d_range_print(i2d_range *, const char *);
int i2d_range_add(i2d_range *, long, long);
int i2d_range_copy(i2d_range *, i2d_range *);
int i2d_range_negate(i2d_range *, i2d_range *);
int i2d_range_bitnot(i2d_range *, i2d_range *);

typedef int (*i2d_range_merge_cb)(i2d_range *, i2d_range_node **, i2d_range_node *);
int i2d_range_merge(i2d_range *, i2d_range_node *, i2d_range_node *, i2d_range_merge_cb);
int i2d_range_merge_or(i2d_range *, i2d_range_node **, i2d_range_node *);
int i2d_range_merge_and(i2d_range *, i2d_range_node **, i2d_range_node *);

int i2d_range_or(i2d_range *, i2d_range *, i2d_range *);
int i2d_range_and(i2d_range *, i2d_range *, i2d_range *);
int i2d_range_not(i2d_range *, i2d_range *);

void i2d_range_get_range(i2d_range *, long *, long *);
void i2d_range_get_range_absolute(i2d_range *, long *, long *);
int i2d_range_compute(i2d_range *, i2d_range *, i2d_range *, int);

typedef int (*i2d_range_iterate_by_number_cb) (long, void *);
int i2d_range_iterate_by_number(i2d_range *, i2d_range_iterate_by_number_cb, void *);

typedef int (*i2d_range_iterate_by_range_cb) (i2d_range_node *, void *);
int i2d_range_iterate_by_range(i2d_range *, i2d_range_iterate_by_range_cb, void *);
#endif
