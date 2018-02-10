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

int i2d_buf_init(i2d_buf ** result, size_t length) {
    int status = I2D_OK;
    i2d_buf * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            object->length = length;
            object->buffer = malloc(object->length);
            if(!object->buffer)
                status = i2d_panic("out of memory");

            if(status)
                i2d_buf_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_buf_deit(i2d_buf ** result) {
    i2d_buf * object;

    object = *result;
    i2d_free(object->buffer);
    i2d_free(object);
    *result = NULL;
}

int i2d_buf_fit(i2d_buf * buffer, size_t length) {
    int status = I2D_OK;
    uint8_t * binary;

    if(buffer->length - buffer->offset < length) {
        length += buffer->offset;
        binary = realloc(buffer->buffer, length);
        if(!binary) {
            status = i2d_panic("out of memory");
        } else {
            buffer->buffer = binary;
            buffer->length = length;
        }
    }

    return status;
}

int i2d_buf_format(i2d_buf * buffer, const char * format, ...) {
    int status = I2D_OK;
    int length;
    int result;
    va_list args;
    va_list copy;

    va_start(args, format);
    va_copy(copy, args);

    length = vsnprintf(NULL, 0, format, args);
    if(0 > length) {
        status = i2d_panic("format message is invalid");
    } else if(0 == length) {
        status = i2d_panic("format message is empty");
    } else if(i2d_buf_fit(buffer, length + 1)) {
        status = I2D_FAIL;
    } else {
        result = vsnprintf(buffer->buffer + buffer->offset, length + 1, format, copy);
        if(result != length)
            status = i2d_panic("format message is truncated");
        else
            buffer->offset += result;
    }

    va_end(copy);
    va_end(args);
    return status;
}

int i2d_buf_binary(i2d_buf * buffer, void * binary, size_t length) {
    int status = I2D_OK;

    if(i2d_buf_fit(buffer, length)) {
        status = I2D_FAIL;
    } else {
        memcpy(buffer->buffer + buffer->offset, binary, length);
    }

    return status;
}

void i2d_buf_dump(i2d_buf * buffer, const char * tag) {
    size_t i;

    fprintf(stderr, "tag: %s\n", tag ? tag : "unspecified");
    for(i = 0; i < buffer->length; i++) {
        if(((i + 1) % 8) == 0)
            fprintf(stderr, "%3" PRIu8 "\n", buffer->buffer[i]);
        else
            fprintf(stderr, "%3" PRIu8 " ", buffer->buffer[i]);
    }
    fprintf(stderr, "\n");
}
