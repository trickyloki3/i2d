#include "i2d_mob.h"

static int i2d_mob_parse(i2d_mob *, char *, size_t);
static int i2d_mob_db_parse(char *, size_t, void *);
static int i2d_mob_db_index(i2d_mob_db *);
static int i2d_mob_race_parse(i2d_mob_race *, char *, size_t);
static int i2d_mob_race_db_parse(char *, size_t, void *);
static int i2d_mob_race_db_index(i2d_mob_race_db *);

int i2d_mob_init(i2d_mob ** result, char * string, size_t length) {
    int status = I2D_OK;
    i2d_mob * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_mob_parse(object, string, length)) {
                status = i2d_panic("failed to load mob -- %s", string);
            } else {
                object->next = object;
                object->prev = object;
            }

            if(status)
                i2d_mob_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_mob_deit(i2d_mob ** result) {
    i2d_mob * object;

    object = *result;
    i2d_free(object->sprite.string);
    i2d_free(object->kro.string);
    i2d_free(object->iro.string);
    i2d_free(object);
    *result = NULL;
}

static int i2d_mob_parse(i2d_mob * mob, char * string, size_t length) {
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
                    case 0: status = i2d_strtol(&mob->id, anchor, extent, 10); break;
                    case 1: status = i2d_string_create(&mob->sprite, anchor, extent); break;
                    case 2: status = i2d_string_create(&mob->kro, anchor, extent); break;
                    case 3: status = i2d_string_create(&mob->iro, anchor, extent); break;
                    case 4: status = i2d_strtol(&mob->level, anchor, extent, 10); break;
                    case 5: status = i2d_strtol(&mob->hp, anchor, extent, 10); break;
                    case 6: status = i2d_strtol(&mob->sp, anchor, extent, 10); break;
                    case 7: status = i2d_strtol(&mob->exp, anchor, extent, 10); break;
                    case 8: status = i2d_strtol(&mob->jexp, anchor, extent, 10); break;
                    case 9: status = i2d_strtol(&mob->range1, anchor, extent, 10); break;
                    case 10: status = i2d_strtol(&mob->atk1, anchor, extent, 10); break;
                    case 11: status = i2d_strtol(&mob->atk2, anchor, extent, 10); break;
                    case 12: status = i2d_strtol(&mob->def, anchor, extent, 10); break;
                    case 13: status = i2d_strtol(&mob->mdef, anchor, extent, 10); break;
                    case 14: status = i2d_strtol(&mob->str, anchor, extent, 10); break;
                    case 15: status = i2d_strtol(&mob->agi, anchor, extent, 10); break;
                    case 16: status = i2d_strtol(&mob->vit, anchor, extent, 10); break;
                    case 17: status = i2d_strtol(&mob->inte, anchor, extent, 10); break;
                    case 18: status = i2d_strtol(&mob->dex, anchor, extent, 10); break;
                    case 19: status = i2d_strtol(&mob->luk, anchor, extent, 10); break;
                    case 20: status = i2d_strtol(&mob->range2, anchor, extent, 10); break;
                    case 21: status = i2d_strtol(&mob->range3, anchor, extent, 10); break;
                    case 22: status = i2d_strtol(&mob->scale, anchor, extent, 10); break;
                    case 23: status = i2d_strtol(&mob->race, anchor, extent, 10); break;
                    case 24: status = i2d_strtol(&mob->element, anchor, extent, 10); break;
                    case 25: status = i2d_strtol(&mob->mode, anchor, extent, 16); break;
                    case 26: status = i2d_strtol(&mob->speed, anchor, extent, 10); break;
                    case 27: status = i2d_strtol(&mob->adelay, anchor, extent, 10); break;
                    case 28: status = i2d_strtol(&mob->amotion, anchor, extent, 10); break;
                    case 29: status = i2d_strtol(&mob->dmotion, anchor, extent, 10); break;
                    case 30: status = i2d_strtod(&mob->mexp, anchor, extent); break;
                    case 31: status = i2d_strtol(&mob->mvp_drop_id[0], anchor, extent, 10); break;
                    case 32: status = i2d_strtol(&mob->mvp_drop_pre[0], anchor, extent, 10); break;
                    case 33: status = i2d_strtol(&mob->mvp_drop_id[1], anchor, extent, 10); break;
                    case 34: status = i2d_strtol(&mob->mvp_drop_pre[1], anchor, extent, 10); break;
                    case 35: status = i2d_strtol(&mob->mvp_drop_id[2], anchor, extent, 10); break;
                    case 36: status = i2d_strtol(&mob->mvp_drop_pre[2], anchor, extent, 10); break;
                    case 37: status = i2d_strtol(&mob->drop_id[0], anchor, extent, 10); break;
                    case 38: status = i2d_strtol(&mob->drop_pre[0], anchor, extent, 10); break;
                    case 39: status = i2d_strtol(&mob->drop_id[1], anchor, extent, 10); break;
                    case 40: status = i2d_strtol(&mob->drop_pre[1], anchor, extent, 10); break;
                    case 41: status = i2d_strtol(&mob->drop_id[2], anchor, extent, 10); break;
                    case 42: status = i2d_strtol(&mob->drop_pre[2], anchor, extent, 10); break;
                    case 43: status = i2d_strtol(&mob->drop_id[3], anchor, extent, 10); break;
                    case 44: status = i2d_strtol(&mob->drop_pre[3], anchor, extent, 10); break;
                    case 45: status = i2d_strtol(&mob->drop_id[4], anchor, extent, 10); break;
                    case 46: status = i2d_strtol(&mob->drop_pre[4], anchor, extent, 10); break;
                    case 47: status = i2d_strtol(&mob->drop_id[5], anchor, extent, 10); break;
                    case 48: status = i2d_strtol(&mob->drop_pre[5], anchor, extent, 10); break;
                    case 49: status = i2d_strtol(&mob->drop_id[6], anchor, extent, 10); break;
                    case 50: status = i2d_strtol(&mob->drop_pre[6], anchor, extent, 10); break;
                    case 51: status = i2d_strtol(&mob->drop_id[7], anchor, extent, 10); break;
                    case 52: status = i2d_strtol(&mob->drop_pre[7], anchor, extent, 10); break;
                    case 53: status = i2d_strtol(&mob->drop_id[8], anchor, extent, 10); break;
                    case 54: status = i2d_strtol(&mob->drop_pre[8], anchor, extent, 10); break;
                    case 55: status = i2d_strtol(&mob->drop_card_id, anchor, extent, 10); break;
                    default: status = i2d_panic("row has too many columns"); break;
                }
                field++;
            }

            anchor = (string + i + 1);
        }
    }

    if(!status) {
        if(56 != field) {
            status = i2d_panic("row is missing columns");
        } else if(&string[i] < anchor) {
            status = i2d_panic("line overflow");
        } else {
            extent = (size_t) &string[i] - (size_t) anchor;
            status = i2d_strtol(&mob->drop_card_per, anchor, extent, 10);
        }
    }

    return status;
}

