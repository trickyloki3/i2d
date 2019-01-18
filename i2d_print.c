#include "i2d_print.h"

int i2d_print_init(i2d_print ** result, i2d_json * json) {
    int status = I2D_OK;
    i2d_print * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_value_map_init(&object->description_by_item_type, json->description_by_item_type, i2d_value_string_stack)) {
                status = i2d_panic("failed to load description_by_item_type ");
            } else if(i2d_data_map_init(&object->description_of_item_property, data_map_by_name, json->description_of_item_property, NULL)) {
                status = i2d_panic("failed to load description_of_item_property");
            }
            if(status) {
                i2d_print_deit(&object);
            } else {
                *result = object;
            }
        }
    }

    return status;
}

void i2d_print_deit(i2d_print ** result) {
    i2d_print * object;

    object = *result;
    i2d_deit(object->description_of_item_property, i2d_data_map_deit);
    i2d_deit(object->description_by_item_type, i2d_value_map_deit);
    i2d_free(object);
    *result = NULL;
}
