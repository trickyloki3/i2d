#include "i2d_print.h"

enum i2d_item_property_type {
    I2D_ITEM_ID,
    I2D_ITEM_AEGIS_NAME,
    I2D_ITEM_NAME,
    I2D_ITEM_TYPE,
    I2D_ITEM_BUY,
    I2D_ITEM_SELL,
    I2D_ITEM_WEIGHT,
    I2D_ITEM_ATK,
    I2D_ITEM_MATK,
    I2D_ITEM_DEF,
    I2D_ITEM_RANGE,
    I2D_ITEM_SLOTS,
    I2D_ITEM_JOB,
    I2D_ITEM_UPPER,
    I2D_ITEM_GENDER,
    I2D_ITEM_LOCATION,
    I2D_ITEM_WEAPON_LEVEL,
    I2D_ITEM_BASE_LEVEL,
    I2D_ITEM_MAX_LEVEL,
    I2D_ITEM_REFINEABLE,
    I2D_ITEM_VIEW,
    I2D_ITEM_SCRIPT,
    I2D_ITEM_ONEQUIP_SCRIPT,
    I2D_ITEM_ONUNEQUIP_SCRIPT,
    I2D_ITEM_COMBO_SCRIPT
};

struct i2d_item_property {
    enum i2d_item_property_type type;
    char * name;
};

typedef struct i2d_item_property i2d_item_property;

i2d_item_property item_properties[] = {
    { I2D_ITEM_ID, "id" },
    { I2D_ITEM_AEGIS_NAME, "aegis_name" },
    { I2D_ITEM_NAME, "name" },
    { I2D_ITEM_TYPE, "type" },
    { I2D_ITEM_BUY, "buy" },
    { I2D_ITEM_SELL, "sell" },
    { I2D_ITEM_WEIGHT, "weight" },
    { I2D_ITEM_ATK, "atk" },
    { I2D_ITEM_MATK, "matk" },
    { I2D_ITEM_DEF, "def" },
    { I2D_ITEM_RANGE, "range" },
    { I2D_ITEM_SLOTS, "slots" },
    { I2D_ITEM_JOB, "job" },
    { I2D_ITEM_UPPER, "upper" },
    { I2D_ITEM_GENDER, "gender" },
    { I2D_ITEM_LOCATION, "location" },
    { I2D_ITEM_WEAPON_LEVEL, "weapon_level" },
    { I2D_ITEM_BASE_LEVEL, "base_level" },
    { I2D_ITEM_MAX_LEVEL, "max_level" },
    { I2D_ITEM_REFINEABLE, "refineable" },
    { I2D_ITEM_VIEW, "view" },
    { I2D_ITEM_SCRIPT, "script" },
    { I2D_ITEM_ONEQUIP_SCRIPT, "onequip_script" },
    { I2D_ITEM_ONUNEQUIP_SCRIPT, "onunequip_script" },
    { I2D_ITEM_COMBO_SCRIPT, "combo_script" }
};

static int i2d_print_get_property_integer(i2d_print *, const char *, i2d_item *, long *);
static int i2d_print_get_property_string(i2d_print *, const char *, i2d_item *, i2d_string *);

typedef int (* i2d_handler_print_cb)(i2d_print *, i2d_data *, i2d_item *, i2d_string_stack *);

struct i2d_handler {
    char * name;
    i2d_handler_print_cb handler;
};

typedef struct i2d_handler i2d_handler;

static int i2d_handler_general(i2d_print *, i2d_data *, i2d_string *, i2d_string_stack *);
static int i2d_handler_integer(i2d_print *, i2d_data *, i2d_item *, i2d_string_stack *);
static int i2d_handler_string(i2d_print *, i2d_data *, i2d_item *, i2d_string_stack *);
static int i2d_handler_type(i2d_print *, i2d_data *, i2d_item *, i2d_string_stack *);
static int i2d_handler_job(i2d_print *, i2d_data *, i2d_item *, i2d_string_stack *);
static int i2d_handler_class(i2d_print *, i2d_data *, i2d_item *, i2d_string_stack *);
static int i2d_handler_gender(i2d_print *, i2d_data *, i2d_item *, i2d_string_stack *);
static int i2d_handler_location(i2d_print *, i2d_data *, i2d_item *, i2d_string_stack *);
static int i2d_handler_refine(i2d_print *, i2d_data *, i2d_item *, i2d_string_stack *);
static int i2d_handler_view(i2d_print *, i2d_data *, i2d_item *, i2d_string_stack *);

