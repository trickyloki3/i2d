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

    size_t i;
    int quote_depth = 0;
    int brace_depth = 0;

    char * anchor;
    size_t extent;

    int field = 0;

    anchor = string;
    for(i = 0; i < length && !status; i++) {
        if('"' == string[i]) {
            quote_depth = !quote_depth;
        } else if('{' == string[i]) {
            brace_depth++;
        } else if('}' == string[i]) {
            brace_depth--;
        } else if(',' == string[i] && !quote_depth && !brace_depth) {
            string[i] = 0;

            if((string + i) < anchor) {
                status = i2d_panic("line overflow");
            } else {
                extent = (size_t) (string + i) - (size_t) anchor;
                switch(field) {
                    case 0: status = i2d_strtol(&item->id, anchor, extent, 10); break;
                    case 1: status = i2d_str_copy(&item->aegis_name, anchor, extent); break;
                    case 2: status = i2d_str_copy(&item->name, anchor, extent); break;
                    case 3: status = i2d_strtol(&item->type, anchor, extent, 10); break;
                    case 4: status = i2d_strtol(&item->buy, anchor, extent, 10); break;
                    case 5: status = i2d_strtol(&item->sell, anchor, extent, 10); break;
                    case 6: status = i2d_strtol(&item->weight, anchor, extent, 10); break;
                    case 7: status = i2d_strtol(&item->atk, anchor, extent, 10); break;
                    case 8: status = i2d_strtol(&item->def, anchor, extent, 10); break;
                    case 9: status = i2d_strtol(&item->range, anchor, extent, 10); break;
                    case 10: status = i2d_strtol(&item->slots, anchor, extent, 10); break;
                    case 11: status = i2d_strtoul(&item->job, anchor, extent, 16); break;
                    case 12: status = i2d_strtoul(&item->upper, anchor, extent, 10); break;
                    case 13: status = i2d_strtol(&item->gender, anchor, extent, 10); break;
                    case 14: status = i2d_strtoul(&item->location, anchor, extent, 10); break;
                    case 15: status = i2d_strtol(&item->weapon_level, anchor, extent, 10); break;
                    case 16: status = i2d_strtol(&item->base_level, anchor, extent, 10); break;
                    case 17: status = i2d_strtol(&item->refineable, anchor, extent, 10); break;
                    case 18: status = i2d_strtol(&item->view, anchor, extent, 10); break;
                    case 19: status = i2d_str_copy(&item->script, anchor, extent); break;
                    case 20: status = i2d_str_copy(&item->onequip_script, anchor, extent); break;
                    default: status = i2d_panic("item has too many columns"); break;
                }
                field++;
            }

            anchor = (string + i + 1);
        }
    }

    if(!status) {
        if(21 != field) {
            status = i2d_panic("item is missing columns");
        } else if(&string[i] < anchor) {
            status = i2d_panic("line overflow");
        } else {
            extent = (size_t) &string[i] - (size_t) anchor;
            status = i2d_str_copy(&item->onunequip_script, anchor, extent);
        }
    }

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
                if(i2d_item_init(&object->list, item->string, item->length)) {
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
    if(object->list) {
        while(object->list != object->list->next) {
            item = object->list->next;
            i2d_item_remove(item);
            i2d_item_deit(&item);
        }
        i2d_item_deit(&object->list);
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
                    i2d_item_append(item, item_db->list);
                    item_db->size++;
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
