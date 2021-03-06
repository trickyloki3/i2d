#include "i2d_util.h"

static int i2d_string_stack_cmp(const void *, const void *);

int i2d_panic_print(const char * format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    return I2D_FAIL;
}

int i2d_string_copy(char ** result, const char * string, size_t length) {
    int status = I2D_OK;
    char * buffer;

    buffer = calloc(length + 1, sizeof(*string));
    if(!buffer) {
        status = i2d_panic("out of memory");
    } else {
        memcpy(buffer, string, length);
        buffer[length] = 0;
        *result = buffer;
    }

    return status;
}

int i2d_string_create(i2d_string * result, const char * string, size_t length) {
    int status = I2D_OK;

    if(i2d_string_copy(&result->string, string, length)) {
        status = i2d_panic("failed to copy string object");
    } else {
        result->length = length;
    }

    return status;
}

void i2d_string_destroy(i2d_string * result) {
    free(result->string);
}

int i2d_string_vprintf(i2d_string * result, const char * format, ...) {
    int status = I2D_OK;
    va_list vl;
    i2d_buffer buffer;
    i2d_string output;

    va_start(vl, format);
    if(i2d_buffer_create(&buffer, BUFFER_SIZE_SMALL)) {
        status = i2d_panic("failed to create buffer object");
    } else {
        if(i2d_buffer_vprintf(&buffer, format, vl)) {
            status = i2d_panic("failed to write buffer object");
        } else {
            i2d_buffer_get(&buffer, &output.string, &output.length);
            status = i2d_string_create(result, output.string, output.length);
        }
        i2d_buffer_destroy(&buffer);
    }

    va_end(vl);
    return status;
}

int i2d_buffer_init(i2d_buffer ** result, size_t size) {
    int status = I2D_OK;
    i2d_buffer * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_buffer_create(object, size)) {
                status = i2d_panic("failed to create buffer object");
            } else {
                object->next = object;
                object->prev = object;
            }

            if(status) {
                i2d_buffer_deit(&object);
            } else {
                *result = object;
            }
        }
    }

    return status;
}

void i2d_buffer_deit(i2d_buffer ** result) {
    i2d_buffer * object;

    object = *result;
    i2d_buffer_destroy(object);
    i2d_free(object);
    *result = NULL;
}

void i2d_buffer_list_deit(i2d_buffer ** result) {
    i2d_buffer * object;
    i2d_buffer * buffer;

    object = *result;
    while(object != object->next) {
        buffer = object->next;
        i2d_buffer_remove(buffer);
        i2d_buffer_deit(&buffer);
    }
    i2d_buffer_deit(&object);
    *result = NULL;
}