i2d_handler print_handlers[] = {
    { "integer", i2d_handler_integer },
    { "string", i2d_handler_string },
    { "type", i2d_handler_type },
    { "job", i2d_handler_job },
    { "class", i2d_handler_class },
    { "gender", i2d_handler_gender },
    { "location", i2d_handler_location },
    { "refine", i2d_handler_refine },
    { "view", i2d_handler_view }
};

int i2d_print_init(i2d_print ** result, i2d_json * json) {
    int status = I2D_OK;
    i2d_print * object;

    size_t i;
    size_t size;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_value_map_init(&object->description_by_item_type, json->description_by_item_type, i2d_value_string_stack)) {
                status = i2d_panic("failed to load description_by_item_type ");
            } else if(i2d_data_map_init(&object->description_of_item_property, data_map_by_name, json->description_of_item_property, NULL)) {
                status = i2d_panic("failed to load description_of_item_property");
            } else if(i2d_rbt_init(&object->print_handlers, i2d_rbt_cmp_str)) {
                status = i2d_panic("failed to create read black tree object");
            } else if(i2d_rbt_init(&object->item_properties, i2d_rbt_cmp_str)) {
                status = i2d_panic("failed to create read black tree object");
            } else if(i2d_buffer_cache_init(&object->buffer_cache)) {
                status = i2d_panic("failed to create buffer cache object");
            } else if(i2d_string_stack_cache_init(&object->stack_cache)) {
                status = i2d_panic("failed to create string stack cache object");
            } else if(i2d_value_map_init(&object->item_type, json->item_type, i2d_value_string)) {
                status = i2d_panic("failed to load item_type");
            } else if(i2d_value_map_init(&object->item_location, json->item_location, i2d_value_string)) {
                status = i2d_panic("failed to load item_location");
            } else if(i2d_value_map_init(&object->ammo_type, json->ammo_type, i2d_value_string)) {
                status = i2d_panic("failed to load ammo_type");
            } else if(i2d_value_map_init(&object->weapon_type, json->weapon_type, i2d_value_string)) {
                status = i2d_panic("failed to load weapon_type");
            } else {
                size = i2d_size(print_handlers);
                for(i = 0; i < size && !status; i++)
                    if(i2d_rbt_insert(object->print_handlers, print_handlers[i].name, &print_handlers[i]))
                        status = i2d_panic("failed to map handler object");

                size = i2d_size(item_properties);
                for(i = 0; i < size && !status; i++)
                    if(i2d_rbt_insert(object->item_properties, item_properties[i].name, &item_properties[i]))
                        status = i2d_panic("failed to map item property object");
            }

            if(status) {
                i2d_print_deit(&object);
            } else {
                *result = object;
            }
        }
    }

    return status;
}

void i2d_print_deit(i2d_print ** result) {
    i2d_print * object;

    object = *result;
    i2d_deit(object->weapon_type, i2d_value_map_deit);
    i2d_deit(object->ammo_type, i2d_value_map_deit);
    i2d_deit(object->item_location, i2d_value_map_deit);
    i2d_deit(object->item_type, i2d_value_map_deit);
    i2d_deit(object->stack_cache, i2d_string_stack_cache_deit);
    i2d_deit(object->buffer_cache, i2d_buffer_cache_deit);
    i2d_deit(object->item_properties, i2d_rbt_deit);
    i2d_deit(object->print_handlers, i2d_rbt_deit);
    i2d_deit(object->description_of_item_property, i2d_data_map_deit);
    i2d_deit(object->description_by_item_type, i2d_value_map_deit);
    i2d_free(object);
    *result = NULL;
}

int i2d_print_format(i2d_print * print, i2d_item * item) {
    int status = I2D_OK;
    i2d_buffer * buffer = NULL;
    i2d_string_stack * stack = NULL;
    i2d_string_stack properties;
    i2d_string * list;
    size_t size;
    size_t i;
    i2d_data * data;
    i2d_handler * handler;
    i2d_string description;

    if(i2d_buffer_cache_get(print->buffer_cache, &buffer)) {
        status = i2d_panic("failed to create buffer object");
    } else {
        if(i2d_string_stack_cache_get(print->stack_cache, &stack)) {
            status = i2d_panic("failed to create string stack object");
        } else {
            if(i2d_value_map_get_string_stack(print->description_by_item_type, item->type, &properties)) {
                status = i2d_panic("failed to get item properties by item type -- %ld", item->type);
            } else if(i2d_string_stack_get(&properties, &list, &size)) {
                status = i2d_panic("failed to get item properties from stack");
            } else {
                for(i = 0; i < size && !status; i++) {
                    if(i2d_data_map_get(print->description_of_item_property, list[i].string, &data)) {
                        status = i2d_panic("failed to get item property by name -- %s", list[i].string);
                    } else if(i2d_rbt_search(print->print_handlers, data->handler.string, (void **) &handler)) {
                        status = i2d_panic("failed to get handler by name -- %s", data->handler.string);
                    } else if(handler->handler(print, data, item, stack)) {
                        status = I2D_FAIL;
                    }
                }
                if(i2d_string_stack_dump_buffer(stack, buffer, "\n")) {
                    status = i2d_panic("failed to dump string stack to buffer");
                } else {
                    i2d_buffer_get(buffer, &description.string, &description.length);
                    if(i2d_print_format_lua(print, item, &description))
                        status = i2d_panic("failed to write item in lua format -- %ld", item->id);
                }
            }
            i2d_string_stack_cache_put(print->stack_cache, &stack);
        }
        i2d_buffer_cache_put(print->buffer_cache, &buffer);
    }

    return status;
}

