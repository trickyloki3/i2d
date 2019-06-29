#include "i2d_produce.h"

static int i2d_produce_create(i2d_produce *, char *, size_t);
static int i2d_produce_parse(i2d_produce *, char *, size_t);
static int i2d_produce_db_parse(char *, size_t, void *);
static int i2d_produce_db_index(i2d_produce_db *);
static int i2d_produce_db_create_produce_list(i2d_produce_db *, long, i2d_produce_list **);

int i2d_produce_init(i2d_produce ** result, char * string, size_t length) {
    int status = I2D_OK;
    i2d_produce * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_produce_create(object, string, length) || i2d_produce_parse(object, string, length)) {
                status = i2d_panic("failed to load produce -- %s", string);
            } else {
                object->next = object;
                object->prev = object;
            }

            if(status)
                i2d_produce_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_produce_deit(i2d_produce ** result) {
    i2d_produce * object;

    object = *result;
    i2d_free(object->materials);
    i2d_free(object);
    *result = NULL;
}

static int i2d_produce_create(i2d_produce * produce, char * string, size_t length) {
    int status = I2D_OK;

    size_t i;
    size_t comma_count = 0;

    for(i = 0; i < length; i++)
        if(',' == string[i])
            comma_count++;

    if(comma_count < 6) {
        status = i2d_panic("row is missing columns");
    } else {
        produce->material_count = comma_count - 4;
        if((produce->material_count % 2) != 0) {
            status = i2d_panic("mismatch in material item id and amount");
        } else {
            produce->materials = calloc(produce->material_count, sizeof(*produce->materials));
            if(!produce->materials)
                status = i2d_panic("out of memory");
        }
    }

    return status;
}

static int i2d_produce_parse(i2d_produce * produce, char * string, size_t length) {
    int status = I2D_OK;

    size_t i;

    char * anchor;
    size_t extent;

    int field = 0;
    int last = 0;

    size_t material_index = 0;

    anchor = string;
    for(i = 0; i < length && !status && !last; i++) {
        /*
         * check for \t, \r, \n (include space)
         */
        if(i2d_isspace(string[i]))
            last = 1;

        /*
         * check for line comments
         */
        if('/' == string[i] && i > 0 && '/' == string[i - 1]) {
            i -= 1;
            last = 1;
        }

        if(',' == string[i] || last) {
            string[i] = 0;

            if(string + i < anchor) {
                status = i2d_panic("line overflow");
            } else {
                extent = (size_t) (string + i) - (size_t) anchor;
                switch(field) {
                    case 0: status = i2d_strtol(&produce->id, anchor, extent, 10); break;
                    case 1: status = i2d_strtol(&produce->item_id, anchor, extent, 10); break;
                    case 2: status = i2d_strtol(&produce->item_level, anchor, extent, 10); break;
                    case 3: status = i2d_strtol(&produce->skill_id, anchor, extent, 10); break;
                    case 4: status = i2d_strtol(&produce->skill_level, anchor, extent, 10); break;
                    default:
                        if(material_index >= produce->material_count) {
                            status = i2d_panic("material overflow");
                        } else {
                            status = i2d_strtol(&produce->materials[material_index], anchor, extent, 10);
                            material_index++;
                        }
                        break;
                }
                field++;
            }

            anchor = (string + i + 1);
        }
    }

    if(!status && 5 > field) {
        status = i2d_panic("row is missing columns");
    } else if(material_index != produce->material_count) {
        status = i2d_panic("invalid material item id or count list");
    }

    return status;
}

