#ifndef i2d_db_h
#define i2d_db_h

#include "i2d_item.h"
#include "i2d_skill.h"

enum i2d_db_type {
    i2d_pre_renewal,
    i2d_renewal
};

struct i2d_db {
    enum i2d_db_type type;
    i2d_item_db * item_db;
    i2d_skill_db * skill_db;
};

typedef struct i2d_db i2d_db;

int i2d_db_init(i2d_db **, enum i2d_db_type, i2d_string *);
void i2d_db_deit(i2d_db **);
#endif
