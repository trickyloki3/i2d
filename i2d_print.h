#ifndef i2d_print_h
#define i2d_print_h

#include "i2d_data.h"
#include "i2d_json.h"
#include "i2d_item.h"

struct i2d_print {
    i2d_value_map * description_by_item_type;
    i2d_data_map * description_of_item_property;
    i2d_rbt * print_handlers;
    i2d_buffer_cache * buffer_cache;
    i2d_string_stack_cache * stack_cache;
};

typedef struct i2d_print i2d_print;

int i2d_print_init(i2d_print **, i2d_json *);
void i2d_print_deit(i2d_print **);
int i2d_print_format(i2d_print *, i2d_item *);
#endif
