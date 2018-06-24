#ifndef i2d_util_h
#define i2d_util_h

#include "math.h"
#include "ctype.h"
#include "stdio.h"
#include "fcntl.h"
#include "stdlib.h"
#include "stdarg.h"
#include "string.h"
#include "stdint.h"
#include "unistd.h"
#include "limits.h"
#include "inttypes.h"
#include "sys/time.h"
#if i2d_debug
#include "assert.h"
#endif

#define I2D_OK 0
#define I2D_FAIL 1
#define I2D_SIZE 4096
#define I2D_SIZE_SMALL 64
#define I2D_STACK 64

#define i2d_panic(format, ...) i2d_panic_print("%s (%s:%zu): " format ".\n", __FILE__, __func__, __LINE__, ## __VA_ARGS__)
int i2d_panic_print(const char *, ...);

#define i2d_is_invalid(ptr) (!(ptr) || *(ptr))
#define i2d_free(ptr) if(ptr) { free(ptr); (ptr) = NULL; }
#define i2d_deit(ptr, deit) if(ptr) { deit(&(ptr)); }
#define i2d_zero(str) memset(&str, 0, sizeof(str))
#define i2d_size(ptr) (sizeof(ptr) / sizeof(ptr[0]))

#define is_negative(x) ((x) <  0)
#define is_positive(x) ((x) >= 0)

#ifndef min
#define min(x, y) ((x < y) ? x : y)
#endif

#ifndef max
#define max(x, y) ((x < y) ? y : x)
#endif

struct i2d_string {
    char * string;
    size_t length;
};

typedef struct i2d_string i2d_string;

int i2d_string_create(i2d_string *, const char *, size_t);
void i2d_string_destroy(i2d_string *);
int i2d_string_vprintf(i2d_string *, const char *, ...);

struct i2d_buffer {
    char * buffer;
    size_t length;
    size_t offset;
    struct i2d_buffer * next;
    struct i2d_buffer * prev;
};

typedef struct i2d_buffer i2d_buffer;

int i2d_buffer_init(i2d_buffer **, size_t);
void i2d_buffer_deit(i2d_buffer **);
void i2d_buffer_list_deit(i2d_buffer **);
void i2d_buffer_append(i2d_buffer *, i2d_buffer *);
void i2d_buffer_remove(i2d_buffer *);
int i2d_buffer_create(i2d_buffer *, size_t);
void i2d_buffer_destroy(i2d_buffer *);
int i2d_buffer_adapt(i2d_buffer *, size_t);
void i2d_buffer_clear(i2d_buffer *);
int i2d_buffer_putc(i2d_buffer *, char);
int i2d_buffer_printf(i2d_buffer *, const char *, ...);
int i2d_buffer_vprintf(i2d_buffer *, const char *, va_list);
int i2d_buffer_memcpy(i2d_buffer *, const char *, size_t);
void i2d_buffer_get(i2d_buffer *, char **, size_t *);

struct i2d_buffer_cache {
    i2d_buffer * list;
};

typedef struct i2d_buffer_cache i2d_buffer_cache;

int i2d_buffer_cache_init(i2d_buffer_cache **);
void i2d_buffer_cache_deit(i2d_buffer_cache **);
int i2d_buffer_cache_get(i2d_buffer_cache *, i2d_buffer **);
int i2d_buffer_cache_put(i2d_buffer_cache *, i2d_buffer **);

struct i2d_string_stack {
    i2d_buffer buffer;
    i2d_string * list;
    size_t * offset;
    size_t size;
    size_t top;
    struct i2d_string_stack * next;
    struct i2d_string_stack * prev;
};

typedef struct i2d_string_stack i2d_string_stack;

int i2d_string_stack_init(i2d_string_stack **, size_t);
void i2d_string_stack_deit(i2d_string_stack **);
void i2d_string_stack_list_deit(i2d_string_stack **);
void i2d_string_stack_append(i2d_string_stack *, i2d_string_stack *);
void i2d_string_stack_remove(i2d_string_stack *);
int i2d_string_stack_create(i2d_string_stack *, size_t);
void i2d_string_stack_destroy(i2d_string_stack *);
int i2d_string_stack_push(i2d_string_stack *, const char *, size_t);
int i2d_string_stack_push_buffer(i2d_string_stack *, i2d_buffer *);
int i2d_string_stack_pop(i2d_string_stack *);
void i2d_string_stack_clear(i2d_string_stack *);
int i2d_string_stack_get(i2d_string_stack *, i2d_string **, size_t *);
int i2d_string_stack_get_sorted(i2d_string_stack *, i2d_string **, size_t *);
int i2d_string_stack_get_unique(i2d_string_stack *, i2d_buffer *);

struct i2d_string_stack_cache {
    i2d_string_stack * list;
};

typedef struct i2d_string_stack_cache i2d_string_stack_cache;

int i2d_string_stack_cache_init(i2d_string_stack_cache **);
void i2d_string_stack_cache_deit(i2d_string_stack_cache **);
int i2d_string_stack_cache_get(i2d_string_stack_cache *, i2d_string_stack **);
int i2d_string_stack_cache_put(i2d_string_stack_cache *, i2d_string_stack **);

int i2d_strtol(long *, const char *, size_t, int);
int i2d_strtoul(unsigned long *, const char *, size_t, int);

typedef int (* i2d_by_line_cb) (char *, size_t, void *);

int i2d_fd_read(int, size_t, i2d_buffer *);
int i2d_by_line(i2d_buffer *, i2d_by_line_cb, void *);

typedef int (* i2d_by_bit_cb) (uint64_t, void *);

int i2d_by_bit64(uint64_t, i2d_by_bit_cb, void *);
#endif
