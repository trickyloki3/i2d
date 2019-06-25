#ifndef i2d_yaml_h
#define i2d_yaml_h

#include "i2d_util.h"
#include "yaml.h"

typedef int (* i2d_yaml_parse_cb)(yaml_event_t *, void *);
int i2d_yaml_parse(i2d_string *, i2d_yaml_parse_cb, void *);
#endif
