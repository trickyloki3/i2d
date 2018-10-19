#include "i2d_mercenary.h"

static int i2d_mercenary_parse(i2d_mercenary *, char *, size_t);
static int i2d_mercenary_db_parse(char *, size_t, void *);
static int i2d_mercenary_db_index(i2d_mercenary_db *);

int i2d_mercenary_init(i2d_mercenary ** result, char * string, size_t length) {
    int status = I2D_OK;
    i2d_mercenary * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_mercenary_parse(object, string, length)) {
                status = i2d_panic("failed to load mercenary -- %s", string);
            } else {
                object->next = object;
                object->prev = object;
            }

            if(status)
                i2d_mercenary_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_mercenary_deit(i2d_mercenary ** result) {
    i2d_mercenary * object;

    object = *result;
    i2d_string_destroy(&object->name);
    i2d_string_destroy(&object->sprite_name);
    i2d_free(object);
    *result = NULL;
}

static int i2d_mercenary_parse(i2d_mercenary * mercenary, char * string, size_t length) {
    int status = I2D_OK;

    size_t i;

    char * anchor;
    size_t extent;

    int field = 0;
    int last = 0;

    anchor = string;
    for(i = 0; i < length && !status && !last; i++) {
        /*
         * check for \t, \r, \n (exclude space)
         */
        if(isspace(string[i]) && ' ' != string[i]) 
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
                    case 0: status = i2d_strtol(&mercenary->id, anchor, extent, 10); break;
                    case 1: status = i2d_string_create(&mercenary->sprite_name, anchor, extent); break;
                    case 2: status = i2d_string_create(&mercenary->name, anchor, extent); break;
                    case 3: status = i2d_strtol(&mercenary->level, anchor, extent, 10); break;
                    case 4: status = i2d_strtol(&mercenary->hp, anchor, extent, 10); break;
                    case 5: status = i2d_strtol(&mercenary->sp, anchor, extent, 10); break;
                    case 6: status = i2d_strtol(&mercenary->range1, anchor, extent, 10); break;
                    case 7: status = i2d_strtol(&mercenary->atk1, anchor, extent, 10); break;
                    case 8: status = i2d_strtol(&mercenary->atk2, anchor, extent, 10); break;
                    case 9: status = i2d_strtol(&mercenary->def, anchor, extent, 10); break;
                    case 10: status = i2d_strtol(&mercenary->mdef, anchor, extent, 10); break;
                    case 11: status = i2d_strtol(&mercenary->str, anchor, extent, 10); break;
                    case 12: status = i2d_strtol(&mercenary->agi, anchor, extent, 10); break;
                    case 13: status = i2d_strtol(&mercenary->vit, anchor, extent, 10); break;
                    case 14: status = i2d_strtol(&mercenary->ini, anchor, extent, 10); break;
                    case 15: status = i2d_strtol(&mercenary->dex, anchor, extent, 10); break;
                    case 16: status = i2d_strtol(&mercenary->luk, anchor, extent, 10); break;
                    case 17: status = i2d_strtol(&mercenary->range2, anchor, extent, 10); break;
                    case 18: status = i2d_strtol(&mercenary->range3, anchor, extent, 10); break;
                    case 19: status = i2d_strtol(&mercenary->scale, anchor, extent, 10); break;
                    case 20: status = i2d_strtol(&mercenary->race, anchor, extent, 10); break;
                    case 21: status = i2d_strtol(&mercenary->element, anchor, extent, 10); break;
                    case 22: status = i2d_strtol(&mercenary->speed, anchor, extent, 10); break;
                    case 23: status = i2d_strtol(&mercenary->adelay, anchor, extent, 10); break;
                    case 24: status = i2d_strtol(&mercenary->amotion, anchor, extent, 10); break;
                    case 25: status = i2d_strtol(&mercenary->dmotion, anchor, extent, 10); break;
                    default: status = i2d_panic("row has too many columns"); break;
                }
                field++;
            }

            anchor = (string + i + 1);
        }
    }

    if(!status && 26 != field)
        status = i2d_panic("row is missing columns");

    return status;
}

void i2d_mercenary_append(i2d_mercenary * x, i2d_mercenary * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_mercenary_remove(i2d_mercenary * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

int i2d_mercenary_db_init(i2d_mercenary_db ** result, i2d_string * path) {
    int status = I2D_OK;
    i2d_mercenary_db * object;

    if(i2d_is_invalid(result) || !path) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_fd_load(path, i2d_mercenary_db_parse, object)) {
                status = i2d_panic("failed to load mercenary db");
            } else if(i2d_mercenary_db_index(object)) {
                status = i2d_panic("failed to index mercenary db");
            }

            if(status)
                i2d_mercenary_db_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_mercenary_db_deit(i2d_mercenary_db ** result) {
    i2d_mercenary_db * object;
    i2d_mercenary * mercenary;

    object = *result;
    i2d_deit(object->index_by_id, i2d_rbt_deit);
    if(object->list) {
        while(object->list != object->list->next) {
            mercenary = object->list->next;
            i2d_mercenary_remove(mercenary);
            i2d_mercenary_deit(&mercenary);
        }
        i2d_mercenary_deit(&object->list);
    }
    i2d_free(object);
    *result = NULL;
}

static int i2d_mercenary_db_parse(char * string, size_t length, void * data) {
    int status = I2D_OK;
    i2d_mercenary_db * mercenary_db = data;
    i2d_mercenary * mercenary = NULL;

    if(i2d_mercenary_init(&mercenary, string, length)) {
        status = i2d_panic("failed to create mercenary object");
    } else {
        if(!mercenary_db->list) {
            mercenary_db->list = mercenary;
        } else {
            i2d_mercenary_append(mercenary, mercenary_db->list);
        }

        mercenary_db->size++;
    }

    return status;
}

static int i2d_mercenary_db_index(i2d_mercenary_db * mercenary_db) {
    int status = I2D_OK;
    i2d_mercenary * mercenary = NULL;

    if(i2d_rbt_init(&mercenary_db->index_by_id, i2d_rbt_cmp_long)) {
        status = i2d_panic("failed to create red black tree object");
    } else {
        mercenary = mercenary_db->list;
        do {
            if( i2d_rbt_insert(mercenary_db->index_by_id, &mercenary->id, mercenary))
                status = i2d_panic("failed to index mercenary by id -- %ld", mercenary->id);
            mercenary = mercenary->next;
        } while(mercenary != mercenary_db->list && !status);
    }

    return status;
}

int i2d_mercenary_db_search_by_id(i2d_mercenary_db * mercenary_db, long id, i2d_mercenary ** mercenary) {
    return i2d_rbt_search(mercenary_db->index_by_id, &id, (void **) mercenary);
}
