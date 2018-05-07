#ifndef i2d_json_h
#define i2d_json_h

#include "i2d_util.h"
#include "i2d_range.h"
#include "jansson.h"

int i2d_json_create(json_t **, i2d_string *);
void i2d_json_destroy(json_t *);

int i2d_object_get_string(json_t *, i2d_string *);
int i2d_object_get_number(json_t *, long *);
int i2d_object_get_range(json_t *, json_t *, i2d_range *);
#endif
