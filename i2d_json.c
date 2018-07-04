#include "i2d_json.h"

static int i2d_json_create_data(json_t **, const char *, const char *);

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

int i2d_object_get_string_stack(json_t * json, i2d_string_stack * result) {
    int status = I2D_OK;
    size_t size;

    size_t index;
    json_t * value;

    i2d_string string;
    i2d_zero(string);

    size = json_array_size(json);
    if(!size) {
        status = i2d_panic("empty array");
    } else if(i2d_string_stack_create(result, size)) {
        status = I2D_FAIL;
    } else {
        json_array_foreach(json, index, value) {
            if(i2d_object_get_string(value, &string)) {
                status = i2d_panic("failed to get string object");
            } else {
                if(i2d_string_stack_push(result, string.string, string.length))
                    status = i2d_panic("failed to push string stack");

                i2d_free(string.string);
            }
            if(status)
                break;
        }
        if(status)
            i2d_string_stack_destroy(result);
    }

    return status;
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

static int i2d_json_create_data(json_t ** json, const char * data_path, const char * file_name) {
    int status = I2D_OK;
    i2d_string path;

    if(i2d_string_vprintf(&path, "%s/%s", data_path, file_name)) {
        status = i2d_panic("failed to create path string");
    } else {
        status = i2d_json_create(json, &path);

        i2d_string_destroy(&path);
    }

    return status;
}

int i2d_json_init(i2d_json ** result, i2d_string * data_path) {
    int status = I2D_OK;
    i2d_json * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_json_create_data(&object->bonus, data_path->string, "bonus.json")) {
                status = i2d_panic("failed to load bonus.json");
            } else if(i2d_json_create_data(&object->bonus2, data_path->string, "bonus2.json")) {
                status = i2d_panic("failed to load bonus2.json");
            } else if(i2d_json_create_data(&object->bonus3, data_path->string, "bonus3.json")) {
                status = i2d_panic("failed to load bonus3.json");
            } else if(i2d_json_create_data(&object->bonus4, data_path->string, "bonus4.json")) {
                status = i2d_panic("failed to load bonus4.json");
            } else if(i2d_json_create_data(&object->bonus5, data_path->string, "bonus5.json")) {
                status = i2d_panic("failed to load bonus5.json");
            } else if(i2d_json_create_data(&object->statements, data_path->string, "statements.json")) {
                status = i2d_panic("failed to load statements.json");
            } else if(i2d_json_create_data(&object->getiteminfo, data_path->string, "getiteminfo.json")) {
                status = i2d_panic("failed to load getiteminfo.json");
            } else if(i2d_json_create_data(&object->strcharinfo, data_path->string, "strcharinfo.json")) {
                status = i2d_panic("failed to load strcharinfo.json");
            } else if(i2d_json_create_data(&object->ammos, data_path->string, "ammos.json")) {
                status = i2d_panic("failed to load ammos.json");
            } else if(i2d_json_create_data(&object->weapons, data_path->string, "weapons.json")) {
                status = i2d_panic("failed to load weapons.json");
            } else if(i2d_json_create_data(&object->functions, data_path->string, "functions.json")) {
                status = i2d_panic("failed to load functions.json");
            } else if(i2d_json_create_data(&object->constants, data_path->string, "constants.json")) {
                status = i2d_panic("failed to load constants.json");
            }

            if(status)
                i2d_json_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_json_deit(i2d_json ** result) {
    i2d_json * object;

    object = *result;
    i2d_json_destroy(object->constants);
    i2d_json_destroy(object->functions);
    i2d_json_destroy(object->weapons);
    i2d_json_destroy(object->ammos);
    i2d_json_destroy(object->strcharinfo);
    i2d_json_destroy(object->getiteminfo);
    i2d_json_destroy(object->statements);
    i2d_json_destroy(object->bonus5);
    i2d_json_destroy(object->bonus4);
    i2d_json_destroy(object->bonus3);
    i2d_json_destroy(object->bonus2);
    i2d_json_destroy(object->bonus);
    i2d_free(object);
    *result = NULL;
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

int i2d_value_map_get(i2d_value_map * value_map, long * key, i2d_string * result) {
    int status = I2D_OK;
    i2d_value * value;

    if(!i2d_rbt_search(value_map->map, key, (void **) &value))
        *result = value->name;

    return status;
}
