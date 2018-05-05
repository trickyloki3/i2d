#include "i2d_json.h"

int i2d_json_create(json_t * json, i2d_string * path) {
    int status = I2D_OK;
    json_error_t error;
    i2d_zero(error);

    json = json_load_file(path->string, JSON_DISABLE_EOF_CHECK, &error);
    if(!json)
        status = i2d_panic("%s (line %d column %d)", error.text, error.line, error.column);

    return status;
}

void i2d_json_destroy(json_t * json) {
    if(json)
        json_decref(json);
}
