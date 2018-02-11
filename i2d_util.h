#ifndef i2d_util_h
#define i2d_util_h

#include "ctype.h"
#include "stdio.h"
#include "fcntl.h"
#include "stdlib.h"
#include "stdarg.h"
#include "string.h"
#include "stdint.h"
#include "inttypes.h"
#include "sys/time.h"

#define I2D_OK      0
#define I2D_FAIL    1

#define i2d_panic(format, ...) i2d_panic_func("%s (%s:%zu): " format ".\n", __FILE__, __func__, __LINE__, ## __VA_ARGS__)
int i2d_panic_func(const char *, ...);

#define i2d_is_invalid(ptr) (!(ptr) || *(ptr))
#define i2d_free(ptr) if(ptr) { free(ptr); (ptr) = NULL; }
#define i2d_deit(ptr, deit) if(ptr) { deit(&(ptr)); }

int i2d_strtol(long *, const char *, size_t, int);
int i2d_strtoul(unsigned long *, const char *, size_t, int);

struct i2d_str {
    char * string;
    size_t length;
};

typedef struct i2d_str i2d_str;

int i2d_str_init(i2d_str **, const char *, size_t);
void i2d_str_deit(i2d_str **);

struct i2d_buf {
    uint8_t * buffer;
    size_t length;
    size_t offset;
};

typedef struct i2d_buf i2d_buf;

int i2d_buf_init(i2d_buf **, size_t);
void i2d_buf_deit(i2d_buf **);
int i2d_buf_fit(i2d_buf *, size_t);
int i2d_buf_format(i2d_buf *, const char *, ...);
int i2d_buf_binary(i2d_buf *, void *, size_t);
int i2d_buf_object(i2d_buf *, size_t, void **);
void i2d_buf_dump(i2d_buf *, const char *);

typedef int (* i2d_by_line_cb) (char *, size_t, void *);

int i2d_fd_read(int, size_t, i2d_buf *);
int i2d_by_line(i2d_buf *, i2d_by_line_cb, void *);
#endif
