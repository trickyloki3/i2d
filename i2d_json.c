#include "i2d_json.h"

static int i2d_block_data_load(i2d_block_data *, json_t *);
static int i2d_json_load(i2d_json *, i2d_str *);
static int i2d_json_block_data_load(i2d_json *, json_t *);
static int i2d_json_block_data_index(i2d_json *);

int i2d_block_data_init(i2d_block_data ** result, json_t * json) {
    int status = I2D_OK;
    i2d_block_data * object = NULL;

    if(i2d_is_invalid(result) || !json) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_block_data_load(object, json)) {
                status = i2d_panic("failed to load block data");
            } else {
                object->next = object;
                object->prev = object;
            }

            if(status)
                i2d_block_data_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_block_data_deit(i2d_block_data ** result) {
    i2d_block_data * object;

    object = *result;
    i2d_deit(object->name, i2d_str_deit);
    i2d_free(object);
    *result = NULL;
}

void i2d_block_data_list_deit(i2d_block_data ** result) {
    i2d_block_data * object;
    i2d_block_data * block_data;

    object = *result;
    if(object) {
        while(object != object->next) {
            block_data = object->next;
            i2d_block_data_remove(block_data);
            i2d_block_data_deit(&block_data);
        }
    }
    i2d_deit(object, i2d_block_data_deit);
}

void i2d_block_data_append(i2d_block_data * x, i2d_block_data * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_block_data_remove(i2d_block_data * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

static int i2d_block_data_load(i2d_block_data * block_data, json_t * json) {
    int status = I2D_OK;
    json_t * name;
    const char * string;
    size_t length;

    name = json_object_get(json, "name");
    if(!name) {
        status = i2d_panic("block data missing name key");
    } else {
        string = json_string_value(name);
        length = json_string_length(name);
        if(!string || !length) {
            status = i2d_panic("block data invalid name value");
        } else if(i2d_str_init(&block_data->name, string, length)) {
            status = i2d_panic("failed to create string object");
        }
    }

    return status;
}

int i2d_json_init(i2d_json ** result, i2d_str * path) {
    int status = I2D_OK;
    i2d_json * object;

    if(i2d_is_invalid(result) || !path) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_json_load(object, path))
                status = i2d_panic("failed to load json file");

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
    i2d_deit(object->block_data_index, i2d_rbt_deit);
    i2d_deit(object->block_data_list, i2d_block_data_list_deit);
    i2d_free(object);
    *result = NULL;
}

static int i2d_json_load(i2d_json * json, i2d_str * path) {
    int status = I2D_OK;
    json_t * object = NULL;

    json_error_t error;
    i2d_zero(error);

    object = json_load_file(path->string, JSON_DISABLE_EOF_CHECK, &error);
    if(!object) {
        status = i2d_panic("%s (line %d column %d)", error.text, error.line, error.column);
    } else {
        if(i2d_json_block_data_load(json, object)) {
            status = i2d_panic("failed to load block data");
        } else if(i2d_json_block_data_index(json)) {
            status = i2d_panic("failed to index block data");
        }
        json_decref(object);
    }

    return status;
}

static int i2d_json_block_data_load(i2d_json * json, json_t * object) {
    int status = I2D_OK;
    json_t * array;

    size_t i;
    size_t size;
    i2d_block_data * block_data = NULL;

    array = json_object_get(object, "block_data");
    if(!array) {
        status = i2d_panic("object missing block data key");
    } else {
        size = json_array_size(array);
        if(!size) {
            status = i2d_panic("block data array is empty");
        } else {
            for(i = 0; i < size; i++) {
                object = json_array_get(array, i);
                if(!object) {
                    status = i2d_panic("block data array at index %zu is NULL", i);
                } else if(i2d_block_data_init(&block_data, object)) {
                    status = i2d_panic("failed to create block data");
                } else {
                    if(!json->block_data_list) {
                        json->block_data_list = block_data;
                    } else {
                        i2d_block_data_append(block_data, json->block_data_list);
                    }
                    block_data = NULL;
                }
            }
        }
    }

    return status;
}

static int i2d_json_block_data_index(i2d_json * json) {
    int status = I2D_OK;
    i2d_block_data * block_data;

    if(i2d_rbt_init(&json->block_data_index, i2d_rbt_cmp_str)) {
        status = i2d_panic("failed to create red block tree object");
    } else if(json->block_data_list) {
        status = i2d_panic("block data list is empty");
    } else {
        block_data = json->block_data_list;
        do {
            if(i2d_rbt_insert(json->block_data_index, block_data->name, block_data))
                status = i2d_panic("failed to add block data to index");

            block_data = block_data->next;
        } while(block_data != json->block_data_list && !status);
    }

    return status;
}

#if i2d_debug
int i2d_json_test() {
    int status = I2D_OK;
    i2d_str * path = NULL;
    i2d_json * json = NULL;

    if(i2d_str_init(&path, "i2d.json", 8)) {
        status = i2d_panic("failed to create string object");
    } else {
        if(i2d_json_init(&json, path)) {
            status = i2d_panic("failed to create json object");
        } else {

            i2d_json_deit(&json);
        }
        i2d_str_deit(&path);
    }

    return status;
}
#endif
