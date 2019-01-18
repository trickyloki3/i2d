#ifndef i2d_print_h
#define i2d_print_h

#include "i2d_data.h"
#include "i2d_json.h"

struct i2d_print {
    i2d_value_map * description_by_item_type;
    i2d_data_map * description_of_item_property;
};

typedef struct i2d_print i2d_print;

int i2d_print_init(i2d_print **, i2d_json *);
void i2d_print_deit(i2d_print **);
#endif
