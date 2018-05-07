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
