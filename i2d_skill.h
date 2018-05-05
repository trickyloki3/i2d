#ifndef i2d_skill_h
#define i2d_skill_h

#include "i2d_util.h"
#include "i2d_rbt.h"

struct i2d_skill {
    long id;
    long * range;
    size_t range_size;
    long hit;
    long inf;
    long * element;
    size_t element_size;
    long nk;
    long * splash;
    size_t splash_size;
    long maxlv;
    long * hit_amount;
    size_t hit_amount_size;
    i2d_string cast_cancel;
    long cast_def_reduce_rate;
    long inf2;
    long * max_count;
    size_t max_count_size;
    i2d_string type;
    long * blow_count;
    size_t blow_count_size;
    long inf3;
    i2d_string macro;
    i2d_string name;
    struct i2d_skill * next;
    struct i2d_skill * prev;
};

typedef struct i2d_skill i2d_skill;

int i2d_skill_init(i2d_skill **, char *, size_t);
void i2d_skill_deit(i2d_skill **);
void i2d_skill_append(i2d_skill *, i2d_skill *);
void i2d_skill_remove(i2d_skill *);

struct i2d_skill_db {
    i2d_skill * list;
    size_t size;
    i2d_rbt * index_by_id;
    i2d_rbt * index_by_macro;
};

typedef struct i2d_skill_db i2d_skill_db;

int i2d_skill_db_init(i2d_skill_db **, i2d_string *);
void i2d_skill_db_deit(i2d_skill_db **);
int i2d_skill_db_search_by_id(i2d_skill_db *, long, i2d_skill **);
int i2d_skill_db_search_by_macro(i2d_skill_db *, i2d_string *, i2d_skill **);

#endif