void i2d_mob_append(i2d_mob * x, i2d_mob * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_mob_remove(i2d_mob * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

int i2d_mob_db_init(i2d_mob_db ** result, i2d_string * path) {
    int status = I2D_OK;
    i2d_mob_db * object;

    if(i2d_is_invalid(result) || !path) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_fd_load(path, i2d_mob_db_parse, object)) {
                status = i2d_panic("failed to load mob db");
            } else if(i2d_mob_db_index(object)) {
                status = i2d_panic("failed to index mob db");
            }

            if(status)
                i2d_mob_db_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_mob_db_deit(i2d_mob_db ** result) {
    i2d_mob_db * object;
    i2d_mob * mob;

    object = *result;
    i2d_deit(object->index_by_id, i2d_rbt_deit);
    if(object->list) {
        while(object->list != object->list->next) {
            mob = object->list->next;
            i2d_mob_remove(mob);
            i2d_mob_deit(&mob);
        }
        i2d_mob_deit(&object->list);
    }
    i2d_free(object);
    *result = NULL;
}

static int i2d_mob_db_parse(char * string, size_t length, void * data) {
    int status = I2D_OK;
    i2d_mob_db * mob_db = data;
    i2d_mob * mob = NULL;

    if(i2d_mob_init(&mob, string, length)) {
        status = i2d_panic("failed to create mob object");
    } else {
        if(!mob_db->list) {
            mob_db->list = mob;
        } else {
            i2d_mob_append(mob, mob_db->list);
        }

        mob_db->size++;
    }

    return status;
}

static int i2d_mob_db_index(i2d_mob_db * mob_db) {
    int status = I2D_OK;
    i2d_mob * mob = NULL;

    if(i2d_rbt_init(&mob_db->index_by_id, i2d_rbt_cmp_long)) {
        status = i2d_panic("failed to create red black tree object");
    } else {
        mob = mob_db->list;
        do {
            if(i2d_rbt_insert(mob_db->index_by_id, &mob->id, mob))
                status = i2d_panic("failed to index mob by id -- %ld", mob->id);
            mob = mob->next;
        } while(mob != mob_db->list && !status);
    }

    return status;
}

int i2d_mob_db_search_by_id(i2d_mob_db * mob_db, long id, i2d_mob ** mob) {
    return i2d_rbt_search(mob_db->index_by_id, &id, (void **) mob);
}

int i2d_mob_race_init(i2d_mob_race ** result, char * string, size_t length) {
    int status = I2D_OK;
    i2d_mob_race * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_mob_race_parse(object, string, length)) {
                status = i2d_panic("failed to load mob race -- %s", string);
            } else {
                object->next = object;
                object->prev = object;
            }

            if(status)
                i2d_mob_race_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_mob_race_deit(i2d_mob_race ** result) {
    i2d_mob_race * object;

    object = *result;
    i2d_free(object->list);
    i2d_string_destroy(&object->macro);
    i2d_free(object);
    *result = NULL;
}

static int i2d_mob_race_parse(i2d_mob_race * mob_race, char * string, size_t length) {
    int status = I2D_OK;

    size_t i;
    size_t comma_count = 0;
    int quote_depth = 0;
    int brace_depth = 0;

    char * anchor;
    size_t extent;

    int field = 0;
    int last = 0;

    for(i = 0; i < length; i++)
        if(',' == string[i])
            comma_count++;

    if(!comma_count) {
        status = i2d_panic("empty list of mob id");
    } else {
        mob_race->list = calloc(comma_count, sizeof(*mob_race->list));
        if(!mob_race->list) {
            status = i2d_panic("out of memory");
        } else {
            anchor = string;
            for(i = 0; i < length && !status && !last; i++) {
                switch(string[i]) {
                    case '"': quote_depth = !quote_depth; break;
                    case '{': brace_depth++; break;
                    case '}': brace_depth--; break;
                    default:
                        /*
                         * check for \t, \r, \n, space
                         */
                        if(isspace(string[i])) 
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
                                    case 0: status = i2d_string_create(&mob_race->macro, anchor, extent); break;
                                    default: 
                                        if(mob_race->size >= comma_count) {
                                            status = i2d_panic("list overflow");
                                        } else {
                                            status = i2d_strtol(&mob_race->list[mob_race->size], anchor, extent, 10); 
                                            mob_race->size++;
                                        }
                                        break;
                                }
                                field++;
                            }

                            anchor = (string + i + 1);
                        }
                }
            }
        }
    }

    return status;
}