void i2d_buffer_append(i2d_buffer * x, i2d_buffer * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_buffer_remove(i2d_buffer * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

int i2d_buffer_create(i2d_buffer * result, size_t length) {
    int status = I2D_OK;

    if(!length) {
        status = i2d_panic("invalid buffer length");
    } else {
        result->length = length;
        result->buffer = calloc(result->length, sizeof(*result->buffer));
        if(!result->buffer) {
            status = i2d_panic("out of memory");
        } else {
            result->offset = 0;
        }
    }

    return status;
}

void i2d_buffer_destroy(i2d_buffer * result) {
    free(result->buffer);
}

int i2d_buffer_adapt(i2d_buffer * result, size_t length) {
    int status = I2D_OK;
    size_t avail;
    void * buffer;

    avail = result->length - result->offset;
    if(avail < length) {
        length = result->length + (length - avail);
        buffer = realloc(result->buffer, length);
        if(!buffer) {
            status = i2d_panic("out of memory");
        } else {
            result->buffer = buffer;
            result->length = length;
        }
    }

    return status;
}

void i2d_buffer_clear(i2d_buffer * result) {
    if(result->offset) {
        memset(result->buffer, 0, result->offset);
        result->offset = 0;
    }
}

int i2d_buffer_putc(i2d_buffer * result, char character) {
    int status = I2D_OK;

    if( result->length == result->offset + 1 &&
        i2d_buffer_adapt(result, result->length + 1) ) {
        status = I2D_FAIL;
    } else {
        result->buffer[result->offset] = character;
        result->offset++;
        result->buffer[result->offset] = 0;
    }

    return status;
}

int i2d_buffer_printf(i2d_buffer * result, const char * format, ...) {
    int status = I2D_OK;
    va_list vl;

    va_start(vl, format);
    status = i2d_buffer_vprintf(result, format, vl);
    va_end(vl);
    return status;
}

int i2d_buffer_vprintf(i2d_buffer * result, const char * format, va_list vl) {
    int status = I2D_OK;
    va_list vl_copy;

    int length;
    char * string;

    va_copy(vl_copy, vl);
    length = vsnprintf(NULL, 0, format, vl);
    if(0 > length) {
        status = i2d_panic("invalid print format specification");
    } else if(i2d_buffer_adapt(result, length + 1)) {
        status = I2D_FAIL;
    } else {
        string = result->buffer + result->offset;
        if(length != vsnprintf(string, length + 1, format, vl_copy)) {
            status = i2d_panic("failed to print format");
        } else {
            result->offset += length;
        }
    }

    va_end(vl_copy);
    return status;
}

int i2d_buffer_memcpy(i2d_buffer * result, const char * string, size_t length) {
    int status = I2D_OK;

    if(i2d_buffer_adapt(result, length + 1)) {
        status = I2D_FAIL;
    } else {
        memcpy(&result->buffer[result->offset], string, length);
        result->buffer[result->offset + length] = 0;
        result->offset += length;
    }

    return status;
}

void i2d_buffer_get(i2d_buffer * result, char ** string, size_t * length) {
    *string = result->buffer;
    *length = result->offset;
}

int i2d_buffer_cache_init(i2d_buffer_cache ** result) {
    int status = I2D_OK;
    i2d_buffer_cache * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_buffer_init(&object->list, BUFFER_SIZE_SMALL))
                status = i2d_panic("failed to create buffer object");

            if(status)
                i2d_buffer_cache_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

int i2d_buffer_copy(i2d_buffer * result, i2d_buffer * buffer) {
    int status = I2D_OK;
    i2d_string string;
    i2d_zero(string);

    i2d_buffer_get(buffer, &string.string, &string.length);
    i2d_buffer_clear(result);

    if(i2d_buffer_memcpy(result, string.string, string.length))
        status = i2d_panic("failed to copy buffer object");

    return status;
}

void i2d_buffer_cache_deit(i2d_buffer_cache ** result) {
    i2d_buffer_cache * object;

    object = *result;
    i2d_deit(object->list, i2d_buffer_list_deit);
    i2d_free(object);
    *result = NULL;
}

int i2d_buffer_cache_get(i2d_buffer_cache * cache, i2d_buffer ** result) {
    int status = I2D_OK;
    i2d_buffer * buffer;

    if(cache->list != cache->list->next) {
        buffer = cache->list->next;
        i2d_buffer_remove(buffer);
        i2d_buffer_clear(buffer);
        *result = buffer;
    } else {
        status = i2d_buffer_init(result, BUFFER_SIZE_SMALL);
    }

    return status;
}

int i2d_buffer_cache_put(i2d_buffer_cache * cache, i2d_buffer ** result) {
    int status = I2D_OK;
    i2d_buffer * buffer;

    buffer = *result;
    i2d_buffer_remove(buffer);
    i2d_buffer_append(buffer, cache->list);
    *result = NULL;

    return status;
}

int i2d_string_stack_init(i2d_string_stack ** result, size_t size) {
    int status = I2D_OK;
    i2d_string_stack * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_string_stack_create(object, size)) {
                status = i2d_panic("failed to create string stack object");
            } else {
                object->next = object;
                object->prev = object;
            }

            if(status) {
                i2d_string_stack_deit(&object);
            } else {
                *result = object;
            }
        }
    }

    return status;
}