void i2d_produce_append(i2d_produce * x, i2d_produce * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_produce_remove(i2d_produce * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

int i2d_produce_list_init(i2d_produce_list ** result, long item_level) {
    int status = I2D_OK;
    i2d_produce_list * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            object->item_level = item_level;
            object->next = object;
            object->prev = object;

            if(status)
                i2d_produce_list_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_produce_list_deit(i2d_produce_list ** result) {
    i2d_produce_list * object;

    object = *result;
    i2d_free(object->list);
    i2d_free(object);
    *result = NULL;
}

int i2d_produce_list_add(i2d_produce_list * produce_list, i2d_produce * produce) {
    int status = I2D_OK;
    i2d_produce ** list = NULL;
    size_t size;

    size = produce_list->size + 1;
    list = realloc(produce_list->list, size * sizeof(*produce_list->list));
    if(!list) {
        status = i2d_panic("out of memory");
    } else {
        list[size - 1] = produce;
        produce_list->list = list;
        produce_list->size = size;
    }

    return status;
}

void i2d_produce_list_append(i2d_produce_list * x, i2d_produce_list * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_produce_list_remove(i2d_produce_list * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

int i2d_produce_db_init(i2d_produce_db ** result, i2d_string * path) {
    int status = I2D_OK;
    i2d_produce_db * object;

    if(i2d_is_invalid(result) || !path) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_fd_load(path, i2d_produce_db_parse, object)) {
                status = i2d_panic("failed to load produce db");
            } else if(i2d_produce_db_index(object)) {
                status = i2d_panic("failed to index produce db");
            }

            if(status)
                i2d_produce_db_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_produce_db_deit(i2d_produce_db ** result) {
    i2d_produce_db * object;
    i2d_produce * produce;
    i2d_produce_list * produce_list;

    object = *result;
    i2d_deit(object->index_by_item_level, i2d_rbt_deit);
    i2d_deit(object->index_by_id, i2d_rbt_deit);
    if(object->list) {
        while(object->list != object->list->next) {
            produce = object->list->next;
            i2d_produce_remove(produce);
            i2d_produce_deit(&produce);
        }
        i2d_produce_deit(&object->list);
    }
    if(object->produce_list) {
        while(object->produce_list != object->produce_list->next) {
            produce_list = object->produce_list->next;
            i2d_produce_list_remove(produce_list);
            i2d_produce_list_deit(&produce_list);
        }
        i2d_produce_list_deit(&object->produce_list);
    }
    i2d_free(object);
    *result = NULL;
}

static int i2d_produce_db_parse(char * string, size_t length, void * data) {
    int status = I2D_OK;
    i2d_produce_db * produce_db = data;
    i2d_produce * produce = NULL;

    if(i2d_produce_init(&produce, string, length)) {
        status = i2d_panic("failed to create produce object");
    } else {
        if(!produce_db->list) {
            produce_db->list = produce;
        } else {
            i2d_produce_append(produce, produce_db->list);
        }

        produce_db->size++;
    }

    return status;
}

static int i2d_produce_db_index(i2d_produce_db * produce_db) {
    int status = I2D_OK;
    i2d_produce * produce = NULL;
    i2d_produce_list * produce_list;

    if(i2d_rbt_init(&produce_db->index_by_id, i2d_rbt_cmp_long)) {
        status = i2d_panic("failed to create red black tree object");
    } else if(i2d_rbt_init(&produce_db->index_by_item_level, i2d_rbt_cmp_long)) {
        status = i2d_panic("failed to create red black tree object");
    } else {
        produce = produce_db->list;
        do {
            if( i2d_rbt_insert(produce_db->index_by_id, &produce->id, produce)) {
                status = i2d_panic("failed to index produce by id -- %ld", produce->id);
            } else {
                if(i2d_produce_db_search_by_item_level(produce_db, produce->item_level, &produce_list)) {
                    if(i2d_produce_db_create_produce_list(produce_db, produce->item_level, &produce_list)) {
                        status = i2d_panic("failed to create produce list object");
                    } else if(i2d_produce_list_add(produce_list, produce)) {
                        status = i2d_panic("failed to add produce");
                    } else if(i2d_rbt_insert(produce_db->index_by_item_level, &produce_list->item_level, produce_list)) {
                        status = i2d_panic("failed to index produce list by id -- %d", produce_list->item_level);
                    }
                } else if(i2d_produce_list_add(produce_list, produce)) {
                    status = i2d_panic("failed to add produce");
                }
            }
            produce = produce->next;
        } while(produce != produce_db->list && !status);
    }

    return status;
}

static int i2d_produce_db_create_produce_list(i2d_produce_db * produce_db, long item_level, i2d_produce_list ** result) {
    int status = I2D_OK;
    i2d_produce_list * produce_list = NULL;

    if(i2d_produce_list_init(&produce_list, item_level)) {
        status = i2d_panic("failed to create produce list object");
    } else {
        if(!produce_db->produce_list) {
            produce_db->produce_list = produce_list;
        } else {
            i2d_produce_list_append(produce_list, produce_db->produce_list);
        }
        produce_db->produce_size++;
        *result = produce_list;
    }

    return status;
}

int i2d_produce_db_search_by_id(i2d_produce_db * produce_db, long id, i2d_produce ** produce) {
    return i2d_rbt_search(produce_db->index_by_id, &id, (void **) produce);
}

int i2d_produce_db_search_by_item_level(i2d_produce_db * produce_db, long item_level, i2d_produce_list ** produce_list) {
    return i2d_rbt_search(produce_db->index_by_item_level, &item_level, (void **) produce_list);
}