#include "i2d_config.h"

int i2d_config_init(i2d_config ** result, i2d_string * path) {
    int status = I2D_OK;
    i2d_config * object;
    json_t * config;
    json_t * data_path;
    json_t * item_id;
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
                data_path = json_object_get(config, "data_path");
                item_id = json_object_get(config, "item_id");
                item_db_path = json_object_get(config, "item_db_path");
                skill_db_path = json_object_get(config, "skill_db_path");
                mob_db_path = json_object_get(config, "mob_db_path");
                mob_race2_db_path = json_object_get(config, "mob_race2_db_path");
                produce_db_path = json_object_get(config, "produce_db_path");
                mercenary_db_path = json_object_get(config, "mercenary_db_path");
                pet_db_path = json_object_get(config, "pet_db_path");
                item_combo_db_path = json_object_get(config, "item_combo_db_path");
                if(!data_path || i2d_object_get_string(data_path, &object->data_path)) {
                    status = i2d_panic("failed to get data path");
                } else if(item_id && i2d_object_get_number(item_id, &object->item_id)) {
                    status = i2d_panic("failed to get item id");
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
    i2d_string_destroy(&object->data_path);
    i2d_free(object);
    *result = NULL;
}