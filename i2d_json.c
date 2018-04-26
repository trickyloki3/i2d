#include "i2d_json.h"

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
            if(!object->object)
                status = i2d_panic("%s (line %d column %d)", error.text, error.line, error.column);

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

int i2d_json_block_map(i2d_json * json, const char * key, json_t ** result) {
    int status = I2D_OK;
    json_t * blocks;
    json_t * statement;

    blocks = json_object_get(json->object, "blocks");
    if(!blocks) {
        status = i2d_panic("failed to get blocks key value");
    } else {
        statement = json_object_get(blocks, key);
        if(!statement) {
            status = i2d_panic("failed to get %s key value", key);
        } else {
            *result = statement;
        }
    }

    return status;
}

int i2d_json_get_str(json_t * json, const char * key, i2d_str_const * result) {
    int status = I2D_OK;
    json_t * object;
    const char * string;
    size_t length;

    object = json_object_get(json, key);
    if(!object) {
        status = i2d_panic("failed to get %s key value", key);
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
