#include "i2d_item.h"

#define READ_SIZE 65536

static int i2d_item_parse_optional(long *, long *, char *, size_t);
static int i2d_item_parse(i2d_item *, char *, size_t);
static int i2d_item_db_load(i2d_item_db *, i2d_str *);
static int i2d_item_db_parse(char *, size_t, void *);
static int i2d_item_db_index(i2d_item_db *);

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
    i2d_deit(object->aegis_name, i2d_str_deit);
    i2d_deit(object->name, i2d_str_deit);
    i2d_deit(object->script, i2d_str_deit);
    i2d_deit(object->onequip_script, i2d_str_deit);
    i2d_deit(object->onunequip_script, i2d_str_deit);
    i2d_free(object);
    *result = NULL;
}

static int i2d_item_parse_optional(long * left, long * right, char * string, size_t length) {
    int status = I2D_OK;
    char * anchor;

    if(!length) {
        *left = 0;
        *right = 0;
    } else {
        anchor = strchr(string, ':');
        if(anchor) {
            *anchor = 0, anchor++;

            status = i2d_strtol(left, string, strlen(string), 10) ||
                     i2d_strtol(right, anchor, strlen(anchor), 10);
        } else {
            status = i2d_strtol(left, string, length, 10);
        }
    }

    return status;
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
                    case 1: status = i2d_str_init(&item->aegis_name, anchor, extent); break;
                    case 2: status = i2d_str_init(&item->name, anchor, extent); break;
                    case 3: status = i2d_strtol(&item->type, anchor, extent, 10); break;
                    case 4: status = i2d_strtol(&item->buy, anchor, extent, 10); break;
                    case 5: status = i2d_strtol(&item->sell, anchor, extent, 10); break;
                    case 6: status = i2d_strtol(&item->weight, anchor, extent, 10); break;
                    case 7: status = i2d_item_parse_optional(&item->atk, &item->matk, anchor, extent); break;
                    case 8: status = i2d_strtol(&item->def, anchor, extent, 10); break;
                    case 9: status = i2d_strtol(&item->range, anchor, extent, 10); break;
                    case 10: status = i2d_strtol(&item->slots, anchor, extent, 10); break;
                    case 11: status = i2d_strtoul(&item->job, anchor, extent, 16); break;
                    case 12: status = i2d_strtoul(&item->upper, anchor, extent, 10); break;
                    case 13: status = i2d_strtol(&item->gender, anchor, extent, 10); break;
                    case 14: status = i2d_strtoul(&item->location, anchor, extent, 10); break;
                    case 15: status = i2d_strtol(&item->weapon_level, anchor, extent, 10); break;
                    case 16: status = i2d_item_parse_optional(&item->base_level, &item->max_level, anchor, extent); break;
                    case 17: status = i2d_strtol(&item->refineable, anchor, extent, 10); break;
                    case 18: status = i2d_strtol(&item->view, anchor, extent, 10); break;
                    case 19: status = i2d_str_init(&item->script, anchor, extent); break;
                    case 20: status = i2d_str_init(&item->onequip_script, anchor, extent); break;
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
            status = i2d_str_init(&item->onunequip_script, anchor, extent);
        }
    }

    return status;
}

void i2d_item_append(i2d_item * x, i2d_item * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_item_remove(i2d_item * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

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
            if(i2d_item_db_load(object, path)) {
                status = i2d_panic("failed to load item db");
            } else if(i2d_item_db_index(object)) {
                status = i2d_panic("failed to index item db");
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
    i2d_deit(object->index_by_name, i2d_rbt_deit);
    i2d_deit(object->index_by_aegis, i2d_rbt_deit);
    i2d_deit(object->index_by_id, i2d_rbt_deit);
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
            result = i2d_fd_read(fd, READ_SIZE, buffer);
            while(0 < result && !status) {
                if(i2d_by_line(buffer, i2d_item_db_parse, item_db))
                    status = i2d_panic("failed to parse buffer");
                result = i2d_fd_read(fd, READ_SIZE, buffer);
            }
            if(!status && buffer->offset && i2d_by_line(buffer, i2d_item_db_parse, item_db))
                status = i2d_panic("failed to parse buffer");
            i2d_buf_deit(&buffer);
        }
        close(fd);
    }

    return status;
}

static int i2d_item_db_parse(char * string, size_t length, void * data) {
    int status = I2D_OK;
    i2d_item_db * item_db = data;
    i2d_item * item = NULL;

    if(i2d_item_init(&item, string, length)) {
        status = i2d_panic("failed to create item object");
    } else {
        if(!item_db->list) {
            item_db->list = item;
        } else {
            i2d_item_append(item, item_db->list);
        }

        item_db->size++;
    }

    return status;
}

static int i2d_item_db_index(i2d_item_db * item_db) {
    int status = I2D_OK;
    i2d_item * item = NULL;

    if( i2d_rbt_init(&item_db->index_by_id, i2d_rbt_cmp_long) ||
        i2d_rbt_init(&item_db->index_by_aegis, i2d_rbt_cmp_str) ||
        i2d_rbt_init(&item_db->index_by_name, i2d_rbt_cmp_str) ) {
        status = i2d_panic("failed to create red black tree objects");
    } else {
        item = item_db->list;
        do {
            if(i2d_rbt_insert(item_db->index_by_id, &item->id, item)) {
                status = i2d_panic("failed to index item by id -- %ld", item->id);
            } else if(i2d_rbt_insert(item_db->index_by_aegis, item->aegis_name, item)) {
                status = i2d_panic("failed to index item by aegis name -- %s", item->aegis_name->string);
            } else if(i2d_rbt_insert(item_db->index_by_name, item->name, item)) {
                status = i2d_panic("failed to index item by name -- %s", item->name->string);
            }
            item = item->next;
        } while(item != item_db->list && !status);
    }

    return status;
}

int i2d_item_db_search_by_id(i2d_item_db * item_db, long item_id, i2d_item ** item) {
    return i2d_rbt_search(item_db->index_by_id, &item_id, (void **) item);
}

int i2d_item_db_search_by_aegis(i2d_item_db * item_db, i2d_str * aegis, i2d_item ** item) {
    return i2d_rbt_search(item_db->index_by_aegis, aegis, (void **) item);
}

int i2d_item_db_search_by_name(i2d_item_db * item_db, i2d_str * name, i2d_item ** item) {
    return i2d_rbt_search(item_db->index_by_name, name, (void **) item);
}
