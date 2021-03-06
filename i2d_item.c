#include "i2d_item.h"

static int i2d_item_parse_optional(long *, long *, char *, size_t);
static int i2d_item_parse(i2d_item *, char *, size_t);
static int i2d_item_db_parse(char *, size_t, void *);
static int i2d_item_db_index(i2d_item_db *);

static int i2d_item_combo_parse_list(i2d_item_combo *, char *, size_t);
static int i2d_item_combo_parse(i2d_item_combo *, char *, size_t);
static int i2d_item_combo_db_parse(char *, size_t, void *);
static int i2d_item_combo_db_index(i2d_item_combo_db *);
static int i2d_item_combo_db_create_combo_list(i2d_item_combo_db *, long, i2d_item_combo_list **);

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
    i2d_free(object->script_description.string);
    i2d_free(object->onequip_script.string);
    i2d_free(object->onequip_script_description.string);
    i2d_free(object->onunequip_script.string);
    i2d_free(object->onunequip_script_description.string);
    i2d_free(object->combo_description.string);
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
    int last = 0;

    anchor = string;
    for(i = 0; i < length && !status && !last; i++) {
        switch(string[i]) {
            case '"': quote_depth = !quote_depth; break;
            case '{': brace_depth++; break;
            case '}': brace_depth--; break;
            default:
                /*
                 * check for \t, \r, \n (exclude space)
                 */
                if(i2d_isspace(string[i]) && ' ' != string[i])
                    last = 1;

                /*
                 * check for line comments
                 */
                if('/' == string[i] && i > 0 && '/' == string[i - 1]) {
                    i -= 1;
                    last = 1;
                }

                if((',' == string[i] || last) && !quote_depth && !brace_depth) {
                    string[i] = 0;

                    if(string + i < anchor) {
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
                            case 21: status = i2d_string_create(&item->onunequip_script, anchor, extent); break;
                            default: status = i2d_panic("row has too many columns"); break;
                        }
                        field++;
                    }

                    anchor = (string + i + 1);
                }
        }
    }

    if(!status && 22 != field)
        status = i2d_panic("row is missing columns");

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
            if(i2d_fd_load(path, i2d_item_db_parse, object)) {
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

int i2d_item_combo_list_init(i2d_item_combo_list ** result, long item_id) {
    int status = I2D_OK;
    i2d_item_combo_list * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            object->item_id = item_id;
            object->next = object;
            object->prev = object;

            if(status)
                i2d_item_combo_list_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_item_combo_list_deit(i2d_item_combo_list ** result) {
    i2d_item_combo_list * object;

    object = *result;
    i2d_free(object->list);
    i2d_free(object);
    *result = NULL;
}

int i2d_item_combo_list_add(i2d_item_combo_list * item_combo_list, i2d_item_combo * item_combo) {
    int status = I2D_OK;
    i2d_item_combo ** list = NULL;
    size_t size;

    size = item_combo_list->size + 1;
    list = realloc(item_combo_list->list, size * sizeof(*item_combo_list->list));
    if(!list) {
        status = i2d_panic("out of memory");
    } else {
        list[size - 1] = item_combo;
        item_combo_list->list = list;
        item_combo_list->size = size;
    }

    return status;
}

void i2d_item_combo_list_append(i2d_item_combo_list * x, i2d_item_combo_list * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_item_combo_list_remove(i2d_item_combo_list * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
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
    int last = 0;

    anchor = string;
    for(i = 0; i < length && !status && !last; i++) {
        switch(string[i]) {
            case '{': brace_depth++; break;
            case '}': brace_depth--; break;
            default:
                /*
                 * check for \t, \r, \n (exclude space)
                 */
                if(i2d_isspace(string[i]) && ' ' != string[i])
                    last = 1;

                /*
                 * check for line comments
                 */
                if('/' == string[i] && i > 0 && '/' == string[i - 1]) {
                    i -= 1;
                    last = 1;
                }

                if((',' == string[i] || last) && !brace_depth) {
                    string[i] = 0;

                    if(string + i < anchor) {
                        status = i2d_panic("line overflow");
                    } else {
                        extent = (size_t) (string + i) - (size_t) anchor;
                        switch(field) {
                            case 0: status = i2d_item_combo_parse_list(item_combo, anchor, extent); break;
                            case 1: status = i2d_string_create(&item_combo->script, anchor, extent); break;
                            default: status = i2d_panic("row has too many columns"); break;
                        }
                        field++;
                    }

                    anchor = (string + i + 1);
                }
        }
    }

    if(!status && 2 != field)
        status = i2d_panic("row is missing columns");

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

int i2d_item_combo_get_string(i2d_item_combo * item_combo, i2d_item_db * item_db, i2d_string * result) {
    int status = I2D_OK;

    i2d_string_stack * stack = NULL;
    i2d_buffer * buffer = NULL;

    size_t i;
    i2d_item * item;
    i2d_string list;

    if(i2d_string_stack_init(&stack, item_combo->size)) {
        status = i2d_panic("failed to create string stack object");
    } else {
        if(i2d_buffer_init(&buffer, BUFFER_SIZE_LARGE)) {
            status = i2d_panic("failed to create buffer object");
        } else {
            for(i = 0; i < item_combo->size && !status; i++) {
                if(i2d_item_db_search_by_id(item_db, item_combo->list[i], &item)) {
                    status = i2d_panic("failed to get item by id -- %ld", item_combo->list[i]);
                } else if(i2d_string_stack_push(stack, item->name.string, item->name.length)) {
                    status = i2d_panic("failed to push string stack object");
                }
            }

            if(!status) {
                if(i2d_string_stack_dump_buffer(stack, buffer, ", ")) {
                    status = i2d_panic("failed to get item list");
                } else {
                    i2d_buffer_get(buffer, &list.string, &list.length);
                    if(i2d_string_create(result, list.string, list.length))
                        status = i2d_panic("failed to create string object");
                }
            }
            i2d_buffer_deit(&buffer);
        }
        i2d_string_stack_deit(&stack);
    }

    return status;
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
            if(i2d_fd_load(path, i2d_item_combo_db_parse, object)) {
                status = i2d_panic("failed to load item combo db");
            } else if(i2d_item_combo_db_index(object)) {
                status = i2d_panic("failed to index item combo db");
            }

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
    i2d_item_combo_list * item_combo_list;

    object = *result;
    i2d_deit(object->index_by_id, i2d_rbt_deit);
    if(object->list) {
        while(object->list != object->list->next) {
            item_combo = object->list->next;
            i2d_item_combo_remove(item_combo);
            i2d_item_combo_deit(&item_combo);
        }
        i2d_item_combo_deit(&object->list);
    }
    if(object->combo_list) {
        while(object->combo_list != object->combo_list->next) {
            item_combo_list = object->combo_list->next;
            i2d_item_combo_list_remove(item_combo_list);
            i2d_item_combo_list_deit(&item_combo_list);
        }
        i2d_item_combo_list_deit(&object->combo_list);
    }
    i2d_free(object);
    *result = NULL;
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

static int i2d_item_combo_db_index(i2d_item_combo_db * item_combo_db) {
    int status = I2D_OK;
    i2d_item_combo * item_combo = NULL;

    size_t i;
    i2d_item_combo_list * item_combo_list;

    if(i2d_rbt_init(&item_combo_db->index_by_id, i2d_rbt_cmp_long)) {
        status = i2d_panic("failed to create red black tree object");
    } else {
        item_combo = item_combo_db->list;
        do {
            for(i = 0; i < item_combo->size; i++) {
                if(i2d_item_combo_db_search_by_id(item_combo_db, item_combo->list[i], &item_combo_list)) {
                    if(i2d_item_combo_db_create_combo_list(item_combo_db, item_combo->list[i], &item_combo_list)) {
                        status = i2d_panic("failed to create item combo list object");
                    } else if(i2d_item_combo_list_add(item_combo_list, item_combo)) {
                        status = i2d_panic("failed to add item combo");
                    } else if(i2d_rbt_insert(item_combo_db->index_by_id, &item_combo_list->item_id, item_combo_list)) {
                        status = i2d_panic("failed to index item combo list by id -- %d", item_combo_list->item_id);
                    }
                } else if(i2d_item_combo_list_add(item_combo_list, item_combo)) {
                    status = i2d_panic("failed to add item combo");
                }
            }
            item_combo = item_combo->next;
        } while(item_combo != item_combo_db->list && !status);
    }

    return status;
}

static int i2d_item_combo_db_create_combo_list(i2d_item_combo_db * item_combo_db, long item_id, i2d_item_combo_list ** result) {
    int status = I2D_OK;
    i2d_item_combo_list * item_combo_list = NULL;

    if(i2d_item_combo_list_init(&item_combo_list, item_id)) {
        status = i2d_panic("failed to create item combo list object");
    } else {
        if(!item_combo_db->combo_list) {
            item_combo_db->combo_list = item_combo_list;
        } else {
            i2d_item_combo_list_append(item_combo_list, item_combo_db->combo_list);
        }
        item_combo_db->combo_size++;
        *result = item_combo_list;
    }

    return status;
}

int i2d_item_combo_db_search_by_id(i2d_item_combo_db * item_combo_db, long id, i2d_item_combo_list ** item_combo_list) {
    return i2d_rbt_search(item_combo_db->index_by_id, &id, (void **) item_combo_list);
}
