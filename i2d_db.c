#include "i2d_db.h"

static int i2d_db_load_item_db(i2d_db *, i2d_string *);
static int i2d_db_load_skill_db(i2d_db *, i2d_string *);

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
    i2d_deit(object->skill_db, i2d_skill_db_deit);
    i2d_deit(object->item_db, i2d_item_db_deit);
    i2d_free(object);
    *result = NULL;
}

static int i2d_db_load_item_db(i2d_db * db, i2d_string * source_path) {
    int status = I2D_OK;
    const char * type;
    i2d_buffer buffer;
    i2d_string path;

    type = db->type == i2d_pre_renewal ? "pre-re" : "re";
    if(i2d_buffer_create(&buffer, I2D_SIZE)) {
        status = i2d_panic("failed to create buffer object");
    } else {
        if(i2d_buffer_printf(&buffer, "%s/db/%s/item_db.txt", source_path->string, type)) {
            status = i2d_panic("failed to write item db path");
        } else {
            i2d_buffer_get(&buffer, &path.string, &path.length);
            status = i2d_item_db_init(&db->item_db, &path);
        }
        i2d_buffer_destroy(&buffer);
    }

    return status;
}

static int i2d_db_load_skill_db(i2d_db * db, i2d_string * source_path) {
    int status = I2D_OK;
    const char * type;
    i2d_buffer buffer;
    i2d_string path;

    type = db->type == i2d_pre_renewal ? "pre-re" : "re";
    if(i2d_buffer_create(&buffer, I2D_SIZE)) {
        status = i2d_panic("failed to create buffer object");
    } else {
        if(i2d_buffer_printf(&buffer, "%s/db/%s/skill_db.txt", source_path->string, type)) {
            status = i2d_panic("failed to write item db path");
        } else {
            i2d_buffer_get(&buffer, &path.string, &path.length);
            status = i2d_skill_db_init(&db->skill_db, &path);
        }
        i2d_buffer_destroy(&buffer);
    }

    return status;
}
