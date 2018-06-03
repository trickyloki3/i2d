#include "i2d_constant.h"

static int i2d_constant_db_index(i2d_constant_db *, json_t *, const char *, i2d_rbt **);
static int i2d_constant_db_load_bf(i2d_constant_db *);
static int i2d_constant_db_load_atf(i2d_constant_db *);

int i2d_constant_create(i2d_constant * result, const char * key, json_t * json) {
    int status = I2D_OK;
    json_t * name;
    json_t * min;
    json_t * max;
    json_t * value;

    if(i2d_string_create(&result->macro, key, strlen(key))) {
        status = i2d_panic("failed to copy macro string");
    } else {
        name = json_object_get(json, "name");
        min = json_object_get(json, "min");
        max = json_object_get(json, "max");
        value = json_object_get(json, "value");

        if(name && i2d_object_get_string(name, &result->name)) {
            status = i2d_panic("failed to copy name string");
        } else {
            if(min && max && i2d_object_get_range(min, max, &result->range)) {
                status = i2d_panic("failed to create range");
            } else {
                if(!value || i2d_object_get_number(value, &result->value))
                    status = i2d_panic("failed to get value number");
                if(status)
                    i2d_range_destroy(&result->range);
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

static int i2d_constant_db_load_bf(i2d_constant_db * constant_db) {
    int status = I2D_OK;

    if( i2d_constant_get_by_macro_value(constant_db, "BF_SHORT", &constant_db->bf.BF_SHORT) ||
        i2d_constant_get_by_macro_value(constant_db, "BF_LONG", &constant_db->bf.BF_LONG) ||
        i2d_constant_get_by_macro_value(constant_db, "BF_WEAPON", &constant_db->bf.BF_WEAPON) ||
        i2d_constant_get_by_macro_value(constant_db, "BF_MAGIC", &constant_db->bf.BF_MAGIC) ||
        i2d_constant_get_by_macro_value(constant_db, "BF_MISC", &constant_db->bf.BF_MISC) ||
        i2d_constant_get_by_macro_value(constant_db, "BF_NORMAL", &constant_db->bf.BF_NORMAL) ||
        i2d_constant_get_by_macro_value(constant_db, "BF_SKILL", &constant_db->bf.BF_SKILL) )
        status = i2d_panic("failed to load bf constants");

    return status;
}

static int i2d_constant_db_load_atf(i2d_constant_db * constant_db) {
    int status = I2D_OK;

    if( i2d_constant_get_by_macro_value(constant_db, "ATF_SELF", &constant_db->atf.ATF_SELF) ||
        i2d_constant_get_by_macro_value(constant_db, "ATF_TARGET", &constant_db->atf.ATF_TARGET) ||
        i2d_constant_get_by_macro_value(constant_db, "ATF_SHORT", &constant_db->atf.ATF_SHORT) ||
        i2d_constant_get_by_macro_value(constant_db, "ATF_LONG", &constant_db->atf.ATF_LONG) ||
        i2d_constant_get_by_macro_value(constant_db, "ATF_WEAPON", &constant_db->atf.ATF_WEAPON) ||
        i2d_constant_get_by_macro_value(constant_db, "ATF_MAGIC", &constant_db->atf.ATF_MAGIC) ||
        i2d_constant_get_by_macro_value(constant_db, "ATF_MISC", &constant_db->atf.ATF_MISC) )
        status = i2d_panic("failed to load atf constants");

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
                        i2d_constant_db_index(object, json, "itemgroups", &object->itemgroups) ) {
                        status = i2d_panic("failed to index categories");
                    } else if(i2d_constant_db_load_bf(object)) {
                        status = i2d_panic("failed to load bf constants");
                    } else if(i2d_constant_db_load_atf(object)) {
                        status = i2d_panic("failed to load atf constants");
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