void i2d_mob_race_append(i2d_mob_race * x, i2d_mob_race * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_mob_race_remove(i2d_mob_race * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

int i2d_mob_race_db_init(i2d_mob_race_db ** result, i2d_string * path) {
    int status = I2D_OK;
    i2d_mob_race_db * object;

    if(i2d_is_invalid(result) || !path) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_fd_load(path, i2d_mob_race_db_parse, object)) {
                status = i2d_panic("failed to load mob race db");
            } else if(i2d_mob_race_db_index(object)) {
                status = i2d_panic("failed to index mob race db");
            }

            if(status)
                i2d_mob_race_db_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_mob_race_db_deit(i2d_mob_race_db ** result) {
    i2d_mob_race_db * object;
    i2d_mob_race * mob_race;

    object = *result;
    i2d_deit(object->index_by_macro, i2d_rbt_deit);
    if(object->list) {
        while(object->list != object->list->next) {
            mob_race = object->list->next;
            i2d_mob_race_remove(mob_race);
            i2d_mob_race_deit(&mob_race);
        }
        i2d_mob_race_deit(&object->list);
    }
    i2d_free(object);
    *result = NULL;
}

static int i2d_mob_race_db_parse(char * string, size_t length, void * data) {
    int status = I2D_OK;
    i2d_mob_race_db * mob_db_race = data;
    i2d_mob_race * mob_race = NULL;

    if(i2d_mob_race_init(&mob_race, string, length)) {
        status = i2d_panic("failed to create mob race object");
    } else {
        if(!mob_db_race->list) {
            mob_db_race->list = mob_race;
        } else {
            i2d_mob_race_append(mob_race, mob_db_race->list);
        }

        mob_db_race->size++;
    }

    return status;
}

static int i2d_mob_race_db_index(i2d_mob_race_db * mob_race_db) {
    int status = I2D_OK;
    i2d_mob_race * mob_race = NULL;

    if(i2d_rbt_init(&mob_race_db->index_by_macro, i2d_rbt_cmp_str)) {
        status = i2d_panic("failed to create red black tree object");
    } else {
        mob_race = mob_race_db->list;
        do {
            if(i2d_rbt_insert(mob_race_db->index_by_macro, mob_race->macro.string, mob_race))
                status = i2d_panic("failed to index mob race by macro -- %s", mob_race->macro.string);
            mob_race = mob_race->next;
        } while(mob_race != mob_race_db->list && !status);
    }

    return status;
}

int i2d_mob_race_db_search_by_macro(i2d_mob_race_db * mob_race_db, const char * key, i2d_mob_race ** mob_race) {
    return i2d_rbt_search(mob_race_db->index_by_macro, key, (void **) mob_race);
}
