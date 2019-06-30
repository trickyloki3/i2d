#include "i2d_pet.h"

struct i2d_pet_state {
    i2d_pet_db * pet_db;
};

typedef struct i2d_pet_state i2d_pet_state;

int i2d_pet_state_init(i2d_pet_state **, i2d_pet_db *);
void i2d_pet_state_deit(i2d_pet_state **);

static int i2d_pet_db_parse_txt(char *, size_t, void *);
static int i2d_pet_parse_txt(i2d_pet *, char *, size_t);
static int i2d_pet_db_parse_yml(i2d_pet_db *, i2d_string *);
static int i2d_pet_parse_yml(yaml_event_t *, void *);

int i2d_pet_init(i2d_pet ** result) {
    int status = I2D_OK;
    i2d_pet * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            object->next = object;
            object->prev = object;

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

int i2d_pet_item_init(i2d_pet_item ** result) {
    int status = I2D_OK;
    i2d_pet_item * object = NULL;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            object->next = object;
            object->prev = object;

            if(status) {
                i2d_pet_item_deit(&object);
            } else {
                *result = object;
            }
        }
    }

    return status;
}

void i2d_pet_item_deit(i2d_pet_item ** result) {
    i2d_pet_item * object;

    object = *result;
    i2d_string_destroy(&object->item);
    i2d_free(object);
    *result = NULL;
}

void i2d_pet_item_append(i2d_pet_item * x, i2d_pet_item * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_pet_item_remove(i2d_pet_item * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

int i2d_pet_evolution_init(i2d_pet_evolution ** result) {
    int status = I2D_OK;
    i2d_pet_evolution * object = NULL;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            object->next = object;
            object->prev = object;

            if(status) {
                i2d_pet_evolution_deit(&object);
            } else {
                *result = object;
            }
        }
    }

    return status;
}

void i2d_pet_evolution_deit(i2d_pet_evolution ** result) {
    i2d_pet_evolution * object;
    i2d_pet_item * item_req;

    object = *result;
    if(object->item_list) {
        while(object->item_list != object->item_list->next) {
            item_req = object->item_list->next;
            i2d_pet_item_remove(item_req);
            i2d_pet_item_deit(&item_req);
        }
        i2d_pet_item_deit(&object->item_list);
    }
    i2d_string_destroy(&object->target);
    i2d_free(object);
    *result = NULL;
}

void i2d_pet_evolution_append(i2d_pet_evolution * x, i2d_pet_evolution * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_pet_evolution_remove(i2d_pet_evolution * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

int i2d_pet_yml_init(i2d_pet_yml ** result) {
    int status = I2D_OK;
    i2d_pet_yml * object = NULL;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            object->hungry_delay = 60;
            object->hungry_increase = 20;
            object->intimacy_start = 250;
            object->intimacy_fed = 50;
            object->intimacy_overfed = -100;
            object->intimacy_hungry = -5;
            object->intimacy_owner_die = -20;
            object->special_performance = 1;
            object->next = object;
            object->prev = object;

            if(status) {
                i2d_pet_yml_deit(&object);
            } else {
                *result = object;
            }
        }
    }

    return status;
}

void i2d_pet_yml_deit(i2d_pet_yml ** result) {
    i2d_pet_yml * object;
    i2d_pet_evolution * evolution;

    object = *result;
    if(object->evolution_list) {
        while(object->evolution_list != object->evolution_list->next) {
            evolution = object->evolution_list->next;
            i2d_pet_evolution_remove(evolution);
            i2d_pet_evolution_deit(&evolution);
        }
        i2d_pet_evolution_deit(&object->evolution_list);
    }
    i2d_string_destroy(&object->support_script);
    i2d_string_destroy(&object->script);
    i2d_string_destroy(&object->food_item);
    i2d_string_destroy(&object->equip_item);
    i2d_string_destroy(&object->egg_item);
    i2d_string_destroy(&object->tame_item);
    i2d_string_destroy(&object->mob);
    i2d_free(object);
    *result = NULL;
}

