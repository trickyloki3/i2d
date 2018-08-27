#include "i2d_skill.h"

static int i2d_skill_parse_list(long **, size_t *, char *, size_t);
static int i2d_skill_parse(i2d_skill *, char *, size_t);
static int i2d_skill_db_load(i2d_skill_db *, i2d_string *);
static int i2d_skill_db_parse(char *, size_t, void *);
static int i2d_skill_db_index(i2d_skill_db *);

int i2d_skill_init(i2d_skill ** result, char * string, size_t length) {
    int status = I2D_OK;
    i2d_skill * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_skill_parse(object, string, length)) {
                status = i2d_panic("failed to load skill -- %s", string);
            } else {
                object->next = object;
                object->prev = object;
            }

            if(status)
                i2d_skill_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_skill_deit(i2d_skill ** result) {
    i2d_skill * object;

    object = *result;
    i2d_free(object->name.string);
    i2d_free(object->macro.string);
    i2d_free(object->blow_count);
    i2d_free(object->type.string);
    i2d_free(object->max_count);
    i2d_free(object->cast_cancel.string);
    i2d_free(object->hit_amount);
    i2d_free(object->splash);
    i2d_free(object->element);
    i2d_free(object->range);
    i2d_free(object);
    *result = NULL;
}

static int i2d_skill_parse_list(long ** result_list, size_t * result_size, char * string, size_t length) {
    int status = I2D_OK;
    size_t i;
    size_t size = 1;
    size_t index = 0;
    long * list = NULL;

    char * anchor;
    size_t extent;

    for(i = 0; i < length; i++)
        if(':' == string[i])
            size++;

    list = calloc(size, sizeof(*list));
    if(!list) {
        status = i2d_panic("out of memory");
    } else {
        anchor = string;
        for(i = 0; i < length && !status; i++) {
            if(':' == string[i]) {
                string[i] = 0;

                if((string + i) < anchor) {
                    status = i2d_panic("line overflow");
                } else {
                    extent = (size_t) (string + i) - (size_t) anchor;
                    if(i2d_strtol(&list[index], anchor, extent, 10)) {
                        status = i2d_panic("failed to convert %s to number", anchor);
                    } else {
                        index++;
                    }
                }

                anchor = (string + i + 1);
            }
        }

        if(!status) {
            if(index + 1 != size) {
                status = i2d_panic("list is missing values");
            } else if(&string[i] < anchor) {
                status = i2d_panic("line overflow");
            } else {
                extent = (size_t) &string[i] - (size_t) anchor;
                if(i2d_strtol(&list[index], anchor, extent, 10))
                    status = i2d_panic("failed to convert %s to number", anchor);
            }
        }

        if(status) {
            free(list);
        } else {
            *result_size = size;
            *result_list = list;
        }
    }


    return status;
}

static int i2d_skill_parse(i2d_skill * skill, char * string, size_t length) {
    int status = I2D_OK;

    size_t i;

    char * anchor;
    size_t extent;

    int field = 0;

    anchor = string;
    for(i = 0; i < length && !status; i++) {
        if(',' == string[i]) {
            string[i] = 0;

            if((string + i) < anchor) {
                status = i2d_panic("line overflow");
            } else {
                extent = (size_t) (string + i) - (size_t) anchor;

                while(extent && isspace(*anchor)) {
                    anchor++;
                    extent--;
                }

                switch(field) {
                    case 0: status = i2d_strtol(&skill->id, anchor, extent, 10); break;
                    case 1: status = i2d_skill_parse_list(&skill->range, &skill->range_size, anchor, extent); break;
                    case 2: status = i2d_strtol(&skill->hit, anchor, extent, 10); break;
                    case 3: status = i2d_strtol(&skill->inf, anchor, extent, 10); break;
                    case 4: status = i2d_skill_parse_list(&skill->element, &skill->element_size, anchor, extent); break;
                    case 5: status = i2d_strtol(&skill->nk, anchor, extent, 16); break;
                    case 6: status = i2d_skill_parse_list(&skill->splash, &skill->splash_size, anchor, extent); break;
                    case 7: status = i2d_strtol(&skill->maxlv, anchor, extent, 10); break;
                    case 8: status = i2d_skill_parse_list(&skill->hit_amount, &skill->hit_amount_size, anchor, extent); break;
                    case 9: status = i2d_string_create(&skill->cast_cancel, anchor, extent); break;
                    case 10: status = i2d_strtol(&skill->cast_def_reduce_rate, anchor, extent, 10); break;
                    case 11: status = i2d_strtol(&skill->inf2, anchor, extent, 16); break;
                    case 12: status = i2d_skill_parse_list(&skill->max_count, &skill->max_count_size, anchor, extent); break;
                    case 13: status = i2d_string_create(&skill->type, anchor, extent); break;
                    case 14: status = i2d_skill_parse_list(&skill->blow_count, &skill->blow_count_size, anchor, extent); break;
                    case 15: status = i2d_strtol(&skill->inf3, anchor, extent, 16); break;
                    case 16: status = i2d_string_create(&skill->macro, anchor, extent); break;
                    default: status = i2d_panic("row has too many columns"); break;
                }
                field++;
            }

            anchor = (string + i + 1);
        }
    }

    if(!status) {
        if(17 != field) {
            status = i2d_panic("row is missing columns");
        } else if(&string[i] < anchor) {
            status = i2d_panic("line overflow");
        } else {
            extent = (size_t) &string[i] - (size_t) anchor;
            status = i2d_string_create(&skill->name, anchor, extent);
        }
    }

    return status;
}

