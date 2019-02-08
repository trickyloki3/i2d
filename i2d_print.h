#ifndef i2d_print_h
#define i2d_print_h

#include "i2d_data.h"
#include "i2d_json.h"
#include "i2d_item.h"

struct i2d_print {
    i2d_value_map * description_by_item_type;
    i2d_data_map * description_of_item_property;
    i2d_rbt * print_handlers;
    i2d_rbt * item_properties;
    i2d_buffer_cache * buffer_cache;
    i2d_string_stack_cache * stack_cache;
    i2d_value_map * item_type;
    i2d_value_map * item_location;
    i2d_value_map * ammo_type;
    i2d_value_map * weapon_type;
    i2d_value_map * gender;
    i2d_value_map * refineable;
    i2d_value_map * job;
    i2d_value_map * job_group;
    i2d_value_map * class;
    i2d_value_map * class_group;
};

typedef struct i2d_print i2d_print;

int i2d_print_init(i2d_print **, i2d_json *);
void i2d_print_deit(i2d_print **);
int i2d_print_format(i2d_print *, i2d_item *);
int i2d_print_format_lua(i2d_print *, i2d_item *, i2d_string *);
#endif
