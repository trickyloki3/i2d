#ifndef i2d_json_h
#define i2d_json_h

#include "i2d_util.h"
#include "i2d_range.h"
#include "i2d_rbt.h"
#include "jansson.h"

int i2d_json_create(json_t **, i2d_string *);
void i2d_json_destroy(json_t *);

int i2d_object_get_string_stack(json_t *, i2d_string_stack *);
int i2d_object_get_string(json_t *, i2d_string *);
int i2d_object_get_number_array(json_t *, long **, size_t *);
int i2d_object_get_number(json_t *, long *);
int i2d_object_get_range(json_t *, json_t *, i2d_range *);
int i2d_object_get_list(json_t *, size_t, void **, size_t *);
int i2d_object_get_boolean(json_t *, int *);

struct i2d_json {
    json_t * statements;
    json_t * functions;
    json_t * constants;
    json_t * arguments;
    json_t * prefixes;
    json_t * bonus_file;
    json_t * bonus;
    json_t * bonus2;
    json_t * bonus3;
    json_t * bonus4;
    json_t * bonus5;
    json_t * data_file;
    json_t * ammo_type;
    json_t * bonus_script_flag;
    json_t * getiteminfo_type;
    json_t * searchstore_effect;
    json_t * skill_flag;
    json_t * strcharinfo_type;
    json_t * weapon_type;
};

typedef struct i2d_json i2d_json;

int i2d_json_init(i2d_json **, i2d_string *);
void i2d_json_deit(i2d_json **);

struct i2d_value {
    long value;
    i2d_string name;
};

typedef struct i2d_value i2d_value;

struct i2d_value_map {
    i2d_rbt * map;
    i2d_value * list;
    size_t size;
};

typedef struct i2d_value_map i2d_value_map;

int i2d_value_map_init(i2d_value_map **, json_t *);
void i2d_value_map_deit(i2d_value_map **);
int i2d_value_map_get(i2d_value_map *, long, i2d_string *);
#endif
