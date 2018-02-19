#include "i2d_logic.h"

enum {
    var,
    and,
    or,
    not
};

int i2d_logic_init(i2d_logic ** result, i2d_str * name, i2d_range_list * range) {
    int status = I2D_OK;
    i2d_logic * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_str_init(&object->name, name->string, name->length)) {
                status = i2d_panic("failed to create string object");
            } else if(i2d_range_list_copy(&object->range, range)) {
                status = i2d_panic("failed to create range list object");
            } else {
                object->type = var;
            }

            if(status)
                i2d_logic_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_logic_deit(i2d_logic ** result) {
    i2d_logic * object;

    object = *result;
    i2d_deit(object->right, i2d_logic_deit);
    i2d_deit(object->left, i2d_logic_deit);
    i2d_deit(object->range, i2d_range_list_deit);
    i2d_deit(object->name, i2d_str_deit);
    i2d_free(object);
    *result = NULL;
}
