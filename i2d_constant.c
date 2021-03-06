#include "i2d_constant.h"

static int i2d_constant_db_index(i2d_constant_db *, json_t *, const char *, i2d_rbt **);

int i2d_constant_create(i2d_constant * result, const char * key, json_t * json) {
    int status = I2D_OK;
    json_t * name;
    json_t * range;
    json_t * value;

    if(i2d_string_create(&result->macro, key, strlen(key))) {
        status = i2d_panic("failed to copy macro string");
    } else {
        name = json_object_get(json, "name");
        range = json_object_get(json, "range");
        value = json_object_get(json, "value");

        if(name && i2d_object_get_string(name, &result->name)) {
            status = i2d_panic("failed to copy name string");
        } else {
            if(!value || i2d_object_get_number(value, &result->value)) {
                status = i2d_panic("failed to get value number");
            } else if( range ?
                i2d_object_get_range_array(range, &result->range) :
                i2d_range_create_add(&result->range, result->value, result->value) ) {
                status = i2d_panic("failed to create range");
            }
            if(status)
                i2d_string_destroy(&result->name);
        }
        if(status)
            i2d_string_destroy(&result->macro);
    }

    return status;
}

void i2d_constant_destroy(i2d_constant * result) {
    i2d_range_destroy(&result->range);
    i2d_string_destroy(&result->name);
    i2d_string_destroy(&result->macro);
}

static int i2d_constant_db_index(i2d_constant_db * constant_db, json_t * json, const char * key, i2d_rbt ** result) {
    int status = I2D_OK;
    i2d_rbt * map = NULL;

    json_t * array;
    size_t index;
    json_t * value;

    const char * string;
    size_t length;
    i2d_constant * constant;

    if(i2d_rbt_init(&map, i2d_rbt_cmp_long)) {
        status = i2d_panic("failed to create red black tree object");
    } else {
        array = json_object_get(json, key);
        if(!array) {
            status = i2d_panic("failed to get %s key value", key);
        } else {
            json_array_foreach(array, index, value) {
                string = json_string_value(value);
                if(!string) {
                    status = i2d_panic("invalid string object");
                } else {
                    length = json_string_length(value);
                    if(!length) {
                        status = i2d_panic("empty string object");
                    } else if(i2d_constant_get_by_macro(constant_db, string, &constant)) {
                        status = i2d_panic("failed to find constant -- %s", string);
                    } else if(i2d_rbt_insert(map, &constant->value, constant)) {
                        status = i2d_panic("failed to map constant object");
                    }
                }
            }
        }
        if(status)
            i2d_rbt_deit(&map);
        else
            *result = map;
    }

    return status;
}

