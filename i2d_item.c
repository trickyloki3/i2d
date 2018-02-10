#include "i2d_item.h"

#define READ_SIZE 65536

static int i2d_item_parse(i2d_item *, char *, size_t);
static void i2d_item_append(i2d_item *, i2d_item *);
static void i2d_item_remove(i2d_item *);
static int i2d_item_db_load(i2d_item_db *, i2d_str *);
static int i2d_item_db_read(int, size_t, i2d_buf *);
static int i2d_item_db_parse(i2d_item_db *, i2d_buf *);

int i2d_item_init(i2d_item ** result, char * string, size_t length) {
    int status = I2D_OK;
    i2d_item * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_item_parse(object, string, length)) {
                status = i2d_panic("failed to load item -- %s", string);
            } else {
                object->next = object;
                object->prev = object;
            }

            if(status)
                i2d_item_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_item_deit(i2d_item ** result) {
    i2d_item * object;

    object = *result;
    object->next = NULL;
    object->prev = NULL;
    i2d_deit(object->aegis_name, i2d_str_deit);
    i2d_deit(object->name, i2d_str_deit);
    i2d_deit(object->script, i2d_str_deit);
    i2d_deit(object->onequip_script, i2d_str_deit);
    i2d_deit(object->onunequip_script, i2d_str_deit);
    i2d_free(object);
    *result = NULL;
}

static int i2d_item_parse(i2d_item * item, char * string, size_t length) {
    int status = I2D_OK;

    /*
     * to-do: parse the fields
     */

    return status;
}

static void i2d_item_append(i2d_item * x, i2d_item * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

static void i2d_item_remove(i2d_item * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

int i2d_item_db_init(i2d_item_db ** result, i2d_str * path) {
    int status = I2D_OK;
    i2d_item_db * object;
    i2d_str * item = NULL;

    if(i2d_is_invalid(result) || !path) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_str_copy(&item, "0,head,node,,,,,,,,,,,,,,,,,{},{},{}", 36)) {
                status = i2d_panic("failed to create string object");
            } else {
                if(i2d_item_init(&object->item_list, item->string, item->length)) {
                    status = i2d_panic("failed to create item object");
                } else if(i2d_item_db_load(object, path)) {
                    status = i2d_panic("failed to load item db -- %s", path->string);
                }
                i2d_str_deit(&item);
            }

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
    i2d_item * item;

    object = *result;
    if(object->item_list) {
        while(object->item_list != object->item_list->next) {
            item = object->item_list->next;
            i2d_item_remove(item);
            i2d_item_deit(&item);
        }
        i2d_item_deit(&object->item_list);
    }
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
        if(i2d_buf_init(&buffer, READ_SIZE + 4096)) {
            status = i2d_panic("failed to create buffer object");
        } else {
            result = i2d_item_db_read(fd, READ_SIZE, buffer);
            while(0 < result && !status) {
                if(i2d_item_db_parse(item_db, buffer))
                    status = i2d_panic("failed to parse buffer");
                result = i2d_item_db_read(fd, READ_SIZE, buffer);
            }
            if(!status && buffer->offset && i2d_item_db_parse(item_db, buffer))
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
    size_t length;
    size_t consume;
    i2d_item * item;

    anchor = buffer->buffer;
    delimit = strchr(anchor, '\n');
    while(delimit && !status) {
        *delimit = 0;

        /*
         * skip initial whitespace
         */
        while(isspace(*anchor))
            anchor++;

        /*
         * skip empty lines
         */
        if(delimit > anchor) {
            length = (size_t) delimit - (size_t) anchor;

            /*
             * each item must have at least
             * 21 commas and 6 curly braces
             * including the newline, hence
             * the minimum length is 28
             */
            if(28 < length && isdigit(*anchor)) {
                item = NULL;

                if(i2d_item_init(&item, anchor, length)) {
                    status = i2d_panic("failed to create item object");
                } else {
                    i2d_item_append(item, item_db->item_list);
                    item_db->item_count++;
                }
            }
        }

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
