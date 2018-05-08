#include "i2d_json.h"

int i2d_json_create(json_t ** json, i2d_string * path) {
    int status = I2D_OK;
    json_error_t error;
    i2d_zero(error);

    *json = json_load_file(path->string, JSON_DISABLE_EOF_CHECK, &error);
    if(!(*json))
        status = i2d_panic("%s (line %d column %d)", error.text, error.line, error.column);

    return status;
}

void i2d_json_destroy(json_t * json) {
    if(json)
        json_decref(json);
}

int i2d_object_get_string(json_t * json, i2d_string * result) {
    int status = I2D_OK;
    const char * string;
    size_t length;

    string = json_string_value(json);
    if(!string) {
        status = i2d_panic("invalid string object");
    } else {
        length = json_string_length(json);
        if(!length) {
            status = i2d_panic("empty string object");
        } else {
            status = i2d_string_create(result, string, length);
        }
    }

    return status;
}

int i2d_object_get_number(json_t * json, long * result) {
    int status = I2D_OK;

    if(!json_is_number(json)) {
        status = i2d_panic("invalid number object");
    } else {
        *result = json_integer_value(json);
    }

    return status;
}

int i2d_object_get_range(json_t * left, json_t * right, i2d_range * result) {
    int status = I2D_OK;
    long min;
    long max;

    if( i2d_object_get_number(left, &min) ||
        i2d_object_get_number(right, &max) ) {
        status = i2d_panic("failed to get min or max");
    } else if(i2d_range_create_add(result, min, max)) {
        status = i2d_panic("failed to create range object");
    }

    return status;
}

int i2d_object_get_list(json_t * json, size_t element, void ** result_list, size_t * result_size) {
    int status = I2D_OK;
    void * list;
    size_t size;

    size = json_object_size(json);
    if(!size) {
        status = i2d_panic("empty object");
    } else {
        list = calloc(size, element);
        if(!list) {
            status = i2d_panic("out of memory");
        } else {
            *result_list = list;
            *result_size = size;
        }
    }

    return status;
}

int i2d_value_map_init(i2d_value_map ** result, json_t * json) {
    int status = I2D_OK;
    i2d_value_map * object;

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
            if(i2d_rbt_init(&object->map, i2d_rbt_cmp_long)) {
                status = i2d_panic("failed to create value map");
            } else if(i2d_object_get_list(json, sizeof(*object->list), (void **) &object->list, &object->size)) {
                status = i2d_panic("failed to create value array");
            } else {
                json_object_foreach(json, key, value) {
                    if(i2d_strtol(&object->list[i].value, key, strlen(key), 10)) {
                        status = i2d_panic("failed to convert value string");
                    } else if(i2d_object_get_string(value, &object->list[i].name)) {
                        status = i2d_panic("failed to copy value name");
                    } else if(i2d_rbt_insert(object->map, &object->list[i].value, &object->list[i])) {
                        status = i2d_panic("failed to map value object");
                    } else {
                        i++;
                    }
                }

            }

            if(status)
                i2d_value_map_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_value_map_deit(i2d_value_map ** result) {
    i2d_value_map * object;
    size_t i;

    object = *result;
    if(object->list) {
        for(i = 0; i < object->size; i++)
            i2d_free(object->list[i].name.string);
        i2d_free(object->list);
    }
    i2d_deit(object->map, i2d_rbt_deit);
    i2d_free(object);
    *result = NULL;
}
