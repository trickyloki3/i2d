#include "i2d_db.h"

static int i2d_db_load_item_db(i2d_db *, i2d_string *);
static int i2d_db_load_skill_db(i2d_db *, i2d_string *);
static int i2d_db_load_mob_db(i2d_db *, i2d_string *);
static int i2d_db_load_mob_race_db(i2d_db *, i2d_string *);

int i2d_db_init(i2d_db ** result, enum i2d_db_type type, i2d_string * source_path) {
    int status = I2D_OK;
    i2d_db * object;

    if(i2d_is_invalid(result) || !source_path) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            object->type = type;
            if(i2d_db_load_item_db(object, source_path)) {
                status = i2d_panic("failed to load item db");
            } else if(i2d_db_load_skill_db(object, source_path)) {
                status = i2d_panic("failed to load skill db");
            } else if(i2d_db_load_mob_db(object, source_path)) {
                status = i2d_panic("failed to load mob db");
            } else if(i2d_db_load_mob_race_db(object, source_path)) {
                status = i2d_panic("failed to load mob race db");
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
    i2d_deit(object->mob_race_db, i2d_mob_race_db_deit);
    i2d_deit(object->mob_db, i2d_mob_db_deit);
    i2d_deit(object->skill_db, i2d_skill_db_deit);
    i2d_deit(object->item_db, i2d_item_db_deit);
    i2d_free(object);
    *result = NULL;
}

static int i2d_db_load_item_db(i2d_db * db, i2d_string * source_path) {
    int status = I2D_OK;
    const char * type;
    i2d_string path;

    type = db->type == i2d_pre_renewal ? "pre-re" : "re";
    if(i2d_string_vprintf(&path, "%s/db/%s/item_db.txt", source_path->string, type)) {
        status = i2d_panic("failed to write item db path");
    } else {
        status = i2d_item_db_init(&db->item_db, &path);
        i2d_free(path.string);
    }

    return status;
}

static int i2d_db_load_skill_db(i2d_db * db, i2d_string * source_path) {
    int status = I2D_OK;
    const char * type;
    i2d_string path;

    type = db->type == i2d_pre_renewal ? "pre-re" : "re";
    if(i2d_string_vprintf(&path, "%s/db/%s/skill_db.txt", source_path->string, type)) {
        status = i2d_panic("failed to write skill db path");
    } else {
        status = i2d_skill_db_init(&db->skill_db, &path);
        i2d_free(path.string);
    }

    return status;
}

static int i2d_db_load_mob_db(i2d_db * db, i2d_string * source_path) {
    int status = I2D_OK;
    const char * type;
    i2d_string path;

    type = db->type == i2d_pre_renewal ? "pre-re" : "re";
    if(i2d_string_vprintf(&path, "%s/db/%s/mob_db.txt", source_path->string, type)) {
        status = i2d_panic("failed to write mob db path");
    } else {
        status = i2d_mob_db_init(&db->mob_db, &path);
        i2d_free(path.string);
    }

    return status;
}

static int i2d_db_load_mob_race_db(i2d_db * db, i2d_string * source_path) {
    int status = I2D_OK;
    const char * type;
    i2d_string path;

    type = db->type == i2d_pre_renewal ? "pre-re" : "re";
    if(i2d_string_vprintf(&path, "%s/db/%s/mob_race2_db.txt", source_path->string, type)) {
        status = i2d_panic("failed to write mob race db path");
    } else {
        status = i2d_mob_race_db_init(&db->mob_race_db, &path);
        i2d_free(path.string);
    }

    return status;
}
