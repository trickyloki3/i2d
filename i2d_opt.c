#include "i2d_opt.h"

#include "getopt.h"

static const char * i2d_short_options = "s:j:i:d:";

static struct option i2d_long_options[] = {
    { "source_path", required_argument, NULL, 's' },
    { "data_path", required_argument, NULL, 'd' },
    { "json_path", required_argument, NULL, 'j' },
    { "item_id", required_argument, NULL, 'i' }
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
                    case 's':
                        if(i2d_string_create(&object->source_path, optarg, strlen(optarg)))
                            status = i2d_panic("failed on source directory path argument -- %s", optarg);
                        break;
                    case 'd':
                        if(i2d_string_create(&object->data_path, optarg, strlen(optarg)))
                            status = i2d_panic("failed on data directory path argument -- %s", optarg);
                        break;
                    case 'i':
                        if(i2d_strtol(&object->item_id, optarg, strlen(optarg), 10))
                            status = i2d_panic("failed to strtol item id argument -- %s", optarg);
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
    i2d_string_destroy(&object->data_path);
    i2d_string_destroy(&object->source_path);
    i2d_free(object);
    *result = NULL;
}
