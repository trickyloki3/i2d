#include "i2d_json.h"

static int i2d_json_load_file(json_t **, i2d_string *, const char *);
static int i2d_json_load_bonus(i2d_json *, i2d_string *);
static int i2d_json_load_data(i2d_json *, i2d_string *);

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

int i2d_object_get_number_array(json_t * json, long ** result, size_t * result_size) {
    int status = I2D_OK;
    size_t size;
    long * list = NULL;

    size_t index;
    json_t * value;

    size = json_array_size(json);
    if(!size) {
        status = i2d_panic("empty array");
    } else {
        list = calloc(size, sizeof(*list));
        if(!list) {
            status = i2d_panic("out of memory");
        } else {
            json_array_foreach(json, index, value) {
                if(i2d_object_get_number(value, &list[index]))
                    status = i2d_panic("failed to get number object");

                if(status)
                    break;
            }

            if(status) {
                free(list);
            } else {
                *result = list;
                *result_size = size;
            }
        }
    }

    return status;
}

int i2d_object_get_number(json_t * json, long * result) {
    int status = I2D_OK;
    json_int_t value;

    if(!json_is_number(json)) {
        status = i2d_panic("invalid number object");
    } else {
        value = json_integer_value(json);
        if(value < LONG_MIN || value > LONG_MAX) {
            status = i2d_panic("integer underflow or overflow");
        } else {
            *result = (long) value;
        }
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

static int i2d_json_load_file(json_t ** json, i2d_string * directory, const char * file) {
    int status = I2D_OK;
    i2d_string path;
    i2d_zero(path);

    if(i2d_string_vprintf(&path, "%s/%s", directory->string, file)) {
        status = i2d_panic("failed to create path string");
    } else {
        if(i2d_json_create(json, &path))
            status = i2d_panic("failed to load %s", path.string);

        i2d_string_destroy(&path);
    }

    return status;
}

static int i2d_json_load_bonus(i2d_json * json, i2d_string * directory) {
    int status = I2D_OK;
    i2d_string path;
    i2d_zero(path);

    if(i2d_string_vprintf(&path, "%s/bonus.json", directory->string)) {
        status = i2d_panic("failed to create string object");
    } else {
        if(i2d_json_create(&json->bonus_file, &path)) {
            status = i2d_panic("failed to load %s", path.string);
        } else {
            json->bonus = json_object_get(json->bonus_file, "bonus");
            json->bonus2 = json_object_get(json->bonus_file, "bonus2");
            json->bonus3 = json_object_get(json->bonus_file, "bonus3");
            json->bonus4 = json_object_get(json->bonus_file, "bonus4");
            json->bonus5 = json_object_get(json->bonus_file, "bonus5");
        }
        i2d_string_destroy(&path);
    }

    return status;
}

static int i2d_json_load_data(i2d_json * json, i2d_string * directory) {
    int status = I2D_OK;
    i2d_string path;
    i2d_zero(path);

    if(i2d_string_vprintf(&path, "%s/data.json", directory->string)) {
        status = i2d_panic("failed to create string object");
    } else {
        if(i2d_json_create(&json->data_file, &path)) {
            status = i2d_panic("failed to load %s", path.string);
        } else {
            json->ammo_type = json_object_get(json->data_file, "ammo_type");
            json->bonus_script_flag = json_object_get(json->data_file, "bonus_script_flag");
            json->getiteminfo_type = json_object_get(json->data_file, "getiteminfo_type");
            json->searchstore_effect = json_object_get(json->data_file, "searchstore_effect");
            json->skill_flag = json_object_get(json->data_file, "skill_flag");
            json->strcharinfo_type = json_object_get(json->data_file, "strcharinfo_type");
            json->weapon_type = json_object_get(json->data_file, "weapon_type");
        }
        i2d_string_destroy(&path);
    }

    return status;
}

int i2d_json_init(i2d_json ** result, i2d_string * directory) {
    int status = I2D_OK;
    i2d_json * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if( i2d_json_load_file(&object->statements, directory, "statements.json") ||
                i2d_json_load_file(&object->functions, directory, "functions.json") ||
                i2d_json_load_file(&object->constants, directory, "constants.json") ||
                i2d_json_load_file(&object->arguments, directory, "arguments.json") ||
                i2d_json_load_file(&object->prefixes, directory, "prefixes.json") ||
                i2d_json_load_bonus(object, directory) ||
                i2d_json_load_data(object, directory) )
                status = i2d_panic("failed to load json object");

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
    i2d_json_destroy(object->data_file);
    i2d_json_destroy(object->bonus_file);
    i2d_json_destroy(object->prefixes);
    i2d_json_destroy(object->arguments);
    i2d_json_destroy(object->constants);
    i2d_json_destroy(object->functions);
    i2d_json_destroy(object->statements);
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

int i2d_value_map_get(i2d_value_map * value_map, long key, i2d_string * result) {
    int status = I2D_OK;
    i2d_value * value;

    if(!i2d_rbt_search(value_map->map, &key, (void **) &value))
        *result = value->name;

    return status;
}
