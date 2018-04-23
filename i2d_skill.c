#include "i2d_skill.h"

#define READ_SIZE 65536

static int i2d_skill_db_load(i2d_skill_db *, i2d_str *);
static int i2d_skill_db_parse(char *, size_t, void *);

int i2d_skill_db_init(i2d_skill_db ** result, i2d_str * path) {
    int status = I2D_OK;
    i2d_skill_db * object;

    if(i2d_is_invalid(result) || !path) {
        status = i2d_panic("invalid parameter");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_skill_db_load(object, path))
                status = i2d_panic("failed to load skill db");

            if(status)
                i2d_skill_db_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_skill_db_deit(i2d_skill_db ** result) {
    i2d_skill_db * object;

    object = *result;
    i2d_free(object);
    *result = NULL;
}

static int i2d_skill_db_load(i2d_skill_db * skill_db, i2d_str * path) {
    int status = I2D_OK;

    int fd;
    i2d_buf * buffer = NULL;
    int result;

    fd = open(path->string, O_RDONLY);
    if(0 > fd) {
        status = i2d_panic("failed to open skill db -- %s", path->string);
    } else {
        if(i2d_buf_init(&buffer, READ_SIZE + 4096)) {
            status = i2d_panic("failed to create buffer object");
        } else {
            result = i2d_fd_read(fd, READ_SIZE, buffer);
            while(0 < result && !status) {
                if(i2d_by_line(buffer, i2d_skill_db_parse, skill_db))
                    status = i2d_panic("failed to parse buffer");
                result = i2d_fd_read(fd, READ_SIZE, buffer);
            }
            if(!status && buffer->offset && i2d_by_line(buffer, i2d_skill_db_parse, skill_db))
                status = i2d_panic("failed to parse buffer");
            i2d_buf_deit(&buffer);
        }
        close(fd);
    }

    return status;
}

static int i2d_skill_db_parse(char * string, size_t length, void * data) {
    int status = I2D_OK;

    return status;
}
