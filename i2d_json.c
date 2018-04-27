#include "i2d_json.h"

int i2d_object_init(i2d_object ** result, json_t * json, const char * object_key, i2d_object_create create, i2d_object_delete delete, i2d_rbt_cmp cmp, void * context) {
    int status = I2D_OK;
    i2d_object * object;

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
            if(i2d_json_get_object(json, object_key, &json)) {
                status = i2d_panic("failed to get object");
            } else if(i2d_rbt_init(&object->map, cmp)) {
                status = i2d_panic("failed to create red black tree object");
            } else {
                if(i2d_json_object_list(json, &object->list, &object->size)) {
                    status = i2d_panic("failed to create object list");
                } else {
                    object->create = create;
                    object->delete = delete;
                    json_object_foreach(json, key, value) {
                        if(object->create(&object->list[i], key, value, object->map, context)) {
                            status = i2d_panic("failed to create object");
                        } else {
                            i++;
                        }
                    }
                }
            }
            if(status)
                i2d_object_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_object_deit(i2d_object ** result) {
    i2d_object * object;
    size_t i;

    object = *result;
    if(object->list && object->delete) {
        for(i = 0; i < object->size; i++)
            object->delete(&object->list[i]);
        i2d_free(object->list);
    }
    i2d_deit(object->map, i2d_rbt_deit);
    i2d_free(object);
    *result = NULL;
}

int i2d_object_map(i2d_object * object, void * key, void ** value) {
    return i2d_rbt_search(object->map, key, value);
}

int i2d_json_init(i2d_json ** result, i2d_str * path) {
    int status = I2D_OK;
    i2d_json * object;

    json_error_t error;
    i2d_zero(error);

    if(i2d_is_invalid(result) || !path) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            object->object = json_load_file(path->string, JSON_DISABLE_EOF_CHECK, &error);
            if(!object->object) {
                status = i2d_panic("%s (line %d column %d)", error.text, error.line, error.column);
            } else if(i2d_json_get_object(object->object, "blocks", &object->blocks)) {
                status = i2d_panic("failed to get object");
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
    if(object->object)
        json_decref(object->object);
    i2d_free(object);
    *result = NULL;
}

int i2d_json_object_list(json_t * json, void *** _list, size_t * _size) {
    int status = I2D_OK;
    void ** list;
    size_t size;

    size = json_object_size(json);
    if(!size) {
        status = i2d_panic("empty object");
    } else {
        list = calloc(size, sizeof(*list));
        if(!list) {
            status = i2d_panic("out of memory");
        } else {
            *_list = list;
            *_size = size;
        }
    }

    return status;
}

int i2d_json_get_object(json_t * json, const char * key, json_t ** result) {
    int status = I2D_OK;
    json_t * object;

    object = json_object_get(json, key);
    if(!object) {
        status = i2d_panic("failed to get %s key value", key);
    } else {
         *result = object;
    }

    return status;
}

int i2d_json_get_str(json_t * json, const char * key, i2d_str_const * result) {
    int status = I2D_OK;
    json_t * object;
    const char * string;
    size_t length;

    if(i2d_json_get_object(json, key, &object)) {
        status = i2d_panic("failed to get object");
    } else {
        string = json_string_value(object);
        if(!string) {
            status = i2d_panic("invalid string on %s key value", key);
        } else {
            length = json_string_length(object);
            if(!length) {
                status = i2d_panic("empty string on %s key value", key);
            } else {
                result->string = string;
                result->length = length;
            }
        }
    }

    return status;
}

int i2d_json_get_int(json_t * json, const char * key, json_int_t * result) {
    int status = I2D_OK;
    json_t * object;

    if(i2d_json_get_object(json, key, &object)) {
        status = i2d_panic("failed to get object");
    } else {
        *result = json_integer_value(object);
    }

    return status;
}

int i2d_str_list_init(i2d_str_list ** result, const char * key, json_t * json) {
    int status = I2D_OK;
    i2d_str_list * object;
    json_t * array;
    size_t index;
    json_t *value;
    const char * string;
    size_t length;

    if(i2d_is_invalid(result) || !key || !json) {
        status = i2d_panic("invalid paramaters");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            array = json_object_get(json, key);
            if(!array) {
                status = i2d_panic("failed to get %s key value", key);
            } else {
                object->size = json_array_size(array);
                if(!object->size) {
                    status = i2d_panic("empty array on %s key value", key);
                } else {
                    object->list = calloc(object->size, sizeof(*object->list));
                    if(!object->list) {
                        status = i2d_panic("out of memory");
                    } else {
                        json_array_foreach(array, index, value) {
                            string = json_string_value(value);
                            if(!string) {
                                status = i2d_panic("failed on invalid string");
                            } else {
                                length = json_string_length(value);
                                if(!length) {
                                    status = i2d_panic("failed on empty string");
                                } else if(i2d_str_init(&object->list[index], string, length)) {
                                    status = i2d_panic("failed to create string object");
                                }
                            }
                        }
                    }
                }
            }

            if(status)
                i2d_str_list_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_str_list_deit(i2d_str_list ** result) {
    i2d_str_list * object;
    size_t i;

    object = *result;
    if(object->list) {
        for(i = 0; i < object->size; i++)
            i2d_deit(object->list[i], i2d_str_deit);
        i2d_free(object->list);
    }
    i2d_free(object);
    *result = NULL;
}

#if i2d_debug
int i2d_json_test() {
    int status = I2D_OK;
    i2d_str * path = NULL;
    i2d_json * json = NULL;

    if(i2d_str_init(&path, "i2d.json", 8)) {
        status = i2d_panic("failed to create string object");
    } else {
        if(i2d_json_init(&json, path)) {
            status = i2d_panic("failed to create json object");
        } else {

            i2d_json_deit(&json);
        }
        i2d_str_deit(&path);
    }

    return status;
}
#endif
