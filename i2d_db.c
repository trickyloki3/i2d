#include "i2d_db.h"

static int i2d_db_load_item_db(i2d_db *, i2d_str *, const char *, i2d_buf *);
static int i2d_db_load_skill_db(i2d_db *, i2d_str *, const char *, i2d_buf *);

int i2d_db_init(i2d_db ** result, enum i2d_db_type type, i2d_str * source_path) {
    int status = I2D_OK;
    i2d_db * object;

    char * directory;
    i2d_buf * buffer = NULL;

    if(i2d_is_invalid(result) || !source_path) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            object->type = type;
            switch(object->type) {
                case i2d_db_pre_renewal:
                    directory = "pre-re";
                    break;
                case i2d_db_renewal:
                    directory = "re";
                    break;
                default:
                    status = i2d_panic("invalid database type -- %d", object->type);
                    break;
            }

            if(!status) {
                if(i2d_buf_init(&buffer, 256)) {
                    status = i2d_panic("failed to create buffer object");
                } else {
                    if(i2d_db_load_item_db(object, source_path, directory, buffer)) {
                        status = i2d_panic("failed to load item database");
                    } else if(i2d_db_load_skill_db(object, source_path, directory, buffer)) {
                        status = i2d_panic("failed to load skill database");
                    }

                    i2d_buf_deit(&buffer);
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
    i2d_deit(object->skill_db, i2d_skill_db_deit);
    i2d_deit(object->item_db, i2d_item_db_deit);
    i2d_free(object);
    *result = NULL;
}

static int i2d_db_load_item_db(i2d_db * db, i2d_str * source_path, const char * directory, i2d_buf * buffer) {
    int status = I2D_OK;
    i2d_str path;

    i2d_buf_zero(buffer);

    if(i2d_buf_format(buffer, "%s/db/%s/item_db.txt", source_path->string, directory)) {
        status = i2d_panic("failed to get item db path");
    } else {
        i2d_buf_get_str(buffer, &path);

        if(i2d_item_db_init(&db->item_db, &path))
            status = i2d_panic("failed to create item database object");
    }

    return status;
}

static int i2d_db_load_skill_db(i2d_db * db, i2d_str * source_path, const char * directory, i2d_buf * buffer) {
    int status = I2D_OK;
    i2d_str path;

    i2d_buf_zero(buffer);

    if(i2d_buf_format(buffer, "%s/db/%s/skill_db.txt", source_path->string, directory)) {
        status = i2d_panic("failed to get item db path");
    } else {
        i2d_buf_get_str(buffer, &path);

        if(i2d_skill_db_init(&db->skill_db, &path))
            status = i2d_panic("failed to create skill database object");
    }

    return status;
}
