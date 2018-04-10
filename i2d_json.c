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
