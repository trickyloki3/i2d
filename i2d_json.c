#include "i2d_json.h"

static i2d_keywords_load(i2d_keywords *, json_t *);
static i2d_json_load(i2d_json *, i2d_str *);

int i2d_keywords_init(i2d_keywords ** result, json_t * json) {
    int status = I2D_OK;
    i2d_keywords * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_keywords_load(object, json))
                status = i2d_panic("failed to load keywords");

            if(status)
                i2d_keywords_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_keywords_deit(i2d_keywords ** result) {
    i2d_keywords * object;
    size_t i;

    object = *result;
    i2d_deit(object->index, i2d_rbt_deit);
    for(i = 0; i < object->size; i++)
        i2d_free(object->list[i].string);
    i2d_free(object->list);
    i2d_free(object);
    *result = NULL;
}

static i2d_keywords_load(i2d_keywords * keywords, json_t * json) {
    int status = I2D_OK;
    json_t * array;
    size_t i;
    json_t * value;

    array = json_object_get(json, "keywords");
    if(!array) {
        status = i2d_panic("missing keywords");
    } else {
        keywords->size = json_array_size(array);
        if(!keywords->size) {
            status = i2d_panic("empty keywords array");
        } else {
            keywords->list = calloc(keywords->size, sizeof(*keywords->list));
            if(!keywords->list) {
                status = i2d_panic("out of memory");
            } else {
                json_array_foreach(array, i, value) {
                    if(i2d_str_copy(&keywords->list[i], json_string_value(value), json_string_length(value))) {
                        status = i2d_panic("failed to create string object");
                        break;
                    }
                }

                if(!status) {
                    if(i2d_rbt_init(&keywords->index, i2d_rbt_cmp_str)) {
                        status = i2d_panic("failed to create red black tree object");
                    } else {
                        for(i = 0; i < keywords->size && !status; i++)
                            if(i2d_rbt_insert(keywords->index, &keywords->list[i], &keywords->list[i]))
                                status = i2d_panic("failed to insert string object");
                    }
                }
            }
        }
    }

    return status;
}

int i2d_json_init(i2d_json ** result, i2d_str * path) {
    int status = I2D_OK;
    i2d_json * object;

    if(i2d_is_invalid(result) || !path) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_json_load(object, path))
                status = i2d_panic("failed to load json file");

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
    i2d_deit(object->keywords, i2d_keywords_deit);
    i2d_free(object);
    *result = NULL;
}

static i2d_json_load(i2d_json * json, i2d_str * path) {
    int status = I2D_OK;
    json_t * object = NULL;

    json_error_t error;
    i2d_zero(error);

    object = json_load_file(path->string, JSON_DISABLE_EOF_CHECK, &error);
    if(!object) {
        status = i2d_panic("%s (line %d column %d)", error.text, error.line, error.column);
    } else {
        if(i2d_keywords_init(&json->keywords, object))
            status = i2d_panic("failed to load keyboards");
        json_decref(object);
    }

    return status;
}
