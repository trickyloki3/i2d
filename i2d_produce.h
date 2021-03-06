#ifndef i2d_produce_h
#define i2d_produce_h

#include "i2d_util.h"
#include "i2d_rbt.h"

struct i2d_produce {
    long id;
    long item_id;
    long item_level;
    long skill_id;
    long skill_level;
    size_t material_count;
    long * materials;
    struct i2d_produce * next;
    struct i2d_produce * prev;
};

typedef struct i2d_produce i2d_produce;

int i2d_produce_init(i2d_produce **, char *, size_t);
void i2d_produce_deit(i2d_produce **);
void i2d_produce_append(i2d_produce *, i2d_produce *);
void i2d_produce_remove(i2d_produce *);

struct i2d_produce_list {
    long item_level;
    i2d_produce ** list;
    size_t size;
    struct i2d_produce_list * next;
    struct i2d_produce_list * prev;
};

typedef struct i2d_produce_list i2d_produce_list;

int i2d_produce_list_init(i2d_produce_list **, long);
void i2d_produce_list_deit(i2d_produce_list **);
int i2d_produce_list_add(i2d_produce_list *, i2d_produce *);
void i2d_produce_list_append(i2d_produce_list *, i2d_produce_list *);
void i2d_produce_list_remove(i2d_produce_list *);

struct i2d_produce_db {
    i2d_produce * list;
    size_t size;
    i2d_produce_list * produce_list;
    size_t produce_size;
    i2d_rbt * index_by_id;
    i2d_rbt * index_by_item_level;
};

typedef struct i2d_produce_db i2d_produce_db;

int i2d_produce_db_init(i2d_produce_db **, i2d_string *);
void i2d_produce_db_deit(i2d_produce_db **);
int i2d_produce_db_search_by_id(i2d_produce_db *, long, i2d_produce **);
int i2d_produce_db_search_by_item_level(i2d_produce_db *, long, i2d_produce_list **);
#endif
