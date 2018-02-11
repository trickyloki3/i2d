#include "i2d_util.h"

int i2d_panic_func(const char * format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    return I2D_FAIL;
}

int i2d_strtol(long * result, const char * string, size_t length, int base) {
    int status = I2D_OK;

    long number;
    char * end = NULL;

    if(!length) {
        *result = 0;
    } else {
        number = strtol(string, &end, base);
        if(string + length != end) {
            status = i2d_panic("invalid string '%s' in '%s'", end, string);
        } else {
            *result = number;
        }
    }


    return status;
}

int i2d_strtoul(unsigned long * result, const char * string, size_t length, int base) {
    int status = I2D_OK;

    unsigned long number;
    char * end = NULL;

    if(!length) {
        *result = 0;
    } else {
        number = strtoul(string, &end, base);
        if(string + length != end) {
            status = i2d_panic("invalid string '%s' in '%s'", end, string);
        } else {
            *result = number;
        }
    }

    return status;
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
                if(length)
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

int i2d_fd_read(int fd, size_t size, i2d_buf * buffer) {
    int status = I2D_OK;

    fd_set set;
    struct timeval timeout;
    ssize_t result;

    FD_ZERO(&set);
    FD_SET(fd, &set);

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    if(0 >= select(fd + 1, &set, NULL, NULL, &timeout) || !FD_ISSET(fd, &set)) {
        status = i2d_panic("failed on select error or timeout");
    } else if(i2d_buf_fit(buffer, size + 1)) {
        status = I2D_FAIL;
    } else {
        result = read(fd, buffer->buffer + buffer->offset, size);
        if(0 > result) {
            status = i2d_panic("failed on read");
        } else {
            buffer->offset += result;
            buffer->buffer[buffer->offset] = 0;
        }
    }

    return status ? -1 : result;
}
