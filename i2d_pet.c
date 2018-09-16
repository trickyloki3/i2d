#include "i2d_pet.h"

static int i2d_pet_parse(i2d_pet *, char *, size_t);
static int i2d_pet_db_parse(char *, size_t, void *);
static int i2d_pet_db_index(i2d_pet_db *);

int i2d_pet_init(i2d_pet ** result, char * string, size_t length) {
    int status = I2D_OK;
    i2d_pet * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_pet_parse(object, string, length)) {
                status = i2d_panic("failed to load pet -- %s", string);
            } else {
                object->next = object;
                object->prev = object;
            }

            if(status)
                i2d_pet_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_pet_deit(i2d_pet ** result) {
    i2d_pet * object;

    object = *result;
    i2d_string_destroy(&object->loyal_script);
    i2d_string_destroy(&object->pet_script);
    i2d_string_destroy(&object->jname);
    i2d_string_destroy(&object->name);
    i2d_free(object);
    *result = NULL;
}

static int i2d_pet_parse(i2d_pet * pet, char * string, size_t length) {
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
                    case 0: status = i2d_strtol(&pet->id, anchor, extent, 10); break;
                    case 1: status = i2d_string_create(&pet->name, anchor, extent); break;
                    case 2: status = i2d_string_create(&pet->jname, anchor, extent); break;
                    case 3: status = i2d_strtol(&pet->lure_id, anchor, extent, 10); break;
                    case 4: status = i2d_strtol(&pet->egg_id, anchor, extent, 10); break;
                    case 5: status = i2d_strtol(&pet->equip_id, anchor, extent, 10); break;
                    case 6: status = i2d_strtol(&pet->food_id, anchor, extent, 10); break;
                    case 7: status = i2d_strtol(&pet->fullness, anchor, extent, 10); break;
                    case 8: status = i2d_strtol(&pet->hungry_delay, anchor, extent, 10); break;
                    case 9: status = i2d_strtol(&pet->r_hungry, anchor, extent, 10); break;
                    case 10: status = i2d_strtol(&pet->r_full, anchor, extent, 10); break;
                    case 11: status = i2d_strtol(&pet->intimate, anchor, extent, 10); break;
                    case 12: status = i2d_strtol(&pet->die, anchor, extent, 10); break;
                    case 13: status = i2d_strtol(&pet->capture, anchor, extent, 10); break;
                    case 14: status = i2d_strtol(&pet->speed, anchor, extent, 10); break;
                    case 15: status = i2d_strtol(&pet->s_performance, anchor, extent, 10); break;
                    case 16: status = i2d_strtol(&pet->talk_convert_class, anchor, extent, 10); break;
                    case 17: status = i2d_strtol(&pet->attack_rate, anchor, extent, 10); break;
                    case 18: status = i2d_strtol(&pet->defence_attack_rate, anchor, extent, 10); break;
                    case 19: status = i2d_strtol(&pet->change_target_rate, anchor, extent, 10); break;
                    case 20: status = i2d_string_create(&pet->pet_script, anchor, extent); break;
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
            status = i2d_string_create(&pet->loyal_script, anchor, extent);
        }
    }

    return status;
}

void i2d_pet_append(i2d_pet * x, i2d_pet * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_pet_remove(i2d_pet * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

int i2d_pet_db_init(i2d_pet_db ** result, i2d_string * path) {
    int status = I2D_OK;
    i2d_pet_db * object;

    if(i2d_is_invalid(result) || !path) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_fd_load(path, i2d_pet_db_parse, object)) {
                status = i2d_panic("failed to load pet db");
            } else if(i2d_pet_db_index(object)) {
                status = i2d_panic("failed to index pet db");
            }

            if(status)
                i2d_pet_db_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_pet_db_deit(i2d_pet_db ** result) {
    i2d_pet_db * object;
    i2d_pet * pet;

    object = *result;
    i2d_deit(object->index_by_id, i2d_rbt_deit);
    if(object->list) {
        while(object->list != object->list->next) {
            pet = object->list->next;
            i2d_pet_remove(pet);
            i2d_pet_deit(&pet);
        }
        i2d_pet_deit(&object->list);
    }
    i2d_free(object);
    *result = NULL;
}

static int i2d_pet_db_parse(char * string, size_t length, void * data) {
    int status = I2D_OK;
    i2d_pet_db * pet_db = data;
    i2d_pet * pet = NULL;

    if(i2d_pet_init(&pet, string, length)) {
        status = i2d_panic("failed to create pet object");
    } else {
        if(!pet_db->list) {
            pet_db->list = pet;
        } else {
            i2d_pet_append(pet, pet_db->list);
        }

        pet_db->size++;
    }

    return status;
}

static int i2d_pet_db_index(i2d_pet_db * pet_db) {
    int status = I2D_OK;
    i2d_pet * pet = NULL;

    if(i2d_rbt_init(&pet_db->index_by_id, i2d_rbt_cmp_long)) {
        status = i2d_panic("failed to create red black tree object");
    } else {
        pet = pet_db->list;
        do {
            if( i2d_rbt_insert(pet_db->index_by_id, &pet->id, pet))
                status = i2d_panic("failed to index pet by id -- %ld", pet->id);
            pet = pet->next;
        } while(pet != pet_db->list && !status);
    }

    return status;
}

int i2d_pet_db_search_by_id(i2d_pet_db * pet_db, long id, i2d_pet ** pet) {
    return i2d_rbt_search(pet_db->index_by_id, &id, (void **) pet);
}
