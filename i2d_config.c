#include "i2d_config.h"

int i2d_config_init(i2d_config ** result, i2d_string * path) {
    int status = I2D_OK;
    i2d_config * object;
    json_t * config;
    json_t * source_path;
    json_t * data_path;
    json_t * renewal;
    json_t * item_id;

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
                source_path = json_object_get(config, "source_path");
                data_path = json_object_get(config, "data_path");
                renewal = json_object_get(config, "renewal");
                item_id = json_object_get(config, "item_id");
                if(!source_path || i2d_object_get_string(source_path, &object->source_path)) {
                    status = i2d_panic("failed to get source path");
                } else if(!data_path || i2d_object_get_string(data_path, &object->data_path)) {
                    status = i2d_panic("failed to get data path");
                } else if(!renewal || i2d_object_get_boolean(renewal, &object->renewal)) {
                    status = i2d_panic("failed to get renewal");
                } else if(item_id && i2d_object_get_number(item_id, &object->item_id)) {
                    status = i2d_panic("failed to get item id");
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
    i2d_string_destroy(&object->data_path);
    i2d_string_destroy(&object->source_path);
    i2d_free(object);
    *result = NULL;
}