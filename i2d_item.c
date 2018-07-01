#include "i2d_item.h"

static int i2d_item_parse_optional(long *, long *, char *, size_t);
static int i2d_item_parse(i2d_item *, char *, size_t);
static int i2d_item_db_load(i2d_item_db *, i2d_string *);
static int i2d_item_db_parse(char *, size_t, void *);
static int i2d_item_db_index(i2d_item_db *);

static int i2d_item_combo_parse_list(i2d_item_combo *, char *, size_t);
static int i2d_item_combo_parse(i2d_item_combo *, char *, size_t);
static int i2d_item_combo_db_load(i2d_item_combo_db *, i2d_string *);
static int i2d_item_combo_db_parse(char *, size_t, void *);

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
    i2d_free(object->aegis_name.string);
    i2d_free(object->name.string);
    i2d_free(object->script.string);
    i2d_free(object->onequip_script.string);
    i2d_free(object->onunequip_script.string);
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
                    case 1: status = i2d_string_create(&item->aegis_name, anchor, extent); break;
                    case 2: status = i2d_string_create(&item->name, anchor, extent); break;
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
                    case 19: status = i2d_string_create(&item->script, anchor, extent); break;
                    case 20: status = i2d_string_create(&item->onequip_script, anchor, extent); break;
                    default: status = i2d_panic("row has too many columns"); break;
                }
                field++;
            }

            anchor = (string + i + 1);
        }
    }

    if(!status) {
        if(21 != field) {
            status = i2d_panic("row is missing columns");
        } else if(&string[i] < anchor) {
            status = i2d_panic("line overflow");
        } else {
            extent = (size_t) &string[i] - (size_t) anchor;
            status = i2d_string_create(&item->onunequip_script, anchor, extent);
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

int i2d_item_db_init(i2d_item_db ** result, i2d_string * path) {
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

static int i2d_item_db_load(i2d_item_db * item_db, i2d_string * path) {
    int status = I2D_OK;

    int fd;
    i2d_buffer buffer;
    int result;

    fd = open(path->string, O_RDONLY);
    if(0 > fd) {
        status = i2d_panic("failed to open item db -- %s", path->string);
    } else {
        if(i2d_buffer_create(&buffer, I2D_SIZE * 2)) {
            status = i2d_panic("failed to create buffer object");
        } else {
            result = i2d_fd_read(fd, I2D_SIZE, &buffer);
            while(0 < result && !status) {
                if(i2d_by_line(&buffer, i2d_item_db_parse, item_db))
                    status = i2d_panic("failed to parse buffer");
                result = i2d_fd_read(fd, I2D_SIZE, &buffer);
            }
            if(!status && buffer.offset && i2d_by_line(&buffer, i2d_item_db_parse, item_db))
                status = i2d_panic("failed to parse buffer");
            i2d_buffer_destroy(&buffer);
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
        i2d_rbt_init(&item_db->index_by_name, i2d_rbt_cmp_str) ) {
        status = i2d_panic("failed to create red black tree object");
    } else {
        item = item_db->list;
        do {
            if( i2d_rbt_insert(item_db->index_by_id, &item->id, item) ||
                i2d_rbt_insert(item_db->index_by_name, item->name.string, item) )
                status = i2d_panic("failed to index item by id -- %ld", item->id);
            item = item->next;
        } while(item != item_db->list && !status);
    }

    return status;
}

int i2d_item_db_search_by_id(i2d_item_db * item_db, long id, i2d_item ** item) {
    return i2d_rbt_search(item_db->index_by_id, &id, (void **) item);
}

int i2d_item_db_search_by_name(i2d_item_db * item_db, const char * name, i2d_item ** item) {
    return i2d_rbt_search(item_db->index_by_name, name, (void **) item);
}

int i2d_item_combo_init(i2d_item_combo ** result, char * string, size_t length) {
    int status = I2D_OK;
    i2d_item_combo * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_item_combo_parse(object, string, length)) {
                status = i2d_panic("failed to load item combo -- %s", string);
            } else {
                object->next = object;
                object->prev = object;
            }

            if(status)
                i2d_item_combo_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_item_combo_deit(i2d_item_combo ** result) {
    i2d_item_combo * object;

    object = *result;
    i2d_string_destroy(&object->script);
    i2d_free(object->list);
    i2d_free(object);
    *result = NULL;
}

static int i2d_item_combo_parse_list(i2d_item_combo * item_combo, char * string, size_t length) {
    int status = I2D_OK;

    size_t i;
    size_t colon_count = 0;

    char * anchor;
    size_t extent;

    size_t index = 0;

    for(i = 0; i < length; i++)
        if(':' == string[i])
            colon_count++;

    item_combo->size = colon_count + 1;
    item_combo->list = calloc(item_combo->size, sizeof(*item_combo->list));
    if(!item_combo->list) {
        status = i2d_panic("out of memory");
    } else {
        anchor = string;
        for(i = 0; i < length && !status; i++) {
            if(':' == string[i]) {
                string[i] = 0;

                if((string + i) < anchor) {
                    status = i2d_panic("line overflow");
                } else {
                    extent = (size_t) (string + i) - (size_t) anchor;
                    if(index >= item_combo->size) {
                        status = i2d_panic("item combo overflow");
                    } else {
                        status = i2d_strtol(&item_combo->list[index], anchor, extent, 10);
                        index++;
                    }
                }

                anchor = (string + i + 1);
            }
        }

        if(!status) {
            if(index != item_combo->size - 1) {
                status = i2d_panic("row is missing columns");
            } else if(&string[i] < anchor) {
                status = i2d_panic("line overflow");
            } else {
                extent = (size_t) &string[i] - (size_t) anchor;
                status = i2d_strtol(&item_combo->list[index], anchor, extent, 10);
            }
        }
    }

    return status;
}

static int i2d_item_combo_parse(i2d_item_combo * item_combo, char * string, size_t length) {
    int status = I2D_OK;

    size_t i;
    int brace_depth = 0;

    char * anchor;
    size_t extent;

    int field = 0;

    anchor = string;
    for(i = 0; i < length && !status; i++) {
        if('{' == string[i]) {
            brace_depth++;
        } else if('}' == string[i]) {
            brace_depth--;
        } else if(',' == string[i] && !brace_depth) {
            string[i] = 0;

            if((string + i) < anchor) {
                status = i2d_panic("line overflow");
            } else {
                extent = (size_t) (string + i) - (size_t) anchor;
                switch(field) {
                    case 0: status = i2d_item_combo_parse_list(item_combo, anchor, extent); break;
                    default: status = i2d_panic("row has too many columns"); break;
                }
                field++;
            }

            anchor = (string + i + 1);
        }
    }

    if(!status) {
        if(1 != field) {
            status = i2d_panic("row is missing columns");
        } else if(&string[i] < anchor) {
            status = i2d_panic("line overflow");
        } else {
            extent = (size_t) &string[i] - (size_t) anchor;
            status = i2d_string_create(&item_combo->script, anchor, extent);
        }
    }

    return status;
}

void i2d_item_combo_append(i2d_item_combo * x, i2d_item_combo * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_item_combo_remove(i2d_item_combo * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

int i2d_item_combo_db_init(i2d_item_combo_db ** result, i2d_string * path) {
    int status = I2D_OK;
    i2d_item_combo_db * object;

    if(i2d_is_invalid(result) || !path) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_item_combo_db_load(object, path))
                status = i2d_panic("failed to load item combo db");

            if(status)
                i2d_item_combo_db_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_item_combo_db_deit(i2d_item_combo_db ** result) {
    i2d_item_combo_db * object;
    i2d_item_combo * item_combo;

    object = *result;
    if(object->list) {
        while(object->list != object->list->next) {
            item_combo = object->list->next;
            i2d_item_combo_remove(item_combo);
            i2d_item_combo_deit(&item_combo);
        }
        i2d_item_combo_deit(&object->list);
    }
    i2d_free(object);
    *result = NULL;
}

static int i2d_item_combo_db_load(i2d_item_combo_db * item_combo_db, i2d_string * path) {
    int status = I2D_OK;

    int fd;
    i2d_buffer buffer;
    int result;

    fd = open(path->string, O_RDONLY);
    if(0 > fd) {
        status = i2d_panic("failed to open item combo db -- %s", path->string);
    } else {
        if(i2d_buffer_create(&buffer, I2D_SIZE * 2)) {
            status = i2d_panic("failed to create buffer object");
        } else {
            result = i2d_fd_read(fd, I2D_SIZE, &buffer);
            while(0 < result && !status) {
                if(i2d_by_line(&buffer, i2d_item_combo_db_parse, item_combo_db))
                    status = i2d_panic("failed to parse buffer");
                result = i2d_fd_read(fd, I2D_SIZE, &buffer);
            }
            if(!status && buffer.offset && i2d_by_line(&buffer, i2d_item_combo_db_parse, item_combo_db))
                status = i2d_panic("failed to parse buffer");
            i2d_buffer_destroy(&buffer);
        }
        close(fd);
    }

    return status;
}

static int i2d_item_combo_db_parse(char * string, size_t length, void * data) {
    int status = I2D_OK;
    i2d_item_combo_db * item_combo_db = data;
    i2d_item_combo * item_combo = NULL;

    if(i2d_item_combo_init(&item_combo, string, length)) {
        status = i2d_panic("failed to create item combo object");
    } else {
        if(!item_combo_db->list) {
            item_combo_db->list = item_combo;
        } else {
            i2d_item_combo_append(item_combo, item_combo_db->list);
        }

        item_combo_db->size++;
    }

    return status;
}
