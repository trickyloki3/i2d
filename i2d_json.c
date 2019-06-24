#include "i2d_json.h"

int i2d_json_create(json_t ** json, i2d_string * path) {
    int status = I2D_OK;
    json_error_t error;
    i2d_zero(error);

    *json = json_load_file(path->string, JSON_DISABLE_EOF_CHECK, &error);
    if(!(*json))
        status = i2d_panic("%s (line %d column %d)", error.text, error.line, error.column);

    return status;
}

void i2d_json_destroy(json_t * json) {
    if(json)
        json_decref(json);
}

int i2d_object_get_string_stack(json_t * json, i2d_string_stack * result) {
    int status = I2D_OK;
    size_t size;

    size_t index;
    json_t * value;

    i2d_string string;
    i2d_zero(string);

    size = json_array_size(json);
    if(!size) {
        status = i2d_panic("empty array");
    } else if(i2d_string_stack_create(result, size)) {
        status = I2D_FAIL;
    } else {
        json_array_foreach(json, index, value) {
            if(i2d_object_get_string(value, &string)) {
                status = i2d_panic("failed to get string object");
            } else {
                if(i2d_string_stack_push(result, string.string, string.length))
                    status = i2d_panic("failed to push string stack");

                i2d_free(string.string);
            }
            if(status)
                break;
        }
        if(status)
            i2d_string_stack_destroy(result);
    }

    return status;
}

int i2d_object_get_string(json_t * json, i2d_string * result) {
    int status = I2D_OK;
    const char * string;
    size_t length;

    string = json_string_value(json);
    if(!string) {
        status = i2d_panic("invalid string object");
    } else {
        length = json_string_length(json);
        if(!length) {
            status = i2d_panic("empty string object");
        } else {
            status = i2d_string_create(result, string, length);
        }
    }

    return status;
}

int i2d_object_get_number_array(json_t * json, long ** result, size_t * result_size) {
    int status = I2D_OK;
    size_t size;
    long * list = NULL;

    size_t index;
    json_t * value;

    size = json_array_size(json);
    if(!size) {
        status = i2d_panic("empty array");
    } else {
        list = calloc(size, sizeof(*list));
        if(!list) {
            status = i2d_panic("out of memory");
        } else {
            json_array_foreach(json, index, value) {
                if(i2d_object_get_number(value, &list[index]))
                    status = i2d_panic("failed to get number object");

                if(status)
                    break;
            }

            if(status) {
                free(list);
            } else {
                *result = list;
                *result_size = size;
            }
        }
    }

    return status;
}

int i2d_object_get_number(json_t * json, long * result) {
    int status = I2D_OK;
    json_int_t value;

    if(!json_is_number(json)) {
        status = i2d_panic("invalid number object");
    } else {
        value = json_integer_value(json);
        if(value < LONG_MIN || value > LONG_MAX) {
            status = i2d_panic("integer underflow or overflow");
        } else {
            *result = (long) value;
        }
    }

    return status;
}

int i2d_object_get_range_array(json_t * json, i2d_range * result) {
    int status = I2D_OK;
    size_t index;
    json_t * value;

    json_t * min;
    json_t * max;

    long min_val;
    long max_val;

    if(!json_array_size(json)) {
        status = i2d_panic("empty array");
    } else {
        json_array_foreach(json, index, value) {
            min = json_object_get(value, "min");
            max = json_object_get(value, "max");
            if(i2d_object_get_number(min, &min_val)) {
                status = i2d_panic("failed to get number object");
            } else if(i2d_object_get_number(max, &max_val)) {
                status = i2d_panic("failed to get number object");
            } else {
                if(!index) {
                    if(i2d_range_create_add(result, min_val, max_val))
                        status = i2d_panic("failed to create range object");
                } else {
                    if(i2d_range_add(result, min_val, max_val))
                        status = i2d_panic("failed to add range object");
                }
            }

            if(status)
                break;
        }
    }

    return status;
}

int i2d_object_get_range(json_t * left, json_t * right, i2d_range * result) {
    int status = I2D_OK;
    long min;
    long max;

    if( i2d_object_get_number(left, &min) ||
        i2d_object_get_number(right, &max) ) {
        status = i2d_panic("failed to get min or max");
    } else if(i2d_range_create_add(result, min, max)) {
        status = i2d_panic("failed to create range object");
    }

    return status;
}

int i2d_object_get_list(json_t * json, size_t element, void ** result_list, size_t * result_size) {
    int status = I2D_OK;
    void * list;
    size_t size;

    size = json_object_size(json);
    if(!size) {
        status = i2d_panic("empty object");
    } else {
        list = calloc(size, element);
        if(!list) {
            status = i2d_panic("out of memory");
        } else {
            *result_list = list;
            *result_size = size;
        }
    }

    return status;
}

