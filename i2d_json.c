#include "i2d_json.h"

static int i2d_json_load(i2d_json *, i2d_str *);

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
    i2d_free(object);
    *result = NULL;
}

static int i2d_json_load(i2d_json * json, i2d_str * path) {
    int status = I2D_OK;
    json_t * object = NULL;

    json_error_t error;
    i2d_zero(error);

    object = json_load_file(path->string, JSON_DISABLE_EOF_CHECK, &error);
    if(!object) {
        status = i2d_panic("%s (line %d column %d)", error.text, error.line, error.column);
    } else {

        json_decref(object);
    }

    return status;
}
