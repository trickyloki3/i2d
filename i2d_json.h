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
int i2d_object_get_range_array(json_t *, i2d_range *);
int i2d_object_get_range(json_t *, json_t *, i2d_range *);
int i2d_object_get_list(json_t *, size_t, void **, size_t *);
int i2d_object_get_boolean(json_t *, int *);

struct i2d_json {
    json_t * statements;
    json_t * functions;
    json_t * constants;
    json_t * arguments;
    json_t * bonus_file;
    json_t * bonus;
    json_t * bonus2;
    json_t * bonus3;
    json_t * bonus4;
    json_t * bonus5;
    json_t * sc_start_file;
    json_t * sc_start;
    json_t * sc_start2;
    json_t * sc_start4;
    json_t * data_file;
    json_t * ammo_type;
    json_t * bonus_script_flag;
    json_t * getiteminfo_type;
    json_t * searchstore_effect;
    json_t * skill_flag;
    json_t * strcharinfo_type;
    json_t * weapon_type;
    json_t * item_type;
    json_t * item_location;
    json_t * job;
    json_t * job_group;
    json_t * class;
    json_t * class_group;
    json_t * gender;
    json_t * refineable;
    json_t * basejob;
    json_t * print_file;
    json_t * description_by_item_type;
    json_t * description_of_item_property;
};

typedef struct i2d_json i2d_json;

int i2d_json_init(i2d_json **, i2d_string *);
void i2d_json_deit(i2d_json **);
#endif
