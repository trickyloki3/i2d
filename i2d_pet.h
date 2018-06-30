#ifndef i2d_pet_h
#define i2d_pet_h

#include "i2d_util.h"
#include "i2d_rbt.h"

struct i2d_pet {
    long id;
    i2d_string name;
    i2d_string jname;
    long lure_id;
    long egg_id;
    long equip_id;
    long food_id;
    long fullness;
    long hungry_delay;
    long r_hungry;
    long r_full;
    long intimate;
    long die;
    long capture;
    long speed;
    long s_performance;
    long talk_convert_class;
    long attack_rate;
    long defence_attack_rate;
    long change_target_rate;
    i2d_string pet_script;
    i2d_string loyal_script;
    struct i2d_pet * next;
    struct i2d_pet * prev;
};

typedef struct i2d_pet i2d_pet;

int i2d_pet_init(i2d_pet **, char *, size_t);
void i2d_pet_deit(i2d_pet **);
void i2d_pet_append(i2d_pet *, i2d_pet *);
void i2d_pet_remove(i2d_pet *);

struct i2d_pet_db {
    i2d_pet * list;
    size_t size;
    i2d_rbt * index_by_id;
};

typedef struct i2d_pet_db i2d_pet_db;

int i2d_pet_db_init(i2d_pet_db **, i2d_string *);
void i2d_pet_db_deit(i2d_pet_db **);
int i2d_pet_db_search_by_id(i2d_pet_db *, long, i2d_pet **);
#endif