int i2d_print_format_lua(i2d_print * print, i2d_item * item, i2d_string * description) {
    int status = I2D_OK;

    fprintf(stdout, "%s\n", description->string);

    return status;
}

static int i2d_print_get_property_integer(i2d_print * print, const char * property, i2d_item * item, long * result) {
    int status = I2D_OK;
    i2d_item_property * item_property = NULL;

    if(i2d_rbt_search(print->item_properties, property, (void **) &item_property)) {
        status = i2d_panic("failed to get item property by string -- %s", property);
    } else {
        switch(item_property->type) {
            case I2D_ITEM_ID: *result = item->id; break;
            case I2D_ITEM_TYPE: *result = item->type; break;
            case I2D_ITEM_BUY: *result = item->buy; break;
            case I2D_ITEM_SELL: *result = item->sell; break;
            case I2D_ITEM_WEIGHT: *result = item->weight; break;
            case I2D_ITEM_ATK: *result = item->atk; break;
            case I2D_ITEM_MATK: *result = item->matk; break;
            case I2D_ITEM_DEF: *result = item->def; break;
            case I2D_ITEM_RANGE: *result = item->range; break;
            case I2D_ITEM_SLOTS: *result = item->slots; break;
            case I2D_ITEM_JOB: *result = item->job; break;
            case I2D_ITEM_UPPER: *result = item->upper; break;
            case I2D_ITEM_GENDER: *result = item->gender; break;
            case I2D_ITEM_LOCATION: *result = item->location; break;
            case I2D_ITEM_WEAPON_LEVEL: *result = item->weapon_level; break;
            case I2D_ITEM_BASE_LEVEL: *result = item->base_level; break;
            case I2D_ITEM_MAX_LEVEL: *result = item->max_level; break;
            case I2D_ITEM_REFINEABLE: *result = item->refineable; break;
            case I2D_ITEM_VIEW: *result = item->view; break;
            default:
                status = i2d_panic("failed item property type -- %d", item_property->type);
        }
    }

    return status;
}

static int i2d_print_get_property_string(i2d_print * print, const char * property, i2d_item * item, i2d_string * result) {
    int status = I2D_OK;
    i2d_item_property * item_property = NULL;

    if(i2d_rbt_search(print->item_properties, property, (void **) &item_property)) {
        status = i2d_panic("failed to get item property by string -- %s", property);
    } else {
        switch(item_property->type) {
            case I2D_ITEM_AEGIS_NAME: *result = item->aegis_name; break;
            case I2D_ITEM_NAME: *result = item->name; break;
            case I2D_ITEM_SCRIPT: *result = item->script_description; break;
            case I2D_ITEM_ONEQUIP_SCRIPT: *result = item->onequip_script_description; break;
            case I2D_ITEM_ONUNEQUIP_SCRIPT: *result = item->onunequip_script_description; break;
            case I2D_ITEM_COMBO_SCRIPT: *result = item->combo_description; break;
            default:
                status = i2d_panic("failed item property type -- %d", item_property->type);
        }
    }

    return status;
}

static int i2d_handler_general(i2d_print * print, i2d_data * data, i2d_string * string, i2d_string_stack * stack) {
    int status = I2D_OK;
    i2d_string_stack * format = NULL;
    i2d_buffer * description = NULL;

    if(string->length > 0) {
        if(i2d_string_stack_cache_get(print->stack_cache, &format)) {
            status = i2d_panic("failed to create string stack object");
        } else {
            if(i2d_string_stack_push(format, string->string, string->length)) {
                status = i2d_panic("failed to push buffer on stack");
            } else if(i2d_buffer_cache_get(print->buffer_cache, &description)) {
                status = i2d_panic("failed to create buffer object");
            } else {
                if(i2d_string_stack_format(format, &data->description, description)) {
                    status = i2d_panic("failed to format string stack");
                } else if(i2d_string_stack_push_buffer(stack, description)) {
                    status = i2d_panic("failed to push buffer on stack");
                }
                i2d_buffer_cache_put(print->buffer_cache, &description);
            }
            i2d_string_stack_cache_put(print->stack_cache, &format);
        }
    }

    return status;
}

