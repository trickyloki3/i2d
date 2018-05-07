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

int i2d_constant_db_init(i2d_constant_db ** result, json_t * json) {
    int status = I2D_OK;
    i2d_constant_db * object;
    json_t * consts;

    size_t i = 0;
    const char * key;
    json_t * value;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            consts = json_object_get(json, "consts");
            if(!consts) {
                status = i2d_panic("failed to get constants object");
            } else if(i2d_object_get_list(consts, sizeof(*object->constants), (void **) &object->constants, &object->size)) {
                status = i2d_panic("failed to create constants array");
            } else if(i2d_rbt_init(&object->macros, i2d_rbt_cmp_str)) {
                status = i2d_panic("failed to create macro map");
            } else {
                 json_object_foreach(consts, key, value) {
                    if(i2d_constant_create(&object->constants[i], key, value)) {
                        status = i2d_panic("failed to create constant object");
                    } else if(i2d_rbt_insert(object->macros, object->constants[i].macro.string, &object->constants[i])) {
                        status = i2d_panic("failed to map constant object");
                    } else {
                        i++;
                    }
                 }
            }

            if(status)
                i2d_constant_db_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_constant_db_deit(i2d_constant_db ** result) {
    i2d_constant_db * object;
    size_t i;

    object = *result;
    if(object->constants)
        for(i = 0; i < object->size; i++)
            i2d_constant_destroy(&object->constants[i]);
    i2d_deit(object->macros, i2d_rbt_deit);
    i2d_free(object->constants);
    i2d_free(object);
    *result = NULL;
}
