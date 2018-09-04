#ifndef i2d_constant_h
#define i2d_constant_h

#include "i2d_util.h"
#include "i2d_range.h"
#include "i2d_rbt.h"
#include "i2d_json.h"
#include "i2d_mob.h"

struct i2d_constant {
    i2d_string macro;
    i2d_string name;
    i2d_range range;
    long value;
};

typedef struct i2d_constant i2d_constant;

int i2d_constant_create(i2d_constant *, const char *, json_t *);
void i2d_constant_destroy(i2d_constant *);

struct i2d_constant_bf {
    i2d_constant * BF_SHORT;
    i2d_constant * BF_LONG;
    i2d_constant * BF_WEAPON;
    i2d_constant * BF_MAGIC;
    i2d_constant * BF_MISC;
    i2d_constant * BF_NORMAL;
    i2d_constant * BF_SKILL;
};

typedef struct i2d_constant_bf i2d_constant_bf;

struct i2d_constant_atf {
    i2d_constant * ATF_SELF;
    i2d_constant * ATF_TARGET;
    i2d_constant * ATF_SHORT;
    i2d_constant * ATF_LONG;
    i2d_constant * ATF_WEAPON;
    i2d_constant * ATF_MAGIC;
    i2d_constant * ATF_MISC;
};

typedef struct i2d_constant_atf i2d_constant_atf;

struct i2d_constant_db {
    i2d_constant * constants;
    size_t size;

    i2d_rbt * macros;
    i2d_rbt * elements;
    i2d_rbt * races;
    i2d_rbt * classes;
    i2d_rbt * locations;
    i2d_rbt * mapflags;
    i2d_rbt * gettimes;
    i2d_rbt * readparam;
    i2d_rbt * sizes;
    i2d_rbt * jobs;
    i2d_rbt * effects;
    i2d_rbt * itemgroups;
    i2d_rbt * options;
    i2d_rbt * announces;
    i2d_rbt * sc_end;
    i2d_rbt * mob_races;

    i2d_constant_bf bf;
    i2d_constant_atf atf;
};

typedef struct i2d_constant_db i2d_constant_db;

int i2d_constant_db_init(i2d_constant_db **, json_t *);
void i2d_constant_db_deit(i2d_constant_db **);
int i2d_constant_index_mob_races(i2d_constant_db *, i2d_mob_race_db *);
int i2d_constant_get_by_macro_value(i2d_constant_db *, const char *, long *);
int i2d_constant_get_by_macro(i2d_constant_db *, const char *, i2d_constant **);
int i2d_constant_get_by_element(i2d_constant_db *, const long, i2d_constant **);
int i2d_constant_get_by_race(i2d_constant_db *, const long, i2d_constant **);
int i2d_constant_get_by_class(i2d_constant_db *, const long, i2d_constant **);
int i2d_constant_get_by_location(i2d_constant_db *, const long, i2d_constant **);
int i2d_constant_get_by_mapflag(i2d_constant_db *, const long, i2d_constant **);
int i2d_constant_get_by_gettime(i2d_constant_db *, const long, i2d_constant **);
int i2d_constant_get_by_readparam(i2d_constant_db *, const long, i2d_constant **);
int i2d_constant_get_by_size(i2d_constant_db *, const long, i2d_constant **);
int i2d_constant_get_by_job(i2d_constant_db *, const long, i2d_constant **);
int i2d_constant_get_by_effect(i2d_constant_db *, const long, i2d_constant **);
int i2d_constant_get_by_itemgroups(i2d_constant_db *, const long, i2d_constant **);
int i2d_constant_get_by_options(i2d_constant_db *, const long, i2d_constant **);
int i2d_constant_get_by_announces(i2d_constant_db *, const long, i2d_constant **);
int i2d_constant_get_by_sc_end(i2d_constant_db *, const long, i2d_constant **);
int i2d_constant_get_by_mob_races(i2d_constant_db *, const long, i2d_constant **);
#endif
