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

struct i2d_item_combo {
    long * list;
    size_t size;
    i2d_string script;
    struct i2d_item_combo * next;
    struct i2d_item_combo * prev;
};

typedef struct i2d_item_combo i2d_item_combo;

int i2d_item_combo_init(i2d_item_combo **, char *, size_t);
void i2d_item_combo_deit(i2d_item_combo **);
void i2d_item_combo_append(i2d_item_combo *, i2d_item_combo *);
void i2d_item_combo_remove(i2d_item_combo *);

struct i2d_item_combo_list {
    long item_id;
    i2d_item_combo ** list;
    size_t size;
    struct i2d_item_combo_list * next;
    struct i2d_item_combo_list * prev;
};

typedef struct i2d_item_combo_list i2d_item_combo_list;

int i2d_item_combo_list_init(i2d_item_combo_list **, long);
void i2d_item_combo_list_deit(i2d_item_combo_list **);
int i2d_item_combo_list_add(i2d_item_combo_list *, i2d_item_combo *);
void i2d_item_combo_list_append(i2d_item_combo_list *, i2d_item_combo_list *);
void i2d_item_combo_list_remove(i2d_item_combo_list *);

struct i2d_item_combo_db {
    i2d_item_combo * list;
    size_t size;
    i2d_item_combo_list * combo_list;
    size_t combo_size;
    i2d_rbt * index_by_id;
};

typedef struct i2d_item_combo_db i2d_item_combo_db;

int i2d_item_combo_db_init(i2d_item_combo_db **, i2d_string *);
void i2d_item_combo_db_deit(i2d_item_combo_db **);
int i2d_item_combo_db_search_by_id(i2d_item_combo_db *, long, i2d_item_combo_list **);
#endif
