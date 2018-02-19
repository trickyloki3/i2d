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

void i2d_logic_print(i2d_logic * logic, int level) {
    int i;

    if(logic) {
        for(i = 0; i < level; i++)
            putc('\t', stdout);
        switch(logic->type) {
            case var: fprintf(stdout, "[var] "); break;
            case and: fprintf(stdout, "[and] "); break;
            case or:  fprintf(stdout, "[or] ");  break;
            case not: fprintf(stdout, "[not] "); break;
        }
        if(logic->range)
            i2d_range_list_print(logic->range, NULL);
        else
            fprintf(stdout, "\n");

        i2d_logic_print(logic->left, level + 1);
        i2d_logic_print(logic->right, level + 1);
    }
}
