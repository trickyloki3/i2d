#ifndef i2d_constant_h
#define i2d_constant_h

#include "i2d_util.h"
#include "i2d_range.h"
#include "i2d_rbt.h"
#include "i2d_json.h"

struct i2d_constant {
    i2d_string macro;
    i2d_string name;
    i2d_range range;
    long value;
};

typedef struct i2d_constant i2d_constant;

int i2d_constant_create(i2d_constant *, const char *, json_t *);
void i2d_constant_destroy(i2d_constant *);

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
};

typedef struct i2d_constant_db i2d_constant_db;

int i2d_constant_db_init(i2d_constant_db **, json_t *);
void i2d_constant_db_deit(i2d_constant_db **);
int i2d_constant_get_by_macro_value(i2d_constant_db *, const char *, long *);
int i2d_constant_get_by_macro(i2d_constant_db *, const char *, i2d_constant **);
int i2d_constant_get_by_element(i2d_constant_db *, const long *, i2d_constant **);
int i2d_constant_get_by_race(i2d_constant_db *, const long *, i2d_constant **);
int i2d_constant_get_by_class(i2d_constant_db *, const long *, i2d_constant **);
int i2d_constant_get_by_location(i2d_constant_db *, const long *, i2d_constant **);
int i2d_constant_get_by_mapflag(i2d_constant_db *, const long *, i2d_constant **);
int i2d_constant_get_by_gettime(i2d_constant_db *, const long *, i2d_constant **);
int i2d_constant_get_by_readparam(i2d_constant_db *, const long *, i2d_constant **);
int i2d_constant_get_by_size(i2d_constant_db *, const long *, i2d_constant **);
#endif