int i2d_object_get_boolean(json_t * json, int * result) {
    int status = I2D_OK;

    if(!json_is_boolean(json)) {
        status = i2d_panic("invalid boolean object");
    } else {
        *result = json_is_true(json) ? 1 : 0;
    }

    return status;
}

int i2d_config_init(i2d_config ** result, i2d_string * path) {
    int status = I2D_OK;
    i2d_config * object;
    json_t * config;
    json_t * item_id;
    json_t * arguments_path;
    json_t * bonus_path;
    json_t * constants_path;
    json_t * data_path;
    json_t * functions_path;
    json_t * print_path;
    json_t * sc_start_path;
    json_t * statements_path;
    json_t * item_db_path;
    json_t * skill_db_path;
    json_t * mob_db_path;
    json_t * mob_race2_db_path;
    json_t * produce_db_path;
    json_t * mercenary_db_path;
    json_t * pet_db_path;
    json_t * item_combo_db_path;

    if(i2d_is_invalid(result) || !path) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_json_create(&config, path)) {
                status = i2d_panic("failed to load -- %s", path->string);
            } else {
                item_id = json_object_get(config, "item_id");
                arguments_path = json_object_get(config, "arguments_path");
                bonus_path = json_object_get(config, "bonus_path");
                constants_path = json_object_get(config, "constants_path");
                data_path = json_object_get(config, "data_path");
                functions_path = json_object_get(config, "functions_path");
                print_path = json_object_get(config, "print_path");
                sc_start_path = json_object_get(config, "sc_start_path");
                statements_path = json_object_get(config, "statements_path");
                item_db_path = json_object_get(config, "item_db_path");
                skill_db_path = json_object_get(config, "skill_db_path");
                mob_db_path = json_object_get(config, "mob_db_path");
                mob_race2_db_path = json_object_get(config, "mob_race2_db_path");
                produce_db_path = json_object_get(config, "produce_db_path");
                mercenary_db_path = json_object_get(config, "mercenary_db_path");
                pet_db_path = json_object_get(config, "pet_db_path");
                item_combo_db_path = json_object_get(config, "item_combo_db_path");
                if(item_id && i2d_object_get_number(item_id, &object->item_id)) {
                    status = i2d_panic("failed to get item id");
                } else if(!arguments_path || i2d_object_get_string(arguments_path, &object->arguments_path)) {
                    status = i2d_panic("failed to get arguments path");
                } else if(!bonus_path || i2d_object_get_string(bonus_path, &object->bonus_path)) {
                    status = i2d_panic("failed to get bonus path");
                } else if(!constants_path || i2d_object_get_string(constants_path, &object->constants_path)) {
                    status = i2d_panic("failed to get constants path");
                } else if(!data_path || i2d_object_get_string(data_path, &object->data_path)) {
                    status = i2d_panic("failed to get data path");
                } else if(!functions_path || i2d_object_get_string(functions_path, &object->functions_path)) {
                    status = i2d_panic("failed to get functions path");
                } else if(!print_path || i2d_object_get_string(print_path, &object->print_path)) {
                    status = i2d_panic("failed to get print path");
                } else if(!sc_start_path || i2d_object_get_string(sc_start_path, &object->sc_start_path)) {
                    status = i2d_panic("failed to get sc_start path");
                } else if(!statements_path || i2d_object_get_string(statements_path, &object->statements_path)) {
                    status = i2d_panic("failed to get statements path");
                } else if(!item_db_path || i2d_object_get_string(item_db_path, &object->item_db_path)) {
                    status = i2d_panic("failed to get item db path");
                } else if(!skill_db_path || i2d_object_get_string(skill_db_path, &object->skill_db_path)) {
                    status = i2d_panic("failed to get skill db path");
                } else if(!mob_db_path || i2d_object_get_string(mob_db_path, &object->mob_db_path)) {
                    status = i2d_panic("failed to get mob db path");
                } else if(!mob_race2_db_path || i2d_object_get_string(mob_race2_db_path, &object->mob_race2_db_path)) {
                    status = i2d_panic("failed to get mob race2 db path");
                } else if(!produce_db_path || i2d_object_get_string(produce_db_path, &object->produce_db_path)) {
                    status = i2d_panic("failed to get produce db path");
                } else if(!mercenary_db_path || i2d_object_get_string(mercenary_db_path, &object->mercenary_db_path)) {
                    status = i2d_panic("failed to get mercenary db path");
                } else if(!pet_db_path || i2d_object_get_string(pet_db_path, &object->pet_db_path)) {
                    status = i2d_panic("failed to get pet db path");
                } else if(!item_combo_db_path || i2d_object_get_string(item_combo_db_path, &object->item_combo_db_path)) {
                    status = i2d_panic("failed to get item combo db path");
                }
                i2d_json_destroy(config);
            }

            if(status)
                i2d_config_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_config_deit(i2d_config ** result) {
    i2d_config * object;

    object = *result;
    i2d_string_destroy(&object->item_combo_db_path);
    i2d_string_destroy(&object->pet_db_path);
    i2d_string_destroy(&object->mercenary_db_path);
    i2d_string_destroy(&object->produce_db_path);
    i2d_string_destroy(&object->mob_race2_db_path);
    i2d_string_destroy(&object->mob_db_path);
    i2d_string_destroy(&object->skill_db_path);
    i2d_string_destroy(&object->item_db_path);
    i2d_string_destroy(&object->statements_path);
    i2d_string_destroy(&object->sc_start_path);
    i2d_string_destroy(&object->print_path);
    i2d_string_destroy(&object->functions_path);
    i2d_string_destroy(&object->data_path);
    i2d_string_destroy(&object->constants_path);
    i2d_string_destroy(&object->bonus_path);
    i2d_string_destroy(&object->arguments_path);
    i2d_free(object);
    *result = NULL;
}