static int i2d_handler_integer(i2d_print * print, i2d_data * data, i2d_item * item, i2d_string_stack * stack) {
    int status = I2D_OK;
    long integer;
    i2d_buffer * buffer = NULL;
    i2d_string string;
    i2d_zero(string);

    if(i2d_print_get_property_integer(print, data->name.string, item, &integer)) {
        status = i2d_panic("failed to get integer by name -- %s", data->name.string);
    } else if(data->empty_description_on_zero && !integer) {
        /* empty description on zero */
    } else {
        if(i2d_buffer_cache_get(print->buffer_cache, &buffer)) {
            status = i2d_panic("failed to create buffer object");
        } else {
            if(i2d_buffer_printf(buffer, "%ld", integer)) {
                status = i2d_panic("failed to write buffer object");
            } else {
                i2d_buffer_get(buffer, &string.string, &string.length);
                status = i2d_handler_general(print, data, &string, stack);
            }
            i2d_buffer_cache_put(print->buffer_cache, &buffer);
        }
    }

    return status;
}

static int i2d_handler_string(i2d_print * print, i2d_data * data, i2d_item * item, i2d_string_stack * stack) {
    int status = I2D_OK;
    i2d_string string;
    i2d_zero(string);

    if(i2d_print_get_property_string(print, data->name.string, item, &string)) {
        status = i2d_panic("failed to get string by name -- %s", data->name.string);
    } else {
        status = i2d_handler_general(print, data, &string, stack);
    }

    return status;
}

static int i2d_handler_type(i2d_print * print, i2d_data * data, i2d_item * item, i2d_string_stack * stack) {
    int status = I2D_OK;
    long integer;
    i2d_string string;
    i2d_zero(string);

    if(i2d_print_get_property_integer(print, data->name.string, item, &integer)) {
        status = i2d_panic("failed to get integer by name -- %s", data->name.string);
    } else if(i2d_value_map_get_string(print->item_type, integer, &string)) {
        status = i2d_panic("failed to get item type by integer -- %ld", integer);
    } else {
        status = i2d_handler_general(print, data, &string, stack);
    }

    return status;
}

static int i2d_handler_job(i2d_print * print, i2d_data * data, i2d_item * item, i2d_string_stack * stack) {
    int status = I2D_OK;
    return status;
}

static int i2d_handler_class(i2d_print * print, i2d_data * data, i2d_item * item, i2d_string_stack * stack) {
    int status = I2D_OK;
    return status;
}

static int i2d_handler_gender(i2d_print * print, i2d_data * data, i2d_item * item, i2d_string_stack * stack) {
    int status = I2D_OK;
    return status;
}

static int i2d_handler_location(i2d_print * print, i2d_data * data, i2d_item * item, i2d_string_stack * stack) {
    int status = I2D_OK;
    long integer;
    i2d_string string;
    i2d_zero(string);

    if(i2d_print_get_property_integer(print, data->name.string, item, &integer)) {
        status = i2d_panic("failed to get integer by name -- %s", data->name.string);
    } else if(i2d_value_map_get_string(print->item_location, integer, &string)) {
        status = i2d_panic("failed to get item location by integer -- %ld", integer);
    } else {
        status = i2d_handler_general(print, data, &string, stack);
    }

    return status;
}

static int i2d_handler_refine(i2d_print * print, i2d_data * data, i2d_item * item, i2d_string_stack * stack) {
    int status = I2D_OK;
    return status;
}

static int i2d_handler_view(i2d_print * print, i2d_data * data, i2d_item * item, i2d_string_stack * stack) {
    int status = I2D_OK;
    long integer;
    i2d_string string;
    i2d_zero(string);

    if(i2d_print_get_property_integer(print, data->name.string, item, &integer)) {
        status = i2d_panic("failed to get integer by name -- %s", data->name.string);
    } else {
        switch(item->type) {
            case 5: /* weapon */
                if(i2d_value_map_get_string(print->weapon_type, integer, &string)) {
                    status = i2d_panic("failed to get weapon type by integer -- %ld", integer);
                } else {
                    status = i2d_handler_general(print, data, &string, stack);
                }
                break;
            case 10: /* ammo */
                if(i2d_value_map_get_string(print->ammo_type, integer, &string)) {
                    status = i2d_panic("failed to get ammo type by integer -- %ld", integer);
                } else {
                    status = i2d_handler_general(print, data, &string, stack);
                }
                break;
                break;
        }
    }

    return status;
}

