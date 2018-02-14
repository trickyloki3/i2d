#ifndef i2d_item_h
#define i2d_item_h

#include "i2d_util.h"
#include "i2d_rbt.h"

struct i2d_item {
    long id;
    i2d_str * aegis_name;
    i2d_str * name;
    long type;
    long buy;
    long sell;
    long weight;
    long atk;
    long matk;
    long def;
    long range;
    long slots;
    unsigned long job;
    unsigned long upper;
    long gender;
    unsigned long location;
    long weapon_level;
    long base_level;
    long refineable;
    long view;
    i2d_str * script;
    i2d_str * onequip_script;
    i2d_str * onunequip_script;
    struct i2d_item * next;
    struct i2d_item * prev;
};

typedef struct i2d_item i2d_item;

int i2d_item_init(i2d_item **, char *, size_t);
void i2d_item_deit(i2d_item **);

struct i2d_item_db {
    i2d_item * list;
    i2d_rbt * index_by_id;
    i2d_rbt * index_by_aegis;
    i2d_rbt * index_by_name;
    size_t size;
};

typedef struct i2d_item_db i2d_item_db;

int i2d_item_db_init(i2d_item_db **, i2d_str *);
void i2d_item_db_deit(i2d_item_db **);
int i2d_item_db_search_by_id(i2d_item_db *, long, i2d_item **);
#endif
