#include "i2d_db.h"

int i2d_db_init(i2d_db ** result, i2d_config * config) {
    int status = I2D_OK;
    i2d_db * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_item_db_init(&object->item_db, &config->item_db_path)) {
                status = i2d_panic("failed to create item db object");
            } else if(i2d_skill_db_init(&object->skill_db, &config->skill_db_path)) {
                status = i2d_panic("failed to create skill db object");
            } else if(i2d_mob_db_init(&object->mob_db, &config->mob_db_path)) {
                status = i2d_panic("failed to create mob db object");
            } else if(i2d_mob_race_db_init(&object->mob_race2_db, &config->mob_race2_db_path)) {
                status = i2d_panic("failed to create mob race2 db object");
            } else if(i2d_produce_db_init(&object->produce_db, &config->produce_db_path)) {
                status = i2d_panic("failed to create produce db object");
            } else if(i2d_mercenary_db_init(&object->mercenary_db, &config->mercenary_db_path)) {
                status = i2d_panic("failed to create mercenary db object");
            } else if(i2d_pet_db_init(&object->pet_db, &config->pet_db_path)) {
                status = i2d_panic("failed to create pet db object");
            } else if(i2d_item_combo_db_init(&object->item_combo_db, &config->item_combo_db_path)) {
                status = i2d_panic("failed to create item combo db object");
            }

            if(status)
                i2d_db_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_db_deit(i2d_db ** result) {
    i2d_db * object;

    object = *result;
    i2d_deit(object->pet_db, i2d_pet_db_deit);
    i2d_deit(object->mercenary_db, i2d_mercenary_db_deit);
    i2d_deit(object->produce_db, i2d_produce_db_deit);
    i2d_deit(object->mob_race2_db, i2d_mob_race_db_deit);
    i2d_deit(object->mob_db, i2d_mob_db_deit);
    i2d_deit(object->skill_db, i2d_skill_db_deit);
    i2d_deit(object->item_combo_db, i2d_item_combo_db_deit);
    i2d_deit(object->item_db, i2d_item_db_deit);
    i2d_free(object);
    *result = NULL;
}
