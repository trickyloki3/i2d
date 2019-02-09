#include "i2d_data.h"

int i2d_data_create(i2d_data * result, const char * key, json_t * json, i2d_constant_db * constant_db) {
    int status = I2D_OK;
    json_t * min;
    json_t * max;
    json_t * description;
    json_t * handler;
    json_t * argument_type;
    json_t * argument_default;
    json_t * argument_order;
    json_t * required;
    json_t * optional;
    json_t * positive;
    json_t * negative;
    json_t * zero;
    json_t * empty_description_on_zero;
    json_t * empty_description_on_empty_string;
    json_t * dump_stack_instead_of_description;

    min = json_object_get(json, "min");
    max = json_object_get(json, "max");
    description = json_object_get(json, "description");
    handler = json_object_get(json, "handler");
    argument_type = json_object_get(json, "argument_type");
    argument_default = json_object_get(json, "argument_default");
    argument_order = json_object_get(json, "argument_order");
    required = json_object_get(json, "required");
    optional = json_object_get(json, "optional");
    positive = json_object_get(json, "positive");
    negative = json_object_get(json, "negative");
    zero = json_object_get(json, "zero");
    empty_description_on_zero = json_object_get(json, "empty_description_on_zero");
    empty_description_on_empty_string = json_object_get(json, "empty_description_on_empty_string");
    dump_stack_instead_of_description = json_object_get(json, "dump_stack_instead_of_description");
    if(constant_db)
        i2d_constant_get_by_macro_value(constant_db, key, &result->constant);

    if(i2d_string_create(&result->name, key, strlen(key))) {
        status = i2d_panic("failed to copy name string");
    } else if(min && max && i2d_object_get_range(min, max, &result->range)) {
        status = i2d_panic("failed to create range");
    } else if(description && i2d_object_get_string(description, &result->description)) {
        status = i2d_panic("failed to create string");
    } else if(handler && i2d_object_get_string(handler, &result->handler)) {
        status = i2d_panic("failed to create string");
    } else if(argument_type && i2d_object_get_string_stack(argument_type, &result->argument_type)) {
        status = i2d_panic("failed to create string stack");
    } else if(argument_default && i2d_object_get_string_stack(argument_default, &result->argument_default)) {
        status = i2d_panic("failed to create string stack");
    } else if(argument_order && i2d_object_get_number_array(argument_order, &result->argument_order.list, &result->argument_order.size)) {
        status = i2d_panic("failed to create number array");
    } else if(required && i2d_object_get_number(required, &result->required)) {
        status = i2d_panic("failed to create number");
    } else if(optional && i2d_object_get_number(optional, &result->optional)) {
        status = i2d_panic("failed to create number");
    } else if(positive && i2d_object_get_string(positive, &result->positive)) {
        status = i2d_panic("failed to create string");
    } else if(negative && i2d_object_get_string(negative, &result->negative)) {
        status = i2d_panic("failed to create string");
    } else if(zero && i2d_object_get_string(zero, &result->zero)) {
        status = i2d_panic("failed to create string");
    } else if(empty_description_on_zero && i2d_object_get_boolean(empty_description_on_zero, &result->empty_description_on_zero)) {
        status = i2d_panic("failed to create boolean");
    } else if(empty_description_on_empty_string && i2d_object_get_boolean(empty_description_on_empty_string, &result->empty_description_on_empty_string)) {
        status = i2d_panic("failed to create boolean");
    } else if(dump_stack_instead_of_description && i2d_object_get_boolean(dump_stack_instead_of_description, &result->dump_stack_instead_of_description)) {
        status = i2d_panic("failed to create boolean");
    }

    return status;
}

void i2d_data_destroy(i2d_data * result) {
    i2d_string_destroy(&result->zero);
    i2d_string_destroy(&result->negative);
    i2d_string_destroy(&result->positive);
    i2d_free(result->argument_order.list);
    i2d_string_stack_destroy(&result->argument_default);
    i2d_string_stack_destroy(&result->argument_type);
    i2d_string_destroy(&result->handler);
    i2d_string_destroy(&result->description);
    i2d_range_destroy(&result->range);
    i2d_string_destroy(&result->name);
}

