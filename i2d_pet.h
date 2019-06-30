#ifndef i2d_pet_h
#define i2d_pet_h

#include "i2d_util.h"
#include "i2d_rbt.h"
#include "i2d_yaml.h"

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

int i2d_pet_init(i2d_pet **);
void i2d_pet_deit(i2d_pet **);
void i2d_pet_append(i2d_pet *, i2d_pet *);
void i2d_pet_remove(i2d_pet *);

struct i2d_pet_item {
    i2d_string item;
    long amount;
    struct i2d_pet_item * next;
    struct i2d_pet_item * prev;
};

typedef struct i2d_pet_item i2d_pet_item;

int i2d_pet_item_init(i2d_pet_item **);
void i2d_pet_item_deit(i2d_pet_item **);
void i2d_pet_item_append(i2d_pet_item *, i2d_pet_item *);
void i2d_pet_item_remove(i2d_pet_item *);

struct i2d_pet_evolution {
    i2d_string target;
    i2d_pet_item * item_list;
    struct i2d_pet_evolution * next;
    struct i2d_pet_evolution * prev;
};

typedef struct i2d_pet_evolution i2d_pet_evolution;

int i2d_pet_evolution_init(i2d_pet_evolution **);
void i2d_pet_evolution_deit(i2d_pet_evolution **);
void i2d_pet_evolution_append(i2d_pet_evolution *, i2d_pet_evolution *);
void i2d_pet_evolution_remove(i2d_pet_evolution *);

struct i2d_pet_yml {
    i2d_string mob;
    i2d_string tame_item;
    i2d_string egg_item;
    i2d_string equip_item;
    i2d_string food_item;
    long fullness;
    long hungry_delay;
    long hungry_increase;
    long intimacy_start;
    long intimacy_fed;
    long intimacy_overfed;
    long intimacy_hungry;
    long intimacy_owner_die;
    long capture_rate;
    long special_performance;
    long attack_rate;
    long retaliate_rate;
    long change_target_rate;
    long allow_auto_feed;
    i2d_string script;
    i2d_string support_script;
    struct i2d_pet_evolution * evolution_list;
    struct i2d_pet_yml * next;
    struct i2d_pet_yml * prev;
};

typedef struct i2d_pet_yml i2d_pet_yml;

int i2d_pet_yml_init(i2d_pet_yml **);
void i2d_pet_yml_deit(i2d_pet_yml **);
void i2d_pet_yml_append(i2d_pet_yml *, i2d_pet_yml *);
void i2d_pet_yml_remove(i2d_pet_yml *);

struct i2d_pet_db {
    i2d_pet * list;
    i2d_rbt * index;
};

typedef struct i2d_pet_db i2d_pet_db;

int i2d_pet_db_init(i2d_pet_db **, i2d_string *);
void i2d_pet_db_deit(i2d_pet_db **);
int i2d_pet_db_search_by_id(i2d_pet_db *, long, i2d_pet **);
#endif
