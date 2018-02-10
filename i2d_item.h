#ifndef i2d_item_h
#define i2d_item_h

#include "i2d_util.h"

struct i2d_item {
    int id;
    i2d_str * aegis_name;
    i2d_str * name;
    int type;
    int buy;
    int sell;
    int weight;
    int atk;
    int matk;
    int def;
    int range;
    int slots;
    unsigned job;
    unsigned upper;
    int gender;
    unsigned location;
    int weapon_level;
    int base_level;
    int refineable;
    int view;
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
    i2d_item * item_list;
    size_t item_count;
};

typedef struct i2d_item_db i2d_item_db;

int i2d_item_db_init(i2d_item_db **, i2d_str *);
void i2d_item_db_deit(i2d_item_db **);
#endif
