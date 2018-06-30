#ifndef i2d_mercenary_h
#define i2d_mercenary_h

#include "i2d_util.h"
#include "i2d_rbt.h"

struct i2d_mercenary {
    long id;
    i2d_string sprite_name;
    i2d_string name;
    long level;
    long hp;
    long sp;
    long range1;
    long atk1;
    long atk2;
    long def;
    long mdef;
    long str;
    long agi;
    long vit;
    long ini;
    long dex;
    long luk;
    long range2;
    long range3;
    long scale;
    long race;
    long element;
    long speed;
    long adelay;
    long amotion;
    long dmotion;
    struct i2d_mercenary * next;
    struct i2d_mercenary * prev;
};

typedef struct i2d_mercenary i2d_mercenary;

int i2d_mercenary_init(i2d_mercenary **, char *, size_t);
void i2d_mercenary_deit(i2d_mercenary **);
void i2d_mercenary_append(i2d_mercenary *, i2d_mercenary *);
void i2d_mercenary_remove(i2d_mercenary *);

struct i2d_mercenary_db {
    i2d_mercenary * list;
    size_t size;
    i2d_rbt * index_by_id;
};

typedef struct i2d_mercenary_db i2d_mercenary_db;

int i2d_mercenary_db_init(i2d_mercenary_db **, i2d_string *);
void i2d_mercenary_db_deit(i2d_mercenary_db **);
int i2d_mercenary_db_search_by_id(i2d_mercenary_db *, long, i2d_mercenary **);
#endif
