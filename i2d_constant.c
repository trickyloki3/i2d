#include "i2d_constant.h"

int i2d_constant_create(i2d_constant * result, const char * key, json_t * json) {
    int status = I2D_OK;
    json_t * name;
    json_t * min;
    json_t * max;
    json_t * value;

    if(i2d_string_create(&result->macro, key, strlen(key))) {
        status = i2d_panic("failed to copy macro string");
    } else {
        name = json_object_get(json, "name");
        min = json_object_get(json, "min");
        max = json_object_get(json, "max");
        value = json_object_get(json, "value");

        if(name && i2d_object_get_string(name, &result->name)) {
            status = i2d_panic("failed to copy name string");
        } else {
            if(min && max && i2d_object_get_range(min, max, &result->range)) {
                status = i2d_panic("failed to create range");
            } else {
                if(!value || i2d_object_get_number(value, &result->value))
                    status = i2d_panic("failed to get value number");
                if(status)
                    i2d_range_destroy(&result->range);
            }
            if(status)
                i2d_string_destroy(&result->name);
        }
        if(status)
            i2d_string_destroy(&result->macro);
    }

    return status;
}

void i2d_constant_destroy(i2d_constant * result) {
    i2d_range_destroy(&result->range);
    i2d_string_destroy(&result->name);
    i2d_string_destroy(&result->macro);
}