int i2d_constant_db_init(i2d_constant_db ** result, json_t * json) {
    int status = I2D_OK;
    i2d_constant_db * object;
    json_t * consts;

    size_t i = 0;
    const char * key;
    json_t * value;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            consts = json_object_get(json, "constants");
            if(!consts) {
                status = i2d_panic("failed to get constants object");
            } else if(i2d_object_get_list(consts, sizeof(*object->constants), (void **) &object->constants, &object->size)) {
                status = i2d_panic("failed to create constants array");
            } else if(i2d_rbt_init(&object->macros, i2d_rbt_cmp_str)) {
                status = i2d_panic("failed to create macro map");
            } else {
                json_object_foreach(consts, key, value) {
                    if(i2d_constant_create(&object->constants[i], key, value)) {
                        status = i2d_panic("failed to create constant object");
                    } else if(i2d_rbt_insert(object->macros, object->constants[i].macro.string, &object->constants[i])) {
                        status = i2d_panic("failed to map constant object");
                    } else {
                        i++;
                    }
                    if(status)
                        break;
                }
                if(!status) {
                    if( i2d_constant_db_index(object, json, "elements", &object->elements) ||
                        i2d_constant_db_index(object, json, "races", &object->races) ||
                        i2d_constant_db_index(object, json, "classes", &object->classes) ||
                        i2d_constant_db_index(object, json, "locations", &object->locations) ||
                        i2d_constant_db_index(object, json, "mapflags", &object->mapflags) ||
                        i2d_constant_db_index(object, json, "gettimes", &object->gettimes) ||
                        i2d_constant_db_index(object, json, "readparam", &object->readparam) ||
                        i2d_constant_db_index(object, json, "sizes", &object->sizes) ||
                        i2d_constant_db_index(object, json, "jobs", &object->jobs) ||
                        i2d_constant_db_index(object, json, "effects", &object->effects) ||
                        i2d_constant_db_index(object, json, "itemgroups", &object->itemgroups) ||
                        i2d_constant_db_index(object, json, "options", &object->options) ||
                        i2d_constant_db_index(object, json, "announces", &object->announces) ||
                        i2d_constant_db_index(object, json, "sc_end", &object->sc_end) ||
                        i2d_constant_db_index(object, json, "sc_start", &object->sc_start) ||
                        i2d_constant_db_index(object, json, "vip_status", &object->vip_status) ) {
                        status = i2d_panic("failed to index categories");
                    } else {
                        if( i2d_constant_get_by_macro(object, "BF_SHORT", &object->BF_SHORT) ||
                            i2d_constant_get_by_macro(object, "BF_LONG", &object->BF_LONG) ||
                            i2d_constant_get_by_macro(object, "BF_WEAPON", &object->BF_WEAPON) ||
                            i2d_constant_get_by_macro(object, "BF_MAGIC", &object->BF_MAGIC) ||
                            i2d_constant_get_by_macro(object, "BF_MISC", &object->BF_MISC) ||
                            i2d_constant_get_by_macro(object, "BF_NORMAL", &object->BF_NORMAL) ||
                            i2d_constant_get_by_macro(object, "BF_SKILL", &object->BF_SKILL) ) {
                            status = i2d_panic("failed to load bf constants");
                        } else {
                            if( i2d_constant_get_by_macro(object, "ATF_SELF", &object->ATF_SELF) ||
                                i2d_constant_get_by_macro(object, "ATF_TARGET", &object->ATF_TARGET) ||
                                i2d_constant_get_by_macro(object, "ATF_SHORT", &object->ATF_SHORT) ||
                                i2d_constant_get_by_macro(object, "ATF_LONG", &object->ATF_LONG) ||
                                i2d_constant_get_by_macro(object, "ATF_WEAPON", &object->ATF_WEAPON) ||
                                i2d_constant_get_by_macro(object, "ATF_MAGIC", &object->ATF_MAGIC) ||
                                i2d_constant_get_by_macro(object, "ATF_MISC", &object->ATF_MISC) )
                                status = i2d_panic("failed to load atf constants");
                        }
                    }
                }
            }

            if(status)
                i2d_constant_db_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_constant_db_deit(i2d_constant_db ** result) {
    i2d_constant_db * object;
    size_t i;

    object = *result;
    i2d_deit(object->mob_races, i2d_rbt_deit);
    i2d_deit(object->vip_status, i2d_rbt_deit);
    i2d_deit(object->sc_start, i2d_rbt_deit);
    i2d_deit(object->sc_end, i2d_rbt_deit);
    i2d_deit(object->announces, i2d_rbt_deit);
    i2d_deit(object->options, i2d_rbt_deit);
    i2d_deit(object->itemgroups, i2d_rbt_deit);
    i2d_deit(object->effects, i2d_rbt_deit);
    i2d_deit(object->jobs, i2d_rbt_deit);
    i2d_deit(object->sizes, i2d_rbt_deit);
    i2d_deit(object->readparam, i2d_rbt_deit);
    i2d_deit(object->gettimes, i2d_rbt_deit);
    i2d_deit(object->mapflags, i2d_rbt_deit);
    i2d_deit(object->locations, i2d_rbt_deit);
    i2d_deit(object->classes, i2d_rbt_deit);
    i2d_deit(object->races, i2d_rbt_deit);
    i2d_deit(object->elements, i2d_rbt_deit);
    i2d_deit(object->macros, i2d_rbt_deit);
    if(object->constants)
        for(i = 0; i < object->size; i++)
            i2d_constant_destroy(&object->constants[i]);
    i2d_free(object->constants);
    i2d_free(object);
    *result = NULL;
}

int i2d_constant_index_mob_races(i2d_constant_db * constant_db, i2d_mob_race_db * mob_race_db) {
    int status = I2D_OK;

    i2d_mob_race * mob_race;
    i2d_constant * constant;

    if(i2d_rbt_init(&constant_db->mob_races, i2d_rbt_cmp_long)) {
        status = i2d_panic("failed to create red black tree object");
    } else if(mob_race_db->list) {
        mob_race = mob_race_db->list;
        do {
            if(i2d_constant_get_by_macro(constant_db, mob_race->macro.string, &constant)) {
                status = i2d_panic("failed to get mob race by macro -- %s", mob_race->macro.string);
            } else if(i2d_rbt_insert(constant_db->mob_races, &constant->value, constant)) {
                status = i2d_panic("failed to map constant object");
            }
            mob_race = mob_race->next;
        } while(mob_race != mob_race_db->list);
    }

    return status;
}

int i2d_constant_get_by_macro_value(i2d_constant_db * constant_db, const char * key, long * result) {
    int status = I2D_OK;
    i2d_constant * constant;

    status = i2d_constant_get_by_macro(constant_db, key, &constant);
    if(!status)
        *result = constant->value;

    return status;
}

int i2d_constant_get_by_macro(i2d_constant_db * constant_db, const char * key, i2d_constant ** result) {
    return i2d_rbt_search(constant_db->macros, key, (void **) result);
}

int i2d_constant_get_by_element(i2d_constant_db * constant_db, const long key, i2d_constant ** result) {
    return i2d_rbt_search(constant_db->elements, &key, (void **) result);
}

int i2d_constant_get_by_race(i2d_constant_db * constant_db, const long key, i2d_constant ** result) {
    return i2d_rbt_search(constant_db->races, &key, (void **) result);
}

int i2d_constant_get_by_class(i2d_constant_db * constant_db, const long key, i2d_constant ** result) {
    return i2d_rbt_search(constant_db->classes, &key, (void **) result);
}

int i2d_constant_get_by_location(i2d_constant_db * constant_db, const long key, i2d_constant ** result) {
    return i2d_rbt_search(constant_db->locations, &key, (void **) result);
}

int i2d_constant_get_by_mapflag(i2d_constant_db * constant_db, const long key, i2d_constant ** result) {
    return i2d_rbt_search(constant_db->mapflags, &key, (void **) result);
}

int i2d_constant_get_by_gettime(i2d_constant_db * constant_db, const long key, i2d_constant ** result) {
    return i2d_rbt_search(constant_db->gettimes, &key, (void **) result);
}

int i2d_constant_get_by_readparam(i2d_constant_db * constant_db, const long key, i2d_constant ** result) {
    return i2d_rbt_search(constant_db->readparam, &key, (void **) result);
}

int i2d_constant_get_by_size(i2d_constant_db * constant_db, const long key, i2d_constant ** result) {
    return i2d_rbt_search(constant_db->sizes, &key, (void **) result);
}

int i2d_constant_get_by_job(i2d_constant_db * constant_db, const long key, i2d_constant ** result) {
    return i2d_rbt_search(constant_db->jobs, &key, (void **) result);
}

int i2d_constant_get_by_effect(i2d_constant_db * constant_db, const long key, i2d_constant ** result) {
    return i2d_rbt_search(constant_db->effects, &key, (void **) result);
}

int i2d_constant_get_by_itemgroups(i2d_constant_db * constant_db, const long key, i2d_constant ** result) {
    return i2d_rbt_search(constant_db->itemgroups, &key, (void **) result);
}

int i2d_constant_get_by_options(i2d_constant_db * constant_db, const long key, i2d_constant ** result) {
    return i2d_rbt_search(constant_db->options, &key, (void **) result);
}

int i2d_constant_get_by_announces(i2d_constant_db * constant_db, const long key, i2d_constant ** result) {
    return i2d_rbt_search(constant_db->announces, &key, (void **) result);
}

int i2d_constant_get_by_sc_end(i2d_constant_db * constant_db, const long key, i2d_constant ** result) {
    return i2d_rbt_search(constant_db->sc_end, &key, (void **) result);
}

int i2d_constant_get_by_sc_start(i2d_constant_db * constant_db, const long key, i2d_constant ** result) {
    return i2d_rbt_search(constant_db->sc_start, &key, (void **) result);
}

int i2d_constant_get_by_vip_status(i2d_constant_db * constant_db, const long key, i2d_constant ** result) {
    return i2d_rbt_search(constant_db->vip_status, &key, (void **) result);
}

int i2d_constant_get_by_mob_races(i2d_constant_db * constant_db, const long key, i2d_constant ** result) {
    return i2d_rbt_search(constant_db->mob_races, &key, (void **) result);
}
