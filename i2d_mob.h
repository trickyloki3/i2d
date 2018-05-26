#ifndef i2d_mob_h
#define i2d_mob_h

#include "i2d_util.h"
#include "i2d_rbt.h"

struct i2d_mob {
    long id;
    i2d_string sprite;
    i2d_string kro;
    i2d_string iro;
    long level;
    long hp;
    long sp;
    long exp;
    long jexp;
    long range1;
    long atk1;
    long atk2;
    long def;
    long mdef;
    long str;
    long agi;
    long vit;
    long inte;
    long dex;
    long luk;
    long range2;
    long range3;
    long scale;
    long race;
    long element;
    long mode;
    long speed;
    long adelay;
    long amotion;
    long dmotion;
    long mexp;
    long mvp_drop_id[3];
    long mvp_drop_pre[3];
    long drop_id[9];
    long drop_pre[9];
    long drop_card_id;
    long drop_card_per;
    struct i2d_mob * next;
    struct i2d_mob * prev;
};

typedef struct i2d_mob i2d_mob;

int i2d_mob_init(i2d_mob **, char *, size_t);
void i2d_mob_deit(i2d_mob **);
void i2d_mob_append(i2d_mob *, i2d_mob *);
void i2d_mob_remove(i2d_mob *);

struct i2d_mob_db {
    i2d_mob * list;
    size_t size;
};

typedef struct i2d_mob_db i2d_mob_db;

int i2d_mob_db_init(i2d_mob_db **, i2d_string *);
void i2d_mob_db_deit(i2d_mob_db **);
#endif
