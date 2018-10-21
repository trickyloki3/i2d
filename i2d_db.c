#include "i2d_db.h"

static int i2d_db_load_item_db(i2d_db *);
static int i2d_db_load_skill_db(i2d_db *);
static int i2d_db_load_mob_db(i2d_db *);
static int i2d_db_load_mob_race_db(i2d_db *);
static int i2d_db_load_produce_db(i2d_db *);
static int i2d_db_load_mercenary_db(i2d_db *);
static int i2d_db_load_pet_db(i2d_db *);
static int i2d_db_load_item_combo_db(i2d_db *);

int i2d_db_init(i2d_db ** result, enum i2d_db_type type, i2d_string * path) {
    int status = I2D_OK;
    i2d_db * object;

    if(i2d_is_invalid(result) || !path) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_string_vprintf(&object->db_path, "%s/db", path->string)) {
                status = i2d_panic("failed to create string object");
            } else {
                switch(type) {
                    case i2d_pre_renewal: 
                        if(i2d_string_vprintf(&object->re_path, "%s/pre-re", object->db_path.string))
                            status = i2d_panic("failed to create string object");
                        break;
                    case i2d_renewal:
                        if(i2d_string_vprintf(&object->re_path, "%s/re", object->db_path.string))
                            status = i2d_panic("failed to create string object");
                        break;
                    default: 
                        status = i2d_panic("invalid db type -- %d", type);
                }

                if(!status) {
                    if(i2d_db_load_item_db(object)) {
                        status = i2d_panic("failed to load item db");
                    } else if(i2d_db_load_skill_db(object)) {
                        status = i2d_panic("failed to load skill db");
                    } else if(i2d_db_load_mob_db(object)) {
                        status = i2d_panic("failed to load mob db");
                    } else if(i2d_db_load_mob_race_db(object)) {
                        status = i2d_panic("failed to load mob race db");
                    } else if(i2d_db_load_produce_db(object)) {
                        status = i2d_panic("failed to load produce db");
                    } else if(i2d_db_load_mercenary_db(object)) {
                        status = i2d_panic("failed to load mercenary db");
                    } else if(i2d_db_load_pet_db(object)) {
                        status = i2d_panic("failed to load pet db");
                    } else if(i2d_db_load_item_combo_db(object)) {
                        status = i2d_panic("failed to load item combo db");
                    }
                }
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
    i2d_deit(object->item_combo_db, i2d_item_combo_db_deit);
    i2d_deit(object->pet_db, i2d_pet_db_deit);
    i2d_deit(object->mercenary_db, i2d_mercenary_db_deit);
    i2d_deit(object->produce_db, i2d_produce_db_deit);
    i2d_deit(object->mob_race_db, i2d_mob_race_db_deit);
    i2d_deit(object->mob_db, i2d_mob_db_deit);
    i2d_deit(object->skill_db, i2d_skill_db_deit);
    i2d_deit(object->item_db, i2d_item_db_deit);
    i2d_string_destroy(&object->re_path);
    i2d_string_destroy(&object->db_path);
    i2d_free(object);
    *result = NULL;
}

static int i2d_db_load_item_db(i2d_db * db) {
    int status = I2D_OK;
    i2d_string path;

    if(i2d_string_vprintf(&path, "%s/item_db.txt", db->re_path.string)) {
        status = i2d_panic("failed to create string object");
    } else {
        if(i2d_item_db_init(&db->item_db, &path))
            status = i2d_panic("failed to create item db object");
        i2d_string_destroy(&path);
    }

    return status;
}

static int i2d_db_load_skill_db(i2d_db * db) {
    int status = I2D_OK;
    i2d_string path;

    if(i2d_string_vprintf(&path, "%s/skill_db.txt", db->re_path.string)) {
        status = i2d_panic("failed to create string object");
    } else {
        if(i2d_skill_db_init(&db->skill_db, &path))
            status = i2d_panic("failed to create skill db object");
        i2d_string_destroy(&path);
    }

    return status;
}

static int i2d_db_load_mob_db(i2d_db * db) {
    int status = I2D_OK;
    i2d_string path;

    if(i2d_string_vprintf(&path, "%s/mob_db.txt", db->re_path.string)) {
        status = i2d_panic("failed to create string object");
    } else {
        if(i2d_mob_db_init(&db->mob_db, &path))
            status = i2d_panic("failed to create mob db object");
        i2d_string_destroy(&path);
    }

    return status;
}

static int i2d_db_load_mob_race_db(i2d_db * db) {
    int status = I2D_OK;
    i2d_string path;

    if(i2d_string_vprintf(&path, "%s/mob_race2_db.txt", db->re_path.string)) {
        status = i2d_panic("failed to create string object");
    } else {
        if(i2d_mob_race_db_init(&db->mob_race_db, &path))
            status = i2d_panic("failed to create mob race db object");
        i2d_string_destroy(&path);
    }

    return status;
}

static int i2d_db_load_produce_db(i2d_db * db) {
    int status = I2D_OK;
    i2d_string path;

    if(i2d_string_vprintf(&path, "%s/produce_db.txt", db->re_path.string)) {
        status = i2d_panic("failed to create string object");
    } else {
        if(i2d_produce_db_init(&db->produce_db, &path))
            status = i2d_panic("failed to create produce db object");
        i2d_string_destroy(&path);
    }

    return status;
}

static int i2d_db_load_mercenary_db(i2d_db * db) {
    int status = I2D_OK;
    i2d_string path;

    if(i2d_string_vprintf(&path, "%s/mercenary_db.txt", db->db_path.string)) {
        status = i2d_panic("failed to create string object");
    } else {
        if(i2d_mercenary_db_init(&db->mercenary_db, &path))
            status = i2d_panic("failed to create mercenary db object");
        i2d_string_destroy(&path);
    }

    return status;
}

static int i2d_db_load_pet_db(i2d_db * db) {
    int status = I2D_OK;
    i2d_string path;

    if(i2d_string_vprintf(&path, "%s/pet_db.txt", db->re_path.string)) {
        status = i2d_panic("failed to create string object");
    } else {
        if(i2d_pet_db_init(&db->pet_db, &path))
            status = i2d_panic("failed to create pet db object");
        i2d_string_destroy(&path);
    }

    return status;
}

static int i2d_db_load_item_combo_db(i2d_db * db) {
    int status = I2D_OK;
    i2d_string path;

    if(i2d_string_vprintf(&path, "%s/item_combo_db.txt", db->re_path.string)) {
        status = i2d_panic("failed to create string object");
    } else {
        if(i2d_item_combo_db_init(&db->item_combo_db, &path))
            status = i2d_panic("failed to create item combo db object");
        i2d_string_destroy(&path);
    }

    return status;
}
