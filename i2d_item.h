#ifndef i2d_item_h
#define i2d_item_h

#include "i2d_util.h"
#include "i2d_rbt.h"

struct i2d_item {
    long id;
    i2d_string aegis_name;
    i2d_string name;
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
    long max_level;
    long refineable;
    long view;
    i2d_string script;
    i2d_string onequip_script;
    i2d_string onunequip_script;
    struct i2d_item * next;
    struct i2d_item * prev;
};

typedef struct i2d_item i2d_item;

int i2d_item_init(i2d_item **, char *, size_t);
void i2d_item_deit(i2d_item **);
void i2d_item_append(i2d_item *, i2d_item *);
void i2d_item_remove(i2d_item *);

struct i2d_item_db {
    i2d_item * list;
    size_t size;
    i2d_rbt * index_by_id;
    i2d_rbt * index_by_name;
};

typedef struct i2d_item_db i2d_item_db;

int i2d_item_db_init(i2d_item_db **, i2d_string *);
void i2d_item_db_deit(i2d_item_db **);
int i2d_item_db_search_by_id(i2d_item_db *, long, i2d_item **);
int i2d_item_db_search_by_name(i2d_item_db *, const char *, i2d_item **);
#endif