void i2d_skill_append(i2d_skill * x, i2d_skill * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_skill_remove(i2d_skill * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

int i2d_skill_db_init(i2d_skill_db ** result, i2d_string * path) {
    int status = I2D_OK;
    i2d_skill_db * object;

    if(i2d_is_invalid(result) || !path) {
        status = i2d_panic("invalid parameter");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_skill_db_load(object, path)) {
                status = i2d_panic("failed to load skill db");
            } else if(i2d_skill_db_index(object)) {
                status = i2d_panic("failed to index skill db");
            }

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
    i2d_skill * skill;

    object = *result;
    i2d_deit(object->index_by_macro, i2d_rbt_deit);
    i2d_deit(object->index_by_id, i2d_rbt_deit);
    if(object->list) {
        while(object->list != object->list->next) {
            skill = object->list->next;
            i2d_skill_remove(skill);
            i2d_skill_deit(&skill);
        }
        i2d_skill_deit(&object->list);
    }
    i2d_free(object);
    *result = NULL;
}

static int i2d_skill_db_load(i2d_skill_db * skill_db, i2d_string * path) {
    int status = I2D_OK;

    int fd;
    i2d_buffer buffer;
    int result;

    fd = open(path->string, O_RDONLY);
    if(0 > fd) {
        status = i2d_panic("failed to open skill db -- %s", path->string);
    } else {
        if(i2d_buffer_create(&buffer, BUFFER_SIZE_LARGE * 2)) {
            status = i2d_panic("failed to create buffer object");
        } else {
            result = i2d_fd_read(fd, BUFFER_SIZE_LARGE, &buffer);
            while(0 < result && !status) {
                if(i2d_by_line(&buffer, i2d_skill_db_parse, skill_db))
                    status = i2d_panic("failed to parse buffer");
                result = i2d_fd_read(fd, BUFFER_SIZE_LARGE, &buffer);
            }
            if(!status && buffer.offset && i2d_by_line(&buffer, i2d_skill_db_parse, skill_db))
                status = i2d_panic("failed to parse buffer");
            i2d_buffer_destroy(&buffer);
        }
        close(fd);
    }

    return status;
}

static int i2d_skill_db_parse(char * string, size_t length, void * data) {
    int status = I2D_OK;
    i2d_skill_db * skill_db = data;
    i2d_skill * skill = NULL;

    if(i2d_skill_init(&skill, string, length)) {
        status = i2d_panic("failed to create skill object");
    } else {
        if(!skill_db->list) {
            skill_db->list = skill;
        } else {
            i2d_skill_append(skill, skill_db->list);
        }

        skill_db->size++;
    }

    return status;
}

static int i2d_skill_db_index(i2d_skill_db * skill_db) {
    int status = I2D_OK;
    i2d_skill * skill = NULL;

    if( i2d_rbt_init(&skill_db->index_by_id, i2d_rbt_cmp_long) ||
        i2d_rbt_init(&skill_db->index_by_macro, i2d_rbt_cmp_str) ) {
        status = i2d_panic("failed to create red black tree objects");
    } else {
        skill = skill_db->list;
        do {
            if( i2d_rbt_insert(skill_db->index_by_id, &skill->id, skill) ||
                i2d_rbt_insert(skill_db->index_by_macro, skill->macro.string, skill) )
                status = i2d_panic("failed to index skill by id -- %ld", skill->id);
            skill = skill->next;
        } while(skill != skill_db->list && !status);
    }

    return status;
}

int i2d_skill_db_search_by_id(i2d_skill_db * skill_db, long skill_id, i2d_skill ** skill) {
    return i2d_rbt_search(skill_db->index_by_id, &skill_id, (void **) skill);
}

int i2d_skill_db_search_by_macro(i2d_skill_db * skill_db, const char * macro, i2d_skill ** skill) {
    return i2d_rbt_search(skill_db->index_by_macro, macro, (void **) skill);
}
