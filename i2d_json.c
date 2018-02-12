#include "i2d_json.h"

#include "jansson.h"

static i2d_json_load(i2d_json *, i2d_str *);
static i2d_json_load_keywords(i2d_json *, json_t *);

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
    size_t i;

    object = *result;
    for(i = 0; i < object->keywords.size; i++)
        i2d_free(object->keywords.list[i].string);
    i2d_free(object->keywords.list);
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
        if(i2d_json_load_keywords(json, object))
            status = i2d_panic("failed to load keyboards");
        json_decref(object);
    }

    return status;
}

static i2d_json_load_keywords(i2d_json * json, json_t * object) {
    int status = I2D_OK;
    json_t * array;
    size_t i;
    json_t * value;

    array = json_object_get(object, "keywords");
    if(!array) {
        status = i2d_panic("missing 'keywords'");
    } else {
        json->keywords.size = json_array_size(array);
        if(!json->keywords.size) {
            status = i2d_panic("empty 'keywords' array");
        } else {
            json->keywords.list = calloc(json->keywords.size, sizeof(*json->keywords.list));
            if(!json->keywords.list) {
                status = i2d_panic("out of memory");
            } else {
                json_array_foreach(array, i, value) {
                    status = i2d_str_copy(&json->keywords.list[i], json_string_value(value), json_string_length(value));
                    if(status)
                        break;
                }
            }
        }
    }

    return status;
}