int i2d_data_map_init(i2d_data_map ** result, enum i2d_data_map_type type, json_t * json, i2d_constant_db * constant_db) {
    int status = I2D_OK;
    i2d_data_map * object;
    i2d_rbt_cmp cmp = NULL;

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
            switch(type) {
                case data_map_by_constant:
                    cmp = i2d_rbt_cmp_long;
                    break;
                case data_map_by_name:
                    cmp = i2d_rbt_cmp_str;
                    break;
                default:
                    status = i2d_panic("invalid data map type");
                    break;
            }

            if(!status) {
                if(i2d_rbt_init(&object->map, cmp)) {
                    status = i2d_panic("failed to create red black tree object");
                } else if(i2d_object_get_list(json, sizeof(*object->list), (void **) &object->list, &object->size)) {
                    status = i2d_panic("failed to create data array");
                } else {
                    json_object_foreach(json, key, value) {
                        if(i2d_data_create(&object->list[i], key, value, constant_db)) {
                            status = i2d_panic("failed to create data object");
                        } else {
                            switch(type) {
                                case data_map_by_constant:
                                    if(i2d_rbt_insert(object->map, &object->list[i].constant, &object->list[i]))
                                        status = i2d_panic("failed to map data object");
                                    break;
                                case data_map_by_name:
                                    if(i2d_rbt_insert(object->map, object->list[i].name.string, &object->list[i]))
                                        status = i2d_panic("failed to map data object");
                                    break;
                                default:
                                    status = i2d_panic("invalid data map type");
                                    break;
                            }
                            i++;
                        }
                    }
                }
            }

            if(status)
                i2d_data_map_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_data_map_deit(i2d_data_map ** result) {
    i2d_data_map * object;
    size_t i;

    object = *result;
    if(object->list) {
        for(i = 0; i < object->size; i++)
            i2d_data_destroy(&object->list[i]);
        i2d_free(object->list);
    }
    i2d_deit(object->map, i2d_rbt_deit);
    i2d_free(object);
    *result = NULL;
}

int i2d_data_map_get(i2d_data_map * data_map, void * key, i2d_data ** result) {
    return i2d_rbt_search(data_map->map, key, (void **) result);
}

int i2d_value_map_init(i2d_value_map ** result, json_t * json, enum i2d_value_type type) {
    int status = I2D_OK;
    i2d_value_map * object;

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
            object->type = type;
            if(i2d_rbt_init(&object->map, i2d_rbt_cmp_long)) {
                status = i2d_panic("failed to create value map");
            } else if(i2d_object_get_list(json, sizeof(*object->list), (void **) &object->list, &object->size)) {
                status = i2d_panic("failed to create value array");
            } else {
                json_object_foreach(json, key, value) {
                    if(i2d_strtoll(&object->list[i].value, key, strlen(key), 10)) {
                        status = i2d_panic("failed to convert value string");
                    } else {
                        switch(object->type) {
                            case i2d_value_string:
                                if(i2d_object_get_string(value, &object->list[i].string))
                                    status = i2d_panic("failed to copy string");
                                break;
                            case i2d_value_string_stack:
                                if(i2d_object_get_string_stack(value, &object->list[i].stack))
                                    status = i2d_panic("failed to copy string stack");
                                break;
                            default:
                                status = i2d_panic("invalid value type -- %d", object->type);
                                break;
                        }
                    }

                    if(status) {
                        break;
                    } else {
                        if(i2d_rbt_insert(object->map, &object->list[i].value, &object->list[i])) {
                            status = i2d_panic("failed to map value object");
                        } else {
                            i++;
                        }
                    }
                }
            }

            if(status)
                i2d_value_map_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_value_map_deit(i2d_value_map ** result) {
    i2d_value_map * object;
    size_t i;

    object = *result;
    if(object->list) {
        for(i = 0; i < object->size; i++) {
            switch(object->type) {
                case i2d_value_string:
                    i2d_string_destroy(&object->list[i].string);
                    break;
                case i2d_value_string_stack:
                    i2d_string_stack_destroy(&object->list[i].stack);
                    break;
            }
        }
        i2d_free(object->list);
    }
    i2d_deit(object->map, i2d_rbt_deit);
    i2d_free(object);
    *result = NULL;
}

int i2d_value_map_get_string(i2d_value_map * value_map, long long key, i2d_string * result) {
    int status = I2D_OK;
    i2d_value * value;

    if(!i2d_rbt_search(value_map->map, &key, (void **) &value))
        *result = value->string;

    return status;
}

int i2d_value_map_get_string_stack(i2d_value_map * value_map, long long key, i2d_string_stack * result) {
    int status = I2D_OK;
    i2d_value * value;

    if(!i2d_rbt_search(value_map->map, &key, (void **) &value))
        *result = value->stack;

    return status;
}
