#ifndef i2d_skill_h
#define i2d_skill_h

#include "i2d_util.h"

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
    i2d_str * cast_cancel;
    int cast_def_reduce_rate;
    long inf2;
    long * maxcount;
    size_t maxcount_size;
    i2d_str * type;
    long * blow_count;
    size_t blow_count_size;
    long inf3;
    i2d_str * macro;
    i2d_str * name;
};

typedef struct i2d_skill i2d_skill;

struct i2d_skill_db {
    i2d_skill * list;
    size_t size;
};

typedef struct i2d_skill_db i2d_skill_db;

int i2d_skill_db_init(i2d_skill_db **, i2d_str *);
void i2d_skill_db_deit(i2d_skill_db **);

#endif
