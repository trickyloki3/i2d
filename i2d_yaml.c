#include "i2d_yaml.h"

static int i2d_yaml_reader(void * context, unsigned char * buffer, size_t length, size_t * result) {
    int status = I2D_OK;
    int * fd = context;
    ssize_t bytes;

    bytes = read(*fd, buffer, length);
    if(bytes < 0) {
        status = i2d_panic("failed to read file");
    } else {
        *result = (size_t) bytes;
    }

    return status ? 0 : 1; 
}

int i2d_yaml_parse(i2d_string * path, i2d_yaml_parse_cb handler, void * context) {
    int status = I2D_OK;
    int sentry = 0;

    int fd = -1;
    yaml_parser_t parser;
    yaml_event_t event;

    fd = open(path->string, O_RDONLY);
    if(fd < 0) {
        status = i2d_panic("failed to open file -- %s", path->string);
    } else {
        if(!yaml_parser_initialize(&parser)) {
            status = i2d_panic("failed to create yaml parser object");
        } else {
            yaml_parser_set_input(&parser, i2d_yaml_reader, &fd);
            while(!status && !sentry) {
                if(!yaml_parser_parse(&parser, &event)) {
                    status = i2d_panic("failed to create yaml event object");
                } else {
                    if(event.type == YAML_STREAM_END_EVENT)
                        sentry = 1;

                    if(handler(&event, context)) 
                        status = i2d_panic("failed on yaml handler");

                    yaml_event_delete(&event);
                }
            }
            yaml_parser_delete(&parser);
        }
        close(fd);
    }

    return status;
}