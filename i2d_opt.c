#include "i2d_opt.h"

#include "getopt.h"

static const char * i2d_short_options = "i:j:";

static struct option i2d_long_options[] = {
    { "item_db_path", required_argument, NULL, 'i' },
    { "json_path", required_argument, NULL, 'j' }
};

int i2d_option_init(i2d_option ** result, int argc, char ** argv) {
    int status = I2D_OK;
    i2d_option * object;

    int opt;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            opt = getopt_long(argc, argv, i2d_short_options, i2d_long_options, NULL);
            while(-1 < opt && !status) {
                switch(opt) {
                    case 'i':
                        if(i2d_str_copy(&object->item_db_path, optarg, strlen(optarg)))
                            status = i2d_panic("failed on item db path argument -- %s", optarg);
                        break;
                    case 'j':
                        if(i2d_str_copy(&object->json_path, optarg, strlen(optarg)))
                            status = i2d_panic("failed on json path argument -- %s", optarg);
                        break;
                }
                opt = getopt_long(argc, argv, i2d_short_options, i2d_long_options, NULL);
            }

            if(status)
                i2d_option_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_option_deit(i2d_option ** result) {
    i2d_option * object;

    object = *result;
    i2d_deit(object->item_db_path, i2d_str_deit);
    i2d_free(object);
    *result = NULL;
}
