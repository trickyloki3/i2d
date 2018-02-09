#include "i2d_util.h"

int i2d_panic_func(const char * format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    return I2D_FAIL;
}

int i2d_str_copy(i2d_str ** result, const char * string, size_t length) {
    int status = I2D_OK;
    i2d_str * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            object->length = length;
            object->string = malloc(object->length + 1);
            if(!object->string) {
                status = i2d_panic("out of memory");
            } else {
                memcpy(object->string, string, length);
                object->string[object->length] = 0;
            }

            if(status)
                i2d_str_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_str_deit(i2d_str ** result) {
    i2d_str * object;

    object = *result;
    i2d_free(object->string);
    i2d_free(object);
    *result = NULL;
}