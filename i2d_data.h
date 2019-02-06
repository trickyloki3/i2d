#ifndef i2d_data_h
#define i2d_data_h

#include "i2d_util.h"
#include "i2d_json.h"
#include "i2d_constant.h"

struct i2d_data {
    long constant;
    i2d_string name;
    i2d_range range;
    i2d_string description;
    i2d_string handler;
    i2d_string_stack argument_type;
    i2d_string_stack argument_default;
    struct {
        long * list;
        size_t size;
    } argument_order;
    long required;
    long optional;
    i2d_string positive;
    i2d_string negative;
    i2d_string zero;
    int empty_description_on_zero;
    int empty_description_on_empty_string;
    int dump_stack_instead_of_description;
};

typedef struct i2d_data i2d_data;

int i2d_data_create(i2d_data *, const char *, json_t *, i2d_constant_db *);
void i2d_data_destroy(i2d_data *);

enum i2d_data_map_type {
    data_map_by_constant = 1,
    data_map_by_name
};

struct i2d_data_map {
    i2d_rbt * map;
    i2d_data * list;
    size_t size;
};

typedef struct i2d_data_map i2d_data_map;

int i2d_data_map_init(i2d_data_map **, enum i2d_data_map_type, json_t *, i2d_constant_db *);
void i2d_data_map_deit(i2d_data_map **);
int i2d_data_map_get(i2d_data_map *, void *, i2d_data **);

enum i2d_value_type {
    i2d_value_string,
    i2d_value_string_stack
};

struct i2d_value {
    long value;
    i2d_string string;
    i2d_string_stack stack;
};

typedef struct i2d_value i2d_value;

struct i2d_value_map {
    enum i2d_value_type type;
    i2d_rbt * map;
    i2d_value * list;
    size_t size;
};

typedef struct i2d_value_map i2d_value_map;

int i2d_value_map_init(i2d_value_map **, json_t *, enum i2d_value_type);
void i2d_value_map_deit(i2d_value_map **);
int i2d_value_map_get_string(i2d_value_map *, long, i2d_string *);
int i2d_value_map_get_string_stack(i2d_value_map *, long, i2d_string_stack *);
#endif