void i2d_string_stack_deit(i2d_string_stack ** result) {
    i2d_string_stack * object;

    object = *result;
    i2d_string_stack_destroy(object);
    i2d_free(object);
    *result = NULL;
}

void i2d_string_stack_list_deit(i2d_string_stack ** result) {
    i2d_string_stack * object;
    i2d_string_stack * stack;

    object = *result;
    while(object != object->next) {
        stack = object->next;
        i2d_string_stack_remove(stack);
        i2d_string_stack_deit(&stack);
    }
    i2d_string_stack_deit(&object);
    *result = NULL;
}

void i2d_string_stack_append(i2d_string_stack * x, i2d_string_stack * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_string_stack_remove(i2d_string_stack * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

int i2d_string_stack_create(i2d_string_stack * result, size_t size) {
    int status = I2D_OK;

    if(!size) {
        status = i2d_panic("invalid stack size");
    } else {
        result->size = size;
        result->top = 0;
        if(i2d_buffer_create(&result->buffer, BUFFER_SIZE_LARGE)) {
            status = I2D_FAIL;
        } else {
            result->list = calloc(result->size, sizeof(*result->list));
            if(!result->list) {
                status = i2d_panic("out of memory");
            } else {
                result->offset = calloc(result->size, sizeof(*result->offset));
                if(!result->offset)
                    status = i2d_panic("out of memory");
                if(status)
                    i2d_free(result->list);
            }
            if(status)
                i2d_buffer_destroy(&result->buffer);
        }
    }

    return status;
}

void i2d_string_stack_destroy(i2d_string_stack * result) {
    i2d_free(result->offset);
    i2d_free(result->list);
    i2d_buffer_destroy(&result->buffer);
}

int i2d_string_stack_push(i2d_string_stack * result, const char * string, size_t length) {
    int status = I2D_OK;

    if(result->top == result->size) {
        status = i2d_panic("string stack overflow");
    } else {
        if( i2d_buffer_memcpy(&result->buffer, string, length) ||
            i2d_buffer_putc(&result->buffer, 0) ) {
            status = I2D_FAIL;
        } else {
            result->offset[result->top] = result->buffer.offset - 1;
            result->top++;
        }
    }

    return status;
}

int i2d_string_stack_push_buffer(i2d_string_stack * result, i2d_buffer * buffer) {
    i2d_string string;

    i2d_buffer_get(buffer, &string.string, &string.length);
    return i2d_string_stack_push(result, string.string, string.length);
}

int i2d_string_stack_pop(i2d_string_stack * result) {
    int status = I2D_OK;

    if(result->top == 0) {
        status = i2d_panic("empty string stack");
    } else {
        result->top--;
        if(result->top == 0) {
            result->buffer.offset = 0;
        } else {
            result->buffer.offset = result->offset[result->top - 1] + 1;
        }
    }

    return status;
}

void i2d_string_stack_clear(i2d_string_stack * result) {
    if(result->top) {
        i2d_buffer_clear(&result->buffer);
        memset(result->list, 0, sizeof(*result->list) * result->top);
        memset(result->offset, 0, sizeof(*result->offset) * result->top);
        result->top = 0;
    }
}

int i2d_string_stack_get(i2d_string_stack * stack, i2d_string ** list, size_t * size) {
    int status = I2D_OK;
    size_t i;
    size_t offset;

    if(stack->top == 0) {
        *list = NULL;
        *size = 0;
    } else {
        for(i = 0; i < stack->top; i++) {
            if(i == 0) {
                stack->list[i].string = stack->buffer.buffer;
                stack->list[i].length = stack->offset[i];
            } else {
                offset = stack->offset[i - 1] + 1;
                stack->list[i].string = &stack->buffer.buffer[offset];
                stack->list[i].length = stack->offset[i] - stack->offset[i - 1] - 1;
            }
        }

        *list = stack->list;
        *size = stack->top;
    }

    return status;
}

static int i2d_string_stack_cmp(const void * left, const void * right) {
    return strcmp(((i2d_string *) left)->string, ((i2d_string *) right)->string);
}

int i2d_string_stack_get_sorted(i2d_string_stack * stack, i2d_string ** list, size_t * size) {
    int status = I2D_OK;

    status = i2d_string_stack_get(stack, list, size);
    if(!status)
        qsort(*list, *size, sizeof(**list), i2d_string_stack_cmp);

    return status;
}

int i2d_string_stack_dump_buffer(i2d_string_stack * stack, i2d_buffer * buffer, const char * delimit) {
    int status = I2D_OK;
    size_t i;
    size_t size;
    size_t last;
    i2d_string * list;

    if(i2d_string_stack_get(stack, &list, &size)) {
        status = i2d_panic("failed to get string list");
    } else {
        for(i = 0, last = 0; i < size && !status; i++) {
            if(list[i].length) {
                if( (i && list[last].length && i2d_buffer_printf(buffer, delimit)) ||
                    i2d_buffer_printf(buffer, "%s", list[i].string) ) {
                    status = i2d_panic("failed to write buffer object");
                } else {
                    last = i;
                }
            }
        }
    }

    return status;
}

int i2d_string_stack_format(i2d_string_stack * stack, i2d_string * format, i2d_buffer * result) {
    int status = I2D_OK;

    i2d_string * list;
    size_t size;

    i2d_buffer buffer;
    i2d_string string;
    long position;

    size_t i;
    char symbol;
    int level = 0;

    if(i2d_string_stack_get(stack, &list, &size)) {
        status = i2d_panic("failed to get string stack");
    } else if(i2d_buffer_create(&buffer, BUFFER_SIZE_SMALL)) {
        status = i2d_panic("failed to create buffere object");
    } else {
        for(i = 0; i < format->length && !status; i++) {
            symbol = format->string[i];
            switch(symbol) {
                case '{':
                    if(level) {
                        status = i2d_panic("invalid starting curly");
                    } else {
                        level++;
                    }
                    break;
                case '}':
                    if(!level) {
                        status = i2d_panic("invalid ending curly");
                    } else {
                        level--;

                        i2d_buffer_get(&buffer, &string.string, &string.length);
                        if(i2d_strtol(&position, string.string, string.length, 10)) {
                            status = i2d_panic("invalid number string -- %s", string.string);
                        } else if(position < 0 || (size_t) position >= size) {
                            status = i2d_panic("invalid position on string stack");
                        } else if(i2d_buffer_printf(result, "%s", list[position].string)) {
                            status = i2d_panic("failed to write buffer");
                        }
                        i2d_buffer_clear(&buffer);
                    }
                    break;
                default:
                    if(level) {
                        if(i2d_buffer_putc(&buffer, symbol))
                            status = i2d_panic("failed to write buffer object");
                    } else {
                        if(i2d_buffer_putc(result, symbol))
                            status = i2d_panic("failed to write buffer object");
                    }
            }
        }
        i2d_buffer_destroy(&buffer);
    }

    return status;
}

int i2d_string_stack_cache_init(i2d_string_stack_cache ** result) {
    int status = I2D_OK;
    i2d_string_stack_cache * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_string_stack_init(&object->list, MAX_STACK))
                status = i2d_panic("failed to create stack object");

            if(status)
                i2d_string_stack_cache_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_string_stack_cache_deit(i2d_string_stack_cache ** result) {
    i2d_string_stack_cache * object;

    object = *result;
    i2d_deit(object->list, i2d_string_stack_list_deit);
    i2d_free(object);
    *result = NULL;
}

int i2d_string_stack_cache_get(i2d_string_stack_cache * cache, i2d_string_stack ** result) {
    int status = I2D_OK;
    i2d_string_stack * stack;

    if(cache->list != cache->list->next) {
        stack = cache->list->next;
        i2d_string_stack_remove(stack);
        i2d_string_stack_clear(stack);
        *result = stack;
    } else {
        status = i2d_string_stack_init(result, MAX_STACK);
    }

    return status;
}

int i2d_string_stack_cache_put(i2d_string_stack_cache * cache, i2d_string_stack ** result) {
    int status = I2D_OK;
    i2d_string_stack * stack;

    stack = *result;
    i2d_string_stack_remove(stack);
    i2d_string_stack_append(stack, cache->list);
    *result = NULL;

    return status;
}

int i2d_strtol(long * result, const char * string, size_t length, int base) {
    int status = I2D_OK;

    long number;
    char * end = NULL;

    if(!length) {
        *result = 0;
    } else {
        number = strtol(string, &end, base);
        if(string + length != end) {
            status = i2d_panic("invalid string '%s' in '%s'", end, string);
        } else {
            *result = number;
        }
    }

    return status;
}

int i2d_strtoll(long long * result, const char * string, size_t length, int base) {
    int status = I2D_OK;

    long long number;
    char * end = NULL;

    if(!length) {
        *result = 0;
    } else {
        number = strtoll(string, &end, base);
        if(string + length != end) {
            status = i2d_panic("invalid string '%s' in '%s'", end, string);
        } else {
            *result = number;
        }
    }

    return status;
}

int i2d_strtoul(unsigned long * result, const char * string, size_t length, int base) {
    int status = I2D_OK;

    unsigned long number;
    char * end = NULL;

    if(!length) {
        *result = 0;
    } else {
        number = strtoul(string, &end, base);
        if(string + length != end) {
            status = i2d_panic("invalid string '%s' in '%s'", end, string);
        } else {
            *result = number;
        }
    }

    return status;
}

int i2d_strtod(double * result, const char * string, size_t length) {
    int status = I2D_OK;

    double number;
    char * end = NULL;

    if(!length) {
        *result = 0;
    } else {
        number = strtod(string, &end);
        if(string + length != end) {
            status = i2d_panic("invalid string '%s' in '%s'", end, string);
        } else {
            *result = number;
        }
    }

    return status;
}

#ifndef _WIN32
int i2d_fd_load(i2d_string * path, i2d_by_line_cb cb, void * data) {
    int status = I2D_OK;

    int fd;
    i2d_buffer buffer;
    int result;

    fd = open(path->string, O_RDONLY);
    if(0 > fd) {
        status = i2d_panic("failed to open file -- %s", path->string);
    } else {
        if(i2d_buffer_create(&buffer, BUFFER_SIZE_LARGE * 2)) {
            status = i2d_panic("failed to create buffer object");
        } else {
            result = i2d_fd_read(fd, BUFFER_SIZE_LARGE, &buffer);
            while(0 < result && !status) {
                if(i2d_by_line(&buffer, cb, data))
                    status = i2d_panic("failed to parse buffer");
                result = i2d_fd_read(fd, BUFFER_SIZE_LARGE, &buffer);
            }
            if(!status && buffer.offset && i2d_by_line(&buffer, cb, data))
                status = i2d_panic("failed to parse buffer");
            i2d_buffer_destroy(&buffer);
        }
        close(fd);
    }

    return status;
}
#else
int i2d_fd_load(i2d_string * path, i2d_by_line_cb cb, void * data) {
    int status = I2D_OK;

    HANDLE hFile;
    i2d_buffer buffer;
    int result;

    hFile = CreateFile(path->string, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(INVALID_HANDLE_VALUE == hFile) {
        status = i2d_panic("failed to open file -- %s", path->string);
    } else {
        if(i2d_buffer_create(&buffer, BUFFER_SIZE_LARGE * 2)) {
            status = i2d_panic("failed to create buffer object");
        } else {
            result = i2d_fd_read(hFile, BUFFER_SIZE_LARGE, &buffer);
            while(0 < result && !status) {
                if(i2d_by_line(&buffer, cb, data))
                    status = i2d_panic("failed to parse buffer");
                result = i2d_fd_read(hFile, BUFFER_SIZE_LARGE, &buffer);
            }
            if(!status && buffer.offset && i2d_by_line(&buffer, cb, data))
                status = i2d_panic("failed to parse buffer");
            i2d_buffer_destroy(&buffer);
        }
        CloseHandle(hFile);
    }

    return status;
}
#endif

#ifndef _WIN32
int i2d_fd_read(int fd, size_t size, i2d_buffer * buffer) {
    int status = I2D_OK;
    ssize_t result;

    if(i2d_buffer_adapt(buffer, size + 1)) {
        status = I2D_FAIL;
    } else {
        result = read(fd, buffer->buffer + buffer->offset, size);
        if(0 > result) {
            status = i2d_panic("failed on read");
        } else {
            buffer->offset += result;
            buffer->buffer[buffer->offset] = 0;
        }
    }

    return status ? -1 : result;
}
#else
int i2d_fd_read(HANDLE hFile, size_t size, i2d_buffer * buffer) {
    int status = I2D_OK;
    int result;
    DWORD dwBytes;

    if(i2d_buffer_adapt(buffer, size + 1)) {
        status = I2D_FAIL;
    } else {
        if(!ReadFile(hFile, buffer->buffer + buffer->offset, size, &dwBytes, NULL)) {
            status = i2d_panic("failed on read");
        } else {
            buffer->offset += dwBytes;
            buffer->buffer[buffer->offset] = 0;

            /*
             * check integer limit before downcasting
             */
            if(dwBytes > INT_MAX) {
                status = i2d_panic("integer overflow");
            } else {
                result = (int) dwBytes;
            }
        }
    }

    return status ? -1 : result;
}
#endif

int i2d_by_line(i2d_buffer * buffer, i2d_by_line_cb cb, void * data) {
    int status = I2D_OK;
    char * anchor;
    char * delimit;
    size_t length;
    size_t consume;

    anchor = (char *) buffer->buffer;
    delimit = strchr(anchor, '\n');
    if(delimit) {
        while(delimit && !status) {
            /*
             * skip initial whitespace
             */
            while(i2d_isspace(*anchor))
                anchor++;

            /*
             * skip empty lines
             */
            if(delimit > anchor) {
                length = (size_t) delimit - (size_t) anchor + 1;

                /*
                 * skip comments
                 */
                if(i2d_isalnum(*anchor))
                    status = cb(anchor, length, data);
            }

            anchor = delimit + 1;
            delimit = strchr(anchor, '\n');
        }

        consume = (size_t) anchor - (size_t) buffer->buffer;
        if(0 == consume) {
            status = i2d_panic("line overflow");
        } else if(buffer->offset < consume) {
            status = i2d_panic("buffer overflow");
        } else {
            if(buffer->offset > consume)
                memcpy(buffer->buffer, buffer->buffer + consume, buffer->offset - consume);

            buffer->offset -= consume;
            buffer->buffer[buffer->offset]= 0;
        }
    }

    return status;
}

int i2d_by_bit64(uint64_t flag, i2d_by_bit_cb cb, void * context) {
    int status = I2D_OK;
    uint64_t bit = 1;

    while(bit && !status) {
        if(flag & bit) {
            status = cb(bit, context);
            flag &= ~bit;
        }
        bit <<= 1;
    }

    return status;
}

int i2d_is_number(i2d_string * string) {
    int status = I2D_OK;
    size_t i = 0;

    if(string->length > 1 && !strncmp("0x", string->string, 2))
        i = 2;

    for(; i < string->length && !status; i++)
        if(!i2d_isdigit(string->string[i]))
            status = I2D_FAIL;

    return status;
}
