#include "i2d_produce.h"

static int i2d_produce_create(i2d_produce *, char *, size_t);
static int i2d_produce_parse(i2d_produce *, char *, size_t);
static int i2d_produce_db_load(i2d_produce_db *, i2d_string *);
static int i2d_produce_db_parse(char *, size_t, void *);
static int i2d_produce_db_index(i2d_produce_db *);

int i2d_produce_init(i2d_produce ** result, char * string, size_t length) {
    int status = I2D_OK;
    i2d_produce * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_produce_create(object, string, length) || i2d_produce_parse(object, string, length)) {
                status = i2d_panic("failed to load produce -- %s", string);
            } else {
                object->next = object;
                object->prev = object;
            }

            if(status)
                i2d_produce_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_produce_deit(i2d_produce ** result) {
    i2d_produce * object;

    object = *result;
    i2d_free(object->materials);
    i2d_free(object);
    *result = NULL;
}

static int i2d_produce_create(i2d_produce * produce, char * string, size_t length) {
    int status = I2D_OK;

    size_t i;
    size_t comma_count = 0;

    for(i = 0; i < length; i++)
        if(',' == string[i])
            comma_count++;

    if(comma_count < 6) {
        status = i2d_panic("row is missing columns");
    } else {
        produce->material_count = comma_count - 4;
        if((produce->material_count % 2) != 0) {
            status = i2d_panic("mismatch in material item id and amount");
        } else {
            produce->materials = calloc(produce->material_count, sizeof(*produce->materials));
            if(!produce->materials)
                status = i2d_panic("out of memory");
        }
    }

    return status;
}

static int i2d_produce_parse(i2d_produce * produce, char * string, size_t length) {
    int status = I2D_OK;

    size_t i;

    char * anchor;
    size_t extent;

    int field = 0;

    int material_index = 0;

    anchor = string;
    for(i = 0; i < length && !status; i++) {
        if(',' == string[i]) {
            string[i] = 0;

            if((string + i) < anchor) {
                status = i2d_panic("line overflow");
            } else {
                extent = (size_t) (string + i) - (size_t) anchor;
                switch(field) {
                    case 0: status = i2d_strtol(&produce->id, anchor, extent, 10); break;
                    case 1: status = i2d_strtol(&produce->item_id, anchor, extent, 10); break;
                    case 2: status = i2d_strtol(&produce->item_level, anchor, extent, 10); break;
                    case 3: status = i2d_strtol(&produce->skill_id, anchor, extent, 10); break;
                    case 4: status = i2d_strtol(&produce->skill_level, anchor, extent, 10); break;
                    default:
                        if(material_index >= produce->material_count) {
                            status = i2d_panic("material overflow");
                        } else {
                            status = i2d_strtol(&produce->materials[material_index], anchor, extent, 10);
                            material_index++;
                        }
                        break;
                }
                field++;
            }

            anchor = (string + i + 1);
        }
    }

    if(!status) {
        if(5 > field) {
            status = i2d_panic("row is missing columns");
        } else if(&string[i] < anchor) {
            status = i2d_panic("line overflow");
        } else {
            extent = (size_t) &string[i] - (size_t) anchor;
            if(material_index >= produce->material_count) {
                status = i2d_panic("material overflow");
            } else {
                status = i2d_strtol(&produce->materials[material_index], anchor, extent, 10);
                material_index++;
            }

            if(material_index != produce->material_count)
                status = i2d_panic("produce is missing material item id and amount");
        }
    }

    return status;
}

void i2d_produce_append(i2d_produce * x, i2d_produce * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_produce_remove(i2d_produce * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

int i2d_produce_db_init(i2d_produce_db ** result, i2d_string * path) {
    int status = I2D_OK;
    i2d_produce_db * object;

    if(i2d_is_invalid(result) || !path) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_produce_db_load(object, path)) {
                status = i2d_panic("failed to load produce db");
            } else if(i2d_produce_db_index(object)) {
                status = i2d_panic("failed to index produce db");
            }

            if(status)
                i2d_produce_db_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_produce_db_deit(i2d_produce_db ** result) {
    i2d_produce_db * object;
    i2d_produce * produce;

    object = *result;
    i2d_deit(object->index_by_id, i2d_rbt_deit);
    if(object->list) {
        while(object->list != object->list->next) {
            produce = object->list->next;
            i2d_produce_remove(produce);
            i2d_produce_deit(&produce);
        }
        i2d_produce_deit(&object->list);
    }
    i2d_free(object);
    *result = NULL;
}

static int i2d_produce_db_load(i2d_produce_db * produce_db, i2d_string * path) {
    int status = I2D_OK;

    int fd;
    i2d_buffer buffer;
    int result;

    fd = open(path->string, O_RDONLY);
    if(0 > fd) {
        status = i2d_panic("failed to open produce db -- %s", path->string);
    } else {
        if(i2d_buffer_create(&buffer, BUFFER_SIZE_LARGE * 2)) {
            status = i2d_panic("failed to create buffer object");
        } else {
            result = i2d_fd_read(fd, BUFFER_SIZE_LARGE, &buffer);
            while(0 < result && !status) {
                if(i2d_by_line(&buffer, i2d_produce_db_parse, produce_db))
                    status = i2d_panic("failed to parse buffer");
                result = i2d_fd_read(fd, BUFFER_SIZE_LARGE, &buffer);
            }
            if(!status && buffer.offset && i2d_by_line(&buffer, i2d_produce_db_parse, produce_db))
                status = i2d_panic("failed to parse buffer");
            i2d_buffer_destroy(&buffer);
        }
        close(fd);
    }

    return status;
}

static int i2d_produce_db_parse(char * string, size_t length, void * data) {
    int status = I2D_OK;
    i2d_produce_db * produce_db = data;
    i2d_produce * produce = NULL;

    if(i2d_produce_init(&produce, string, length)) {
        status = i2d_panic("failed to create produce object");
    } else {
        if(!produce_db->list) {
            produce_db->list = produce;
        } else {
            i2d_produce_append(produce, produce_db->list);
        }

        produce_db->size++;
    }

    return status;
}

static int i2d_produce_db_index(i2d_produce_db * produce_db) {
    int status = I2D_OK;
    i2d_produce * produce = NULL;

    if(i2d_rbt_init(&produce_db->index_by_id, i2d_rbt_cmp_long)) {
        status = i2d_panic("failed to create red black tree object");
    } else {
        produce = produce_db->list;
        do {
            if( i2d_rbt_insert(produce_db->index_by_id, &produce->id, produce))
                status = i2d_panic("failed to index produce by id -- %ld", produce->id);
            produce = produce->next;
        } while(produce != produce_db->list && !status);
    }

    return status;
}

int i2d_produce_db_search_by_id(i2d_produce_db * produce_db, long id, i2d_produce ** produce) {
    return i2d_rbt_search(produce_db->index_by_id, &id, (void **) produce);
}
