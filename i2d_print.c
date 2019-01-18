#include "i2d_print.h"

typedef int (* i2d_handler_print_cb)(i2d_print *, i2d_data *, i2d_item *, i2d_string_stack *);

struct i2d_handler {
    char * name;
    i2d_handler_print_cb handler;
};

typedef struct i2d_handler i2d_handler;

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
            } else if(i2d_buffer_cache_init(&object->buffer_cache)) {
                status = i2d_panic("failed to create buffer cache object");
            } else if(i2d_string_stack_cache_init(&object->stack_cache)) {
                status = i2d_panic("failed to create string stack cache object");
            } else {
                size = i2d_size(print_handlers);
                for(i = 0; i < size && !status; i++)
                    if(i2d_rbt_insert(object->print_handlers, print_handlers[i].name, &print_handlers[i]))
                        status = i2d_panic("failed to map handler object");
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
    i2d_deit(object->stack_cache, i2d_string_stack_cache_deit);
    i2d_deit(object->buffer_cache, i2d_buffer_cache_deit);
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

    return status;
}

static int i2d_handler_integer(i2d_print * print, i2d_data * data, i2d_item * item, i2d_string_stack * stack) {
    int status = I2D_OK;
    return status;
}

static int i2d_handler_string(i2d_print * print, i2d_data * data, i2d_item * item, i2d_string_stack * stack) {
    int status = I2D_OK;
    return status;
}

static int i2d_handler_type(i2d_print * print, i2d_data * data, i2d_item * item, i2d_string_stack * stack) {
    int status = I2D_OK;
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
    return status;
}

static int i2d_handler_refine(i2d_print * print, i2d_data * data, i2d_item * item, i2d_string_stack * stack) {
    int status = I2D_OK;
    return status;
}

static int i2d_handler_view(i2d_print * print, i2d_data * data, i2d_item * item, i2d_string_stack * stack) {
    int status = I2D_OK;
    return status;
}