void i2d_pet_yml_append(i2d_pet_yml * x, i2d_pet_yml * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_pet_yml_remove(i2d_pet_yml * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

int i2d_pet_state_init(i2d_pet_state ** result, i2d_pet_db * pet_db) {
    int status = I2D_OK;
    i2d_pet_state * object = NULL;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            object->pet_db = pet_db;

            if(status) {
                i2d_pet_state_deit(&object);
            } else {
                *result = object;
            }
        }
    }

    return status;
}

void i2d_pet_state_deit(i2d_pet_state ** result) {
    i2d_pet_state * object;

    object = * result;
    i2d_free(object);
    *result = NULL;
}

static int i2d_pet_db_parse_txt(char * string, size_t length, void * data) {
    int status = I2D_OK;
    i2d_pet_db * pet_db = data;
    i2d_pet * pet = NULL;

    if(i2d_pet_init(&pet)) {
        status = i2d_panic("failed to create pet object");
    } else if(i2d_pet_parse_txt(pet, string, length)) {
        status = i2d_panic("failed to load pet -- %s", string);
    } else if(i2d_rbt_insert(pet_db->index, &pet->id, pet)) {
        status = i2d_panic("failed to index pet by id -- %ld", pet->id);
    } else {
        i2d_pet_append(pet, pet_db->list);
    }

    return status;
}

static int i2d_pet_parse_txt(i2d_pet * pet, char * string, size_t length) {
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
                            case 21: status = i2d_string_create(&pet->loyal_script, anchor, extent); break;
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

static int i2d_pet_db_parse_yml(i2d_pet_db * pet_db, i2d_string * path) {
    int status = I2D_OK;
    i2d_pet_state * state = NULL;

    if(i2d_pet_state_init(&state, pet_db)) {
        status = i2d_panic("failed to create pet yml state object");
    } else {
        if(i2d_yaml_parse(path, i2d_pet_parse_yml, state))
            status = i2d_panic("failed to parse yml file -- %s", path->string);

        i2d_pet_state_deit(&state);
    }

    return status;
}

static int i2d_pet_parse_yml(yaml_event_t * event, void * context) {
    int status = I2D_OK;

    switch(event->type) {
        case YAML_STREAM_START_EVENT:
        case YAML_DOCUMENT_START_EVENT:
        case YAML_STREAM_END_EVENT:
        case YAML_DOCUMENT_END_EVENT:
        case YAML_ALIAS_EVENT:
        case YAML_SCALAR_EVENT:
        case YAML_SEQUENCE_START_EVENT:
        case YAML_SEQUENCE_END_EVENT:
        case YAML_MAPPING_START_EVENT:
        case YAML_MAPPING_END_EVENT:
        default:
            status = i2d_panic("unsupport event type -- %d", event->type);
            break;
    }

    return status;
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
            if(i2d_pet_init(&object->list)) {
                status = i2d_panic("failed to create pet object");
            } else if(i2d_rbt_init(&object->index, i2d_rbt_cmp_long)) {
                status = i2d_panic("failed to create red black tree object");
            } else {
                if(strstr(path->string, ".txt")) {
                    if(i2d_fd_load(path, i2d_pet_db_parse_txt, object))
                        status = i2d_panic("failed to load pet db -- %s", path->string);
                } else if(strstr(path->string, ".yml")) {
                    if(i2d_pet_db_parse_yml(object, path))
                        status = i2d_panic("failed to load pet db -- %s", path->string);
                } else {
                    status = i2d_panic("unsupport file format -- %s", path->string);
                }
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
    i2d_deit(object->index, i2d_rbt_deit);
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

int i2d_pet_db_search_by_id(i2d_pet_db * pet_db, long id, i2d_pet ** pet) {
    return i2d_rbt_search(pet_db->index, &id, (void **) pet);
}
