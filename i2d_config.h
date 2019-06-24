#ifndef i2d_config_h
#define i2d_config_h

#include "i2d_util.h"
#include "i2d_json.h"

struct i2d_config {
    i2d_string data_path;
    long item_id;
    i2d_string item_db_path;
    i2d_string skill_db_path;
    i2d_string mob_db_path;
    i2d_string mob_race2_db_path;
    i2d_string produce_db_path;
    i2d_string mercenary_db_path;
    i2d_string pet_db_path;
    i2d_string item_combo_db_path;
};

typedef struct i2d_config i2d_config;

int i2d_config_init(i2d_config **, i2d_string *);
void i2d_config_deit(i2d_config  **);
#endif