int i2d_json_init(i2d_json ** result, i2d_config * config) {
    int status = I2D_OK;
    i2d_json * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_json_create(&object->statements, &config->statements_path)) {
                status = i2d_panic("failed to load json file -- %s", &config->statements_path);
            } else if(i2d_json_create(&object->functions, &config->functions_path)) {
                status = i2d_panic("failed to load json file -- %s", &config->functions_path);
            } else if(i2d_json_create(&object->constants, &config->constants_path)) {
                status = i2d_panic("failed to load json file -- %s", &config->constants_path);
            } else if(i2d_json_create(&object->arguments, &config->arguments_path)) {
                status = i2d_panic("failed to load json file -- %s", &config->arguments_path);
            } else if(i2d_json_create(&object->bonus_file, &config->bonus_path)) {
                status = i2d_panic("failed to load json file -- %s", &config->bonus_path);
            } else if(i2d_json_create(&object->sc_start_file, &config->sc_start_path)) {
                status = i2d_panic("failed to load json file -- %s", &config->sc_start_path);
            } else if(i2d_json_create(&object->data_file, &config->data_path)) {
                status = i2d_panic("failed to load json file -- %s", &config->data_path);
            } else if(i2d_json_create(&object->print_file, &config->print_path)) {
                status = i2d_panic("failed to load json file -- %s", &config->print_path);
            } else {
                object->bonus = json_object_get(object->bonus_file, "bonus");
                object->bonus2 = json_object_get(object->bonus_file, "bonus2");
                object->bonus3 = json_object_get(object->bonus_file, "bonus3");
                object->bonus4 = json_object_get(object->bonus_file, "bonus4");
                object->bonus5 = json_object_get(object->bonus_file, "bonus5");
                object->ammo_type = json_object_get(object->data_file, "ammo_type");
                object->bonus_script_flag = json_object_get(object->data_file, "bonus_script_flag");
                object->getiteminfo_type = json_object_get(object->data_file, "getiteminfo_type");
                object->searchstore_effect = json_object_get(object->data_file, "searchstore_effect");
                object->skill_flag = json_object_get(object->data_file, "skill_flag");
                object->strcharinfo_type = json_object_get(object->data_file, "strcharinfo_type");
                object->weapon_type = json_object_get(object->data_file, "weapon_type");
                object->item_type = json_object_get(object->data_file, "item_type");
                object->item_location = json_object_get(object->data_file, "item_location");
                object->job = json_object_get(object->data_file, "job");
                object->job_group = json_object_get(object->data_file, "job_group");
                object->class = json_object_get(object->data_file, "class");
                object->class_group = json_object_get(object->data_file, "class_group");
                object->gender = json_object_get(object->data_file, "gender");
                object->refineable = json_object_get(object->data_file, "refineable");
                object->basejob = json_object_get(object->data_file, "basejob");
                object->sc_start = json_object_get(object->sc_start_file, "sc_start");
                object->sc_start2 = json_object_get(object->sc_start_file, "sc_start2");
                object->sc_start4 = json_object_get(object->sc_start_file, "sc_start4");
                object->description_by_item_type = json_object_get(object->print_file, "description_by_item_type");
                object->description_of_item_property = json_object_get(object->print_file, "description_of_item_property");
            }

            if(status)
                i2d_json_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_json_deit(i2d_json ** result) {
    i2d_json * object;

    object = *result;
    i2d_json_destroy(object->print_file);
    i2d_json_destroy(object->data_file);
    i2d_json_destroy(object->sc_start_file);
    i2d_json_destroy(object->bonus_file);
    i2d_json_destroy(object->arguments);
    i2d_json_destroy(object->constants);
    i2d_json_destroy(object->functions);
    i2d_json_destroy(object->statements);
    i2d_free(object);
    *result = NULL;
}
