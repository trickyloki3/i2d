#include "i2d_item.h"

#define READ_SIZE 65536

static int i2d_item_db_load(i2d_item_db *, i2d_str *);
static int i2d_item_db_read(int, size_t, i2d_buf *);
static int i2d_item_db_parse(i2d_item_db *, i2d_buf *);

int i2d_item_db_init(i2d_item_db ** result, i2d_str * path) {
    int status = I2D_OK;
    i2d_item_db * object;

    if(i2d_is_invalid(result) || !path) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_item_db_load(object, path))
                status = i2d_panic("failed to load item db -- %s", path->string);
            if(status)
                i2d_item_db_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_item_db_deit(i2d_item_db ** result) {
    i2d_item_db * object;

    object = *result;
    i2d_free(object);
    *result = NULL;
}

static int i2d_item_db_load(i2d_item_db * item_db, i2d_str * path) {
    int status = I2D_OK;

    int fd;
    i2d_buf * buffer = NULL;
    int result;

    fd = open(path->string, O_RDONLY);
    if(0 > fd) {
        status = i2d_panic("failed to open item db -- %s", path->string);
    } else {
        if(i2d_buf_init(&buffer, READ_SIZE)) {
            status = i2d_panic("failed to create buffer object");
        } else {
            result = i2d_item_db_read(fd, READ_SIZE, buffer);
            while(0 < result && !status) {
                if(i2d_item_db_parse(item_db, buffer))
                    status = i2d_panic("failed to parse buffer");
                result = i2d_item_db_read(fd, READ_SIZE, buffer);
            }
            if(buffer->offset)
                if(i2d_item_db_parse(item_db, buffer))
                        status = i2d_panic("failed to parse buffer");
            i2d_buf_deit(&buffer);
        }
        close(fd);
    }

    return status;
}

static int i2d_item_db_read(int fd, size_t size, i2d_buf * buffer) {
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

static int i2d_item_db_parse(i2d_item_db * item_db, i2d_buf * buffer) {
    int status = I2D_OK;
    char * anchor;
    char * delimit;
    size_t consume;

    anchor = buffer->buffer;
    delimit = strchr(anchor, '\n');
    while(delimit && !status) {
        *delimit = 0;

        /*
         * to-do:
         * parse line for item
         */

        anchor = delimit + 1;
        delimit = strchr(anchor, '\n');
    }

    consume = (size_t) anchor - (size_t) buffer->buffer;
    if(0 == consume) {
        status = i2d_panic("line overflow");
    } else if(buffer->offset < consume) {
        status = i2d_panic("buffer overflow");
    } else {
        if(buffer->offset > consume)
            memcpy(buffer->buffer, buffer->buffer + consume, buffer->offset - consume);

        buffer->offset -= consume;
        buffer->buffer[buffer->offset]= 0;
    }

    return status;
}
