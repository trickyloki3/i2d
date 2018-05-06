#ifndef i2d_json_h
#define i2d_json_h

#include "i2d_util.h"
#include "jansson.h"

int i2d_json_create(json_t **, i2d_string *);
void i2d_json_destroy(json_t *);
#endif
