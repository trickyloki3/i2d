#include "i2d_script.h"

static int i2d_rbt_add_variable(i2d_rbt *, i2d_node *);
static int i2d_rbt_get_variable(i2d_rbt *, i2d_node *, i2d_node **);

struct i2d_local {
    i2d_buffer * buffer;
    i2d_string_stack * stack;
};

typedef struct i2d_local i2d_local;

int i2d_local_create(i2d_local *, i2d_script *);
int i2d_local_destroy(i2d_local *, i2d_script *);

typedef int (* i2d_handler_single_node_cb) (i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
typedef int (* i2d_handler_multiple_node_cb) (i2d_script *, i2d_rbt *, i2d_node **, i2d_local *);
typedef int (* i2d_handler_single_node_data_cb) (i2d_data *, i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
typedef int (* i2d_handler_block_statement_cb) (i2d_script *, i2d_block *, i2d_rbt *, i2d_data *);

enum i2d_handler_type {
    single_node,
    multiple_node,
    single_node_data,
    block_statement
};

struct i2d_handler {
    char * name;
    enum i2d_handler_type type;
    union {
        void * ptr;
        i2d_handler_single_node_cb single_node;
        i2d_handler_multiple_node_cb multiple_node;
        i2d_handler_single_node_data_cb single_node_data;
        i2d_handler_block_statement_cb block_statement;
    };
    i2d_data * data;
    struct i2d_handler * next;
    struct i2d_handler * prev;
};

typedef struct i2d_handler i2d_handler;

static int i2d_handler_init(i2d_handler **, enum i2d_handler_type, i2d_data *, void *);
static void i2d_handler_deit(i2d_handler **);
static void i2d_handler_list_deit(i2d_handler **);
static void i2d_handler_append(i2d_handler *, i2d_handler *);
static void i2d_handler_remove(i2d_handler *);
static int i2d_handler_list_append(i2d_handler **, enum i2d_handler_type, i2d_data *, void *);

static int i2d_handler_general(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_readparam(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_getskilllv(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_isequipped(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_countitem(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_gettime(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_strcharinfo(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_getequipid(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_getiteminfo(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_getmapflag(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_max(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_min(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_getequiprefinerycnt_cb(i2d_script *, i2d_string_stack *, long);
static int i2d_handler_getequiprefinerycnt(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_pow(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_checkoption_loop(uint64_t, void *);
static int i2d_handler_checkoption(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_rand(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_callfunc(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_getequipweaponlv(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_getexp2(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);

i2d_handler function_handlers[] = {
    { "getrefine", single_node, {i2d_handler_general}},
    { "readparam", single_node, {i2d_handler_readparam}},
    { "getskilllv", single_node, {i2d_handler_getskilllv}},
    { "isequipped", single_node, {i2d_handler_isequipped}},
    { "getpartnerid", single_node, {i2d_handler_general}},
    { "checkmadogear", single_node, {i2d_handler_general}},
    { "eaclass", single_node, {i2d_handler_general}},
    { "countitem", single_node, {i2d_handler_countitem}},
    { "gettime", single_node, {i2d_handler_gettime}},
    { "strcharinfo", single_node, {i2d_handler_strcharinfo}},
    { "getequipid", single_node, {i2d_handler_getequipid}},
    { "getiteminfo", single_node, {i2d_handler_getiteminfo}},
    { "getmapflag", single_node, {i2d_handler_getmapflag}},
    { "max", single_node, {i2d_handler_max}},
    { "min", single_node, {i2d_handler_min}},
    { "getequiprefinerycnt", single_node, {i2d_handler_getequiprefinerycnt}},
    { "pow", single_node, {i2d_handler_pow}},
    { "checkoption", single_node, {i2d_handler_checkoption}},
    { "rand", single_node, {i2d_handler_rand}},
    { "callfunc", single_node, {i2d_handler_callfunc}},
    { "ismounting", single_node, {i2d_handler_general}},
    { "setmounting", single_node, {i2d_handler_general}},
    { "getequipweaponlv", single_node, {i2d_handler_getequipweaponlv}},
    { "getexp2", single_node, {i2d_handler_getexp2}},
    { "getcharid", single_node, {i2d_handler_general}},
    { "checkfalcon", single_node, {i2d_handler_general}}
};

typedef int (*i2d_handler_range_cb)(i2d_script *, i2d_string_stack *, long);
static int i2d_handler_range(i2d_script *, i2d_node *, i2d_local *, i2d_handler_range_cb);
static int i2d_handler_expression(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_milliseconds(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_seconds(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_regen(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_splash(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_elements_cb(i2d_script *, i2d_string_stack *, long);
static int i2d_handler_elements(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_races_cb(i2d_script *, i2d_string_stack *, long);
static int i2d_handler_races(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_classes_cb(i2d_script *, i2d_string_stack *, long);
static int i2d_handler_classes(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_integer(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_integer_sign(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_integer_absolute(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_percent(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_percent_sign(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_percent_sign_inverse(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_percent_absolute(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_percent10(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_percent100(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_ignore(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_sizes_cb(i2d_script *, i2d_string_stack *, long);
static int i2d_handler_sizes(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_skill_cb(i2d_script *, i2d_string_stack *, long);
static int i2d_handler_skill(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_mob_cb(i2d_script *, i2d_string_stack *, long);
static int i2d_handler_mob(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_effects_cb(i2d_script *, i2d_string_stack *, long);
static int i2d_handler_effects(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_mob_races_cb(i2d_script *, i2d_string_stack *, long);
static int i2d_handler_mob_races(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_weapons_cb(i2d_script *, i2d_string_stack *, long);
static int i2d_handler_weapons(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_zeny(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_item_cb(i2d_script *, i2d_string_stack *, long);
static int i2d_handler_item(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_itemgroups_cb(i2d_script *, i2d_string_stack *, long);
static int i2d_handler_itemgroups(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_bf_type(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_bf_damage(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_atf_target(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_atf_type(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_script(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_skill_flags(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_string(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_searchstore_effect(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_announce_flag(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_mercenary_cb(i2d_script *, i2d_string_stack *, long);
static int i2d_handler_mercenary(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_bonus_script_flag_cb(uint64_t, void *);
static int i2d_handler_bonus_script_flag(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_pet_cb(i2d_script *, i2d_string_stack *, long);
static int i2d_handler_pet(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_pet_script(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_pet_loyal_script(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_produce(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_sc_end(i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_custom(i2d_data *, i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_prefix(i2d_data *, i2d_script *, i2d_rbt *, i2d_node *, i2d_local *);
static int i2d_handler_bonus(i2d_script *, i2d_rbt *, i2d_node **, i2d_local *);
static int i2d_handler_bonus2(i2d_script *, i2d_rbt *, i2d_node **, i2d_local *);
static int i2d_handler_bonus3(i2d_script *, i2d_rbt *, i2d_node **, i2d_local *);
static int i2d_handler_bonus4(i2d_script *, i2d_rbt *, i2d_node **, i2d_local *);
static int i2d_handler_bonus5(i2d_script *, i2d_rbt *, i2d_node **, i2d_local *);
static int i2d_handler_sc_start(i2d_script *, i2d_rbt *, i2d_node **, i2d_local *);
static int i2d_handler_sc_start2(i2d_script *, i2d_rbt *, i2d_node **, i2d_local *);
static int i2d_handler_sc_start4(i2d_script *, i2d_rbt *, i2d_node **, i2d_local *);

i2d_handler argument_handlers[] = {
    { "milliseconds", single_node, {i2d_handler_milliseconds} },
    { "seconds", single_node, {i2d_handler_seconds} },
    { "regen", single_node, {i2d_handler_regen} },
    { "splash", single_node, {i2d_handler_splash} },
    { "elements", single_node, {i2d_handler_elements} },
    { "races", single_node, {i2d_handler_races} },
    { "classes", single_node, {i2d_handler_classes} },
    { "integer", single_node, {i2d_handler_integer} },
    { "integer_sign", single_node, {i2d_handler_integer_sign} },
    { "integer_absolute", single_node, {i2d_handler_integer_absolute} },
    { "percent", single_node, {i2d_handler_percent} },
    { "percent_sign", single_node, {i2d_handler_percent_sign} },
    { "percent_sign_inverse", single_node, {i2d_handler_percent_sign_inverse} },
    { "percent_absolute", single_node, {i2d_handler_percent_absolute} },
    { "percent10", single_node, {i2d_handler_percent10} },
    { "percent100", single_node, {i2d_handler_percent100} },
    { "ignore", single_node, {i2d_handler_ignore} },
    { "sizes", single_node, {i2d_handler_sizes} },
    { "skill", single_node, {i2d_handler_skill} },
    { "mob", single_node, {i2d_handler_mob} },
    { "effects", single_node, {i2d_handler_effects} },
    { "mob_races", single_node, {i2d_handler_mob_races} },
    { "weapons", single_node, {i2d_handler_weapons} },
    { "zeny", single_node, {i2d_handler_zeny} },
    { "item", single_node, {i2d_handler_item} },
    { "itemgroups", single_node, {i2d_handler_itemgroups} },
    { "bf_type", single_node, {i2d_handler_bf_type} },
    { "bf_damage", single_node, {i2d_handler_bf_damage} },
    { "atf_target", single_node, {i2d_handler_atf_target} },
    { "atf_type", single_node, {i2d_handler_atf_type} },
    { "script", single_node, {i2d_handler_script} },
    { "skill_flags", single_node, {i2d_handler_skill_flags} },
    { "string", single_node, {i2d_handler_string} },
    { "searchstore_effect", single_node, {i2d_handler_searchstore_effect} },
    { "announce_flag", single_node, {i2d_handler_announce_flag} },
    { "mercenary", single_node, {i2d_handler_mercenary} },
    { "bonus_script_flag", single_node, {i2d_handler_bonus_script_flag} },
    { "pet", single_node, {i2d_handler_pet} },
    { "pet_script", single_node, {i2d_handler_pet_script} },
    { "pet_loyal_script", single_node, {i2d_handler_pet_loyal_script} },
    { "produce", single_node, {i2d_handler_produce} },
    { "sc_end", single_node, {i2d_handler_sc_end} },
    { "custom", single_node_data, {i2d_handler_custom} },
    { "prefix", single_node_data, {i2d_handler_prefix} },
    { "bonus", multiple_node, {i2d_handler_bonus} },
    { "bonus2", multiple_node, {i2d_handler_bonus2} },
    { "bonus3", multiple_node, {i2d_handler_bonus3} },
    { "bonus4", multiple_node, {i2d_handler_bonus4} },
    { "bonus5", multiple_node, {i2d_handler_bonus5} },
    { "sc_start", multiple_node, {i2d_handler_sc_start} },
    { "sc_start2", multiple_node, {i2d_handler_sc_start2} },
    { "sc_start4", multiple_node, {i2d_handler_sc_start4} }
};

i2d_handler statement_handlers[] = {
    { "ignore", block_statement, {i2d_script_statement_ignore} },
    { "set", block_statement, {i2d_script_statement_set} },
    { "generic", block_statement, {i2d_script_statement_generic} }
};

const char * i2d_token_string[] = {
    "token",
    "{",
    "}",
    "(",
    ")",
    "[",
    "]",
    ",",
    ";",
    "id",
    "@",
    "$",
    "$@",
    ".",
    ".@",
    "'",
    "#",
    "##",
    "+",
    "-",
    "+",
    "-",
    "*",
    "/",
    "%",
    "+=",
    "-=",
    "*=",
    "/=",
    "%=",
    ">",
    "<",
    "!",
    "==",
    ">=",
    "<=",
    "!=",
    ">>",
    "<<",
    "&",
    "|",
    "^",
    "~",
    ">>=",
    "<<=",
    "&=",
    "|=",
    "^=",
    "&&",
    "||",
    "?",
    ":",
    "::",
    "=",
    "//",
    "/**/",
    "\"\"",
    "pos",
    "++",
    "--"
};

int i2d_token_precedence[] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    15, /* , */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    4, /* + binary */
    4, /* - binary */
    2, /* +  unary*/
    2, /* -  unary*/
    3, /* * */
    3, /* / */
    3, /* % */
    14, /* += */
    14, /* -= */
    14, /* *= */
    14, /* /= */
    14, /* %= */
    6, /* > */
    6, /* < */
    2, /* ! */
    7, /* == */
    6, /* >= */
    6, /* <= */
    7, /* != */
    5, /* >> */
    5, /* << */
    8, /* & */
    10, /* | */
    9, /* ^ */
    2, /* ~ */
    14, /* >>= */
    14, /* <<= */
    14, /* &= */
    14, /* |= */
    14, /* ^= */
    11, /* && */
    12, /* || */
    13, /* ? */
    13, /* : */
    0,
    14, /* = */
    0,
    0,
    0,
    0,
    2, /* ++ */
    2  /* -- */
};

int i2d_token_right_to_left[] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1, /* += */
    1, /* -= */
    1, /* *= */
    1, /* /= */
    1, /* %= */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1, /* >>= */
    1, /* <<= */
    1, /* &= */
    1, /* |= */
    1, /* ^= */
    0,
    0,
    0,
    0,
    0,
    1, /* = */
    0,
    0,
    0,
    0,
    0,
    0,
};

int i2d_token_init(i2d_token ** result, enum i2d_token_type type) {
    int status = I2D_OK;
    i2d_token * object = NULL;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_buffer_create(&object->buffer, 256)) {
                status = i2d_panic("failed to create buffer object");
            } else {
                object->type = type;
                object->next = object;
                object->prev = object;
            }

            if(status) {
                i2d_token_deit(&object);
            } else {
                *result = object;
            }
        }
    }

    return status;
}

void i2d_token_deit(i2d_token ** result) {
    i2d_token * object;

    object = *result;
    i2d_buffer_destroy(&object->buffer);
    i2d_free(object);
    *result = NULL;
}

void i2d_token_list_deit(i2d_token ** result) {
    i2d_token * object;
    i2d_token * token;

    object = *result;
    if(object) {
        while(object != object->next) {
            token = object->next;
            i2d_token_remove(token);
            i2d_token_deit(&token);
        }
        i2d_token_deit(&object);
    }
    *result = NULL;
}

void i2d_token_append(i2d_token * x, i2d_token * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_token_remove(i2d_token * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

void i2d_token_print(i2d_token * token) {
    i2d_token * iterator;
    i2d_string string;

    iterator = token;
    do {
        if(!i2d_token_get_string(iterator, &string)) {
            fprintf(stdout, "%s(%s) ", i2d_token_string[iterator->type], string.string);
        } else {
            fprintf(stdout, "%s ", i2d_token_string[iterator->type]);
        }
        iterator = iterator->next;
    } while(iterator != token);
}

char i2d_token_getc(i2d_token * token) {
    return token->buffer.offset > 0 ? token->buffer.buffer[token->buffer.offset - 1] : 0;
}

int i2d_token_putc(i2d_token * token, char character) {
    return i2d_buffer_putc(&token->buffer, character);
}

int i2d_token_get_string(i2d_token * token, i2d_string * result) {
    int status = I2D_OK;

    if(I2D_LITERAL != token->type && I2D_POSITION != token->type) {
        status = I2D_FAIL;
    } else {
        i2d_buffer_get(&token->buffer, &result->string, &result->length);
    }

    return status;
}

int i2d_token_set_string(i2d_token * token, i2d_string * string) {
    int status = I2D_OK;

    i2d_buffer_clear(&token->buffer);
    if(i2d_buffer_printf(&token->buffer, "%s", string->string))
        status = i2d_panic("failed to copy string");

    return status;
}

int i2d_token_get_constant(i2d_token * token, long * result) {
    int status = I2D_OK;
    i2d_string string;
    int base;

    if(i2d_token_get_string(token, &string)) {
        status = i2d_panic("failed to get token string");
    } else {
        if(!strncmp(string.string, "0x", 2) && string.length > 2) {
            base = 16;
        } else if(string.string[0] == '0' && string.length > 1) {
            base = 8;
        } else {
            base = 10;
        }

        status = i2d_strtol(result, string.string, string.length, base);
    }

    return status;
}

int i2d_format_create(i2d_format * result, const char * string, size_t length) {
    int status = I2D_OK;

    if(i2d_format_tokenize(string, length, &result->tokens))
        status = i2d_panic("failed to parse format -- %s", string);

    if(status)
        i2d_format_destroy(result);

    return status;
}

int i2d_format_create_json(i2d_format * result, json_t * json) {
    int status = I2D_OK;
    i2d_string format;
    i2d_zero(format);

    if(i2d_object_get_string(json, &format)) {
        status = i2d_panic("failed to get string object");
    } else {
        status = i2d_format_create(result, format.string, format.length);

        i2d_string_destroy(&format);
    }

    return status;
}

void i2d_format_destroy(i2d_format * result) {
    i2d_deit(result->tokens, i2d_token_list_deit);
}

int i2d_format_tokenize(const char * string, size_t length, i2d_token ** result) {
    int status = I2D_OK;

    size_t i;
    char symbol;
    i2d_token * root = NULL;
    i2d_token * token = NULL;
    i2d_token * state = NULL;
    int curly_level = 0;

    if(i2d_token_init(&root, I2D_TOKEN)) {
        status = i2d_panic("failed to create token object");
    } else {
        for(i = 0; i < length && !status; i++) {
            symbol = string[i];
            switch(symbol) {
                case '{':
                    if(curly_level) {
                        status = i2d_panic("invalid position level");
                    } else {
                        curly_level++;
                        state = NULL;
                    }
                    break;
                case '}':
                    if(!state) {
                        status = i2d_panic("invalid position token");
                    } else {
                        curly_level--;
                        state = NULL;
                    }
                    break;
                default:
                    if(state) {
                        status = i2d_token_putc(state, symbol);
                    } else {
                        status = i2d_token_init(&token, curly_level ? I2D_POSITION : I2D_LITERAL) ||
                                 i2d_token_putc(token, symbol);
                    }
                    break;
            }

            if(token) {
                i2d_token_append(token, root);
                state = token;
                token = NULL;
            }
        }

        if(root == root->next)
            status = i2d_panic("empty format specification");

        if(status)
            i2d_token_list_deit(&root);
        else
            *result = root;
    }

    return status;
}

int i2d_format_write(i2d_format * format, i2d_string_stack * stack, i2d_buffer * buffer) {
    int status = I2D_OK;
    i2d_string * list;
    size_t size;

    i2d_token * token;
    i2d_string string;
    long position;

    if(i2d_string_stack_get(stack, &list, &size)) {
        status = i2d_panic("failed to get string stack");
    } else {
        token = format->tokens->next;
        while(token->type != I2D_TOKEN && !status) {
            switch(token->type) {
                case I2D_LITERAL:
                    if(i2d_token_get_string(token, &string)) {
                        status = i2d_panic("failed to get token string");
                    } else if(i2d_buffer_printf(buffer, "%s", string.string)) {
                        status = i2d_panic("failed to write buffer");
                    }
                    break;
                case I2D_POSITION:
                    if(i2d_token_get_constant(token, &position)) {
                        status = i2d_panic("failed to get token constant");
                    } else if(position < 0 || (size_t) position >= size) {
                        status = i2d_panic("invalid position on string stack");
                    } else if(i2d_buffer_printf(buffer, "%s", list[position].string)) {
                        status = i2d_panic("failed to write buffer");
                    }
                    break;
                default:
                    status = i2d_panic("invalid token type -- %d", token->type);
            }
            token = token->next;
        }
    }

    return status;
}

int i2d_data_create(i2d_data * result, const char * key, json_t * json, i2d_constant_db * constant_db) {
    int status = I2D_OK;
    json_t * min;
    json_t * max;
    json_t * description;
    json_t * handler;
    json_t * argument_type;
    json_t * argument_default;
    json_t * argument_order;
    json_t * required;
    json_t * optional;
    json_t * positive;
    json_t * negative;
    json_t * zero;
    json_t * empty_description_on_zero;
    json_t * empty_description_on_empty_string;
    json_t * dump_stack_instead_of_description;

    min = json_object_get(json, "min");
    max = json_object_get(json, "max");
    description = json_object_get(json, "description");
    handler = json_object_get(json, "handler");
    argument_type = json_object_get(json, "argument_type");
    argument_default = json_object_get(json, "argument_default");
    argument_order = json_object_get(json, "argument_order");
    required = json_object_get(json, "required");
    optional = json_object_get(json, "optional");
    positive = json_object_get(json, "positive");
    negative = json_object_get(json, "negative");
    zero = json_object_get(json, "zero");
    empty_description_on_zero = json_object_get(json, "empty_description_on_zero");
    empty_description_on_empty_string = json_object_get(json, "empty_description_on_empty_string");
    dump_stack_instead_of_description = json_object_get(json, "dump_stack_instead_of_description");
    i2d_constant_get_by_macro_value(constant_db, key, &result->constant);

    if(i2d_string_create(&result->name, key, strlen(key))) {
        status = i2d_panic("failed to copy name string");
    } else if(min && max && i2d_object_get_range(min, max, &result->range)) {
        status = i2d_panic("failed to create range");
    } else if(description && i2d_format_create_json(&result->description, description)) {
        status = i2d_panic("failed to create format object");
    } else if(handler && i2d_object_get_string(handler, &result->handler)) {
        status = i2d_panic("failed to create string");
    } else if(argument_type && i2d_object_get_string_stack(argument_type, &result->argument_type)) {
        status = i2d_panic("failed to create string stack");
    } else if(argument_default && i2d_object_get_string_stack(argument_default, &result->argument_default)) {
        status = i2d_panic("failed to create string stack");
    } else if(argument_order && i2d_object_get_number_array(argument_order, &result->argument_order.list, &result->argument_order.size)) {
        status = i2d_panic("failed to create number array");
    } else if(required && i2d_object_get_number(required, &result->required)) {
        status = i2d_panic("failed to create number");
    } else if(optional && i2d_object_get_number(optional, &result->optional)) {
        status = i2d_panic("failed to create number");
    } else if(positive && i2d_object_get_string(positive, &result->positive)) {
        status = i2d_panic("failed to create string");
    } else if(negative && i2d_object_get_string(negative, &result->negative)) {
        status = i2d_panic("failed to create string");
    } else if(zero && i2d_object_get_string(zero, &result->zero)) {
        status = i2d_panic("failed to create string");
    } else if(empty_description_on_zero && i2d_object_get_boolean(empty_description_on_zero, &result->empty_description_on_zero)) {
        status = i2d_panic("failed to create boolean");
    } else if(empty_description_on_empty_string && i2d_object_get_boolean(empty_description_on_empty_string, &result->empty_description_on_empty_string)) {
        status = i2d_panic("failed to create boolean");
    } else if(dump_stack_instead_of_description && i2d_object_get_boolean(dump_stack_instead_of_description, &result->dump_stack_instead_of_description)) {
        status = i2d_panic("failed to create boolean");
    }

    return status;
}

void i2d_data_destroy(i2d_data * result) {
    i2d_string_destroy(&result->zero);
    i2d_string_destroy(&result->negative);
    i2d_string_destroy(&result->positive);
    i2d_free(result->argument_order.list);
    i2d_string_stack_destroy(&result->argument_default);
    i2d_string_stack_destroy(&result->argument_type);
    i2d_string_destroy(&result->handler);
    i2d_format_destroy(&result->description);
    i2d_range_destroy(&result->range);
    i2d_string_destroy(&result->name);
}

int i2d_data_map_init(i2d_data_map ** result, enum i2d_data_map_type type, json_t * json, i2d_constant_db * constant_db) {
    int status = I2D_OK;
    i2d_data_map * object;
    i2d_rbt_cmp cmp = NULL;

    size_t i = 0;
    const char * key;
    json_t * value;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            switch(type) {
                case data_map_by_constant:
                    cmp = i2d_rbt_cmp_long;
                    break;
                case data_map_by_name:
                    cmp = i2d_rbt_cmp_str;
                    break;
                default:
                    status = i2d_panic("invalid data map type");
                    break;
            }

            if(!status) {
                if(i2d_rbt_init(&object->map, cmp)) {
                    status = i2d_panic("failed to create red black tree object");
                } else if(i2d_object_get_list(json, sizeof(*object->list), (void **) &object->list, &object->size)) {
                    status = i2d_panic("failed to create data array");
                } else {
                    json_object_foreach(json, key, value) {
                        if(i2d_data_create(&object->list[i], key, value, constant_db)) {
                            status = i2d_panic("failed to create data object");
                        } else {
                            switch(type) {
                                case data_map_by_constant:
                                    if(i2d_rbt_insert(object->map, &object->list[i].constant, &object->list[i]))
                                        status = i2d_panic("failed to map data object");
                                    break;
                                case data_map_by_name:
                                    if(i2d_rbt_insert(object->map, object->list[i].name.string, &object->list[i]))
                                        status = i2d_panic("failed to map data object");
                                    break;
                                default:
                                    status = i2d_panic("invalid data map type");
                                    break;
                            }
                            i++;
                        }
                    }
                }
            }

            if(status)
                i2d_data_map_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_data_map_deit(i2d_data_map ** result) {
    i2d_data_map * object;
    size_t i;

    object = *result;
    if(object->list) {
        for(i = 0; i < object->size; i++)
            i2d_data_destroy(&object->list[i]);
        i2d_free(object->list);
    }
    i2d_deit(object->map, i2d_rbt_deit);
    i2d_free(object);
    *result = NULL;
}

int i2d_data_map_get(i2d_data_map * data_map, void * key, i2d_data ** result) {
    return i2d_rbt_search(data_map->map, key, (void **) result);
}

int i2d_lexer_init(i2d_lexer ** result) {
    int status = I2D_OK;
    i2d_lexer * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_token_init(&object->cache, I2D_TOKEN))
                status = i2d_panic("failed to create token object");

            if(status)
                i2d_lexer_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_lexer_deit(i2d_lexer ** result) {
    i2d_lexer * object;

    object = *result;
    i2d_deit(object->cache, i2d_token_list_deit);
    i2d_free(object);
    *result = NULL;
}

void i2d_lexer_reset(i2d_lexer * lexer, i2d_token ** result) {
    i2d_token * token;

    token = *result;
    do {
        i2d_buffer_clear(&token->buffer);
        token->type = I2D_TOKEN;
        token = token->next;
    } while(token != *result);
    i2d_token_append(token, lexer->cache);
    *result = NULL;
}

int i2d_lexer_token_init(i2d_lexer * lexer, i2d_token ** result, enum i2d_token_type type) {
    int status = I2D_OK;
    i2d_token * token;

    if(lexer->cache != lexer->cache->next) {
        token = lexer->cache->next;
        i2d_token_remove(token);
        token->type = type;
        *result = token;
    } else {
        status = i2d_token_init(result, type);
    }

    return status;
}

int i2d_lexer_tokenize(i2d_lexer * lexer, i2d_string * script, i2d_token ** result) {
    int status = I2D_OK;
    size_t i;
    char symbol;
    i2d_token * root = NULL;
    i2d_token * token = NULL;
    i2d_token * state = NULL;

    if(i2d_lexer_token_init(lexer, &root, I2D_TOKEN)) {
        status = i2d_panic("failed to create token object");
    } else {
        for(i = 0; i < script->length && !status; i++) {
            symbol = script->string[i];
            token = NULL;

            if(state) {
                if(I2D_QUOTE == state->type) {
                    if('"' == symbol) {
                        state->type = I2D_LITERAL;
                    } else if('\\' == symbol) {
                        i++;
                        if(i >= script->length) {
                            status = i2d_panic("invalid escape character");
                        } else {
                            status = i2d_token_putc(state, script->string[i]);
                        }
                    } else {
                        status = i2d_token_putc(state, symbol);
                    }
                    continue;
                } else if(I2D_LINE_COMMENT == state->type) {
                    if('\n' == symbol) {
                        i2d_token_remove(state);
                        i2d_token_deit(&state);
                    }
                    continue;
                } else if(I2D_BLOCK_COMMENT == state->type) {
                    if('/' != symbol) {
                        status = i2d_token_putc(state, symbol);
                    } else if('*' == i2d_token_getc(state)) {
                        i2d_token_remove(state);
                        i2d_token_deit(&state);
                    }
                    continue;
                }
            }

            switch(symbol) {
                case '{': status = i2d_lexer_token_init(lexer, &token, I2D_CURLY_OPEN); break;
                case '}': status = i2d_lexer_token_init(lexer, &token, I2D_CURLY_CLOSE); break;
                case '(': status = i2d_lexer_token_init(lexer, &token, I2D_PARENTHESIS_OPEN); break;
                case ')': status = i2d_lexer_token_init(lexer, &token, I2D_PARENTHESIS_CLOSE); break;
                case '[': status = i2d_lexer_token_init(lexer, &token, I2D_BRACKET_OPEN); break;
                case ']': status = i2d_lexer_token_init(lexer, &token, I2D_BRACKET_CLOSE); break;
                case ',': status = i2d_lexer_token_init(lexer, &token, I2D_COMMA); break;
                case ';': status = i2d_lexer_token_init(lexer, &token, I2D_SEMICOLON); break;
                case '$':
                    if(state && I2D_LITERAL == state->type && '$' != i2d_token_getc(state)) {
                        status = i2d_token_putc(state, '$');
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_PERMANENT_GLOBAL);
                    }
                    break;
                case '.': status = i2d_lexer_token_init(lexer, &token, I2D_TEMPORARY_NPC); break;
                case '\'': status = i2d_lexer_token_init(lexer, &token, I2D_TEMPORARY_INSTANCE); break;
                case '@':
                    if(state && I2D_PERMANENT_GLOBAL == state->type) {
                        state->type = I2D_TEMPORARY_GLOBAL;
                    } else if(state && I2D_TEMPORARY_NPC == state->type) {
                        state->type = I2D_TEMPORARY_SCOPE;
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_TEMPORARY_CHARACTER); break;
                    }
                    break;
                case '#':
                    if(state && I2D_PERMANENT_ACCOUNT_LOCAL == state->type) {
                        state->type = I2D_PERMANENT_ACCOUNT_GLOBAL;
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_PERMANENT_ACCOUNT_LOCAL);
                    }
                    break;
                case '+':
                    if(state && I2D_ADD == state->type) {
                        state->type = I2D_INCREMENT;
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_ADD);
                    }
                    break;
                case '-':
                    if(state && I2D_SUBTRACT == state->type) {
                        state->type = I2D_DECREMENT;
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_SUBTRACT);
                    }
                    break;
                case '*':
                    if(state && I2D_DIVIDE == state->type) {
                        state->type = I2D_BLOCK_COMMENT;
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_MULTIPLY);
                    }
                    break;
                case '/':
                    if(state && I2D_DIVIDE == state->type) {
                        state->type = I2D_LINE_COMMENT;
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_DIVIDE);
                    }
                    break;
                case '%': status = i2d_lexer_token_init(lexer, &token, I2D_MODULUS); break;
                case '>':
                    if(state && I2D_GREATER == state->type) {
                        state->type = I2D_RIGHT_SHIFT;
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_GREATER);
                    }
                    break;
                case '<':
                    if(state && I2D_LESS == state->type) {
                        state->type = I2D_LEFT_SHIFT;
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_LESS);
                    }
                    break;
                case '!': status = i2d_lexer_token_init(lexer, &token, I2D_NOT); break;
                case '&':
                    if(state && I2D_BIT_AND == state->type) {
                        state->type = I2D_AND;
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_BIT_AND);
                    }
                    break;
                case '|':
                    if(state && I2D_BIT_OR == state->type) {
                        state->type = I2D_OR;
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_BIT_OR);
                    }
                    break;
                case '^': status = i2d_lexer_token_init(lexer, &token, I2D_BIT_XOR); break;
                case '~': status = i2d_lexer_token_init(lexer, &token, I2D_BIT_NOT); break;
                case '=':
                    if(state) {
                        switch(state->type) {
                            case I2D_ADD: state->type = I2D_ADD_ASSIGN; break;
                            case I2D_SUBTRACT: state->type = I2D_SUBTRACT_ASSIGN; break;
                            case I2D_MULTIPLY: state->type = I2D_MULTIPLY_ASSIGN; break;
                            case I2D_DIVIDE: state->type = I2D_DIVIDE_ASSIGN; break;
                            case I2D_MODULUS: state->type = I2D_MODULUS_ASSIGN; break;
                            case I2D_ASSIGN: state->type = I2D_EQUAL; break;
                            case I2D_GREATER: state->type = I2D_GREATER_EQUAL; break;
                            case I2D_LESS: state->type = I2D_LESS_EQUAL; break;
                            case I2D_NOT: state->type = I2D_NOT_EQUAL; break;
                            case I2D_RIGHT_SHIFT: state->type = I2D_RIGHT_SHIFT_ASSIGN; break;
                            case I2D_LEFT_SHIFT: state->type = I2D_LEFT_SHIFT_ASSIGN; break;
                            case I2D_BIT_AND: state->type = I2D_BIT_AND_ASSIGN; break;
                            case I2D_BIT_OR: state->type = I2D_BIT_OR_ASSIGN; break;
                            case I2D_BIT_XOR: state->type = I2D_BIT_XOR_ASSIGN; break;
                            default: status = i2d_lexer_token_init(lexer, &token, I2D_ASSIGN); break;
                        }
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_ASSIGN);
                    }
                    break;
                case '?': status = i2d_lexer_token_init(lexer, &token, I2D_CONDITIONAL); break;
                case ':':
                    if(state && I2D_COLON == state->type) {
                        state->type = I2D_UNIQUE_NAME;
                    } else {
                        status = i2d_lexer_token_init(lexer, &token, I2D_COLON);
                    }
                    break;
                case '"': status = i2d_lexer_token_init(lexer, &token, I2D_QUOTE); break;
                case '\\': continue;
                default:
                    if('_' == symbol || isalpha(symbol) || isdigit(symbol)) {
                        if(state) {
                            switch(state->type) {
                                case I2D_LITERAL:
                                    status = i2d_token_putc(state, symbol);
                                    break;
                                case I2D_TEMPORARY_CHARACTER:
                                    status = i2d_token_putc(state, '@') ||
                                             i2d_token_putc(state, symbol);
                                    state->type = I2D_LITERAL;
                                    break;
                                case I2D_PERMANENT_GLOBAL:
                                    status = i2d_token_putc(state, '$') ||
                                             i2d_token_putc(state, symbol);
                                    state->type = I2D_LITERAL;
                                    break;
                                case I2D_TEMPORARY_GLOBAL:
                                    status = i2d_token_putc(state, '$') ||
                                             i2d_token_putc(state, '@') ||
                                             i2d_token_putc(state, symbol);
                                    state->type = I2D_LITERAL;
                                    break;
                                case I2D_TEMPORARY_NPC:
                                    status = i2d_token_putc(state, '.') ||
                                             i2d_token_putc(state, symbol);
                                    state->type = I2D_LITERAL;
                                    break;
                                case I2D_TEMPORARY_SCOPE:
                                    status = i2d_token_putc(state, '.') ||
                                             i2d_token_putc(state, '@') ||
                                             i2d_token_putc(state, symbol);
                                    state->type = I2D_LITERAL;
                                    break;
                                case I2D_TEMPORARY_INSTANCE:
                                    status = i2d_token_putc(state, '\'') ||
                                             i2d_token_putc(state, symbol);
                                    state->type = I2D_LITERAL;
                                    break;
                                case I2D_PERMANENT_ACCOUNT_LOCAL:
                                    status = i2d_token_putc(state, '#') ||
                                             i2d_token_putc(state, symbol);
                                    state->type = I2D_LITERAL;
                                    break;
                                case I2D_PERMANENT_ACCOUNT_GLOBAL:
                                    status = i2d_token_putc(state, '#') ||
                                             i2d_token_putc(state, '#') ||
                                             i2d_token_putc(state, symbol);
                                    state->type = I2D_LITERAL;
                                    break;
                                default:
                                    status = i2d_lexer_token_init(lexer, &token, I2D_LITERAL) ||
                                             i2d_token_putc(token, symbol);
                                    break;
                            }
                        } else {
                            status = i2d_lexer_token_init(lexer, &token, I2D_LITERAL) ||
                                     i2d_token_putc(token, symbol);
                        }
                    } else if(isspace(symbol)) {
                        state = NULL;
                    } else {
                        status = i2d_panic("unknown symbol %c", symbol);
                    }
                    break;
            }

            if(token) {
                i2d_token_append(token, root);
                state = token;
            }
        }
    }

    if(status)
        i2d_lexer_reset(lexer, &root);
    else
        *result = root;

    return status;
}

const char * i2d_node_string[] = {
    "node",
    "number",
    "variable",
    "function",
    "index",
    "unary",
    "binary"
};

int i2d_rbt_cmp_node(const void * left, const void * right) {
    int status = I2D_OK;
    i2d_node * l_node = (i2d_node *) left;
    i2d_node * r_node = (i2d_node *) right;
    i2d_string l_name;
    i2d_string r_name;
    long l_min;
    long l_max;
    long r_min;
    long r_max;

    if(!i2d_node_get_string(l_node, &l_name) && !i2d_node_get_string(r_node, &r_name))
#ifndef _WIN32
        status = strcasecmp(l_name.string, r_name.string);
#else
        status = _stricmp(l_name.string, r_name.string);
#endif

    if(!status && l_node->type == I2D_INDEX && r_node->type == I2D_INDEX) {
        i2d_range_get_range(&l_node->index->range, &l_min, &l_max);
        i2d_range_get_range(&r_node->index->range, &r_min, &r_max);
        if(l_min == r_min && l_max == r_max) {
            status = 0;
        } else if(l_max < r_max) {
            status = -1;
        } else {
            status = 1;
        }
    }

    return status;
}

int i2d_node_init(i2d_node ** result, enum i2d_node_type type, i2d_token * tokens) {
    int status = I2D_OK;
    i2d_node * object = NULL;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            object->type = type;
            object->tokens = tokens;

            if(status)
                i2d_node_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_node_deit(i2d_node ** result) {
    i2d_node * object;

    object = *result;
    i2d_deit(object->right, i2d_node_deit);
    i2d_deit(object->left, i2d_node_deit);
    i2d_deit(object->index, i2d_node_deit);
    i2d_deit(object->tokens, i2d_token_list_deit);
    i2d_deit(object->logic, i2d_logic_deit);
    i2d_range_destroy(&object->range);
    i2d_free(object);
    *result = NULL;
}

void i2d_node_list_deit(i2d_node ** result) {
    i2d_node * object;
    i2d_node * node;

    object = *result;
    if(object) {
        while(object != object->left) {
            node = object->left;
            i2d_node_remove(node);
            i2d_free(node);
        }
        i2d_free(object);
    }
    *result = NULL;
}

void i2d_node_append(i2d_node * x, i2d_node * y) {
    x->left->right = y->right;
    y->right->left = x->left;
    x->left = y;
    y->right = x;
}

void i2d_node_remove(i2d_node * x) {
    x->right->left = x->left;
    x->left->right = x->right;
    x->left = x;
    x->right = x;
}

void i2d_node_print(i2d_node * node, int level) {
    int i;

    for(i = 0; i < level; i++)
        fprintf(stdout, "    ");

    fprintf(stdout, "(%s) ", i2d_node_string[node->type]);

    i2d_token_print(node->tokens);
    i2d_range_print(&node->range, NULL);

    if(node->logic)
        i2d_logic_print(node->logic, level + 1);

    if(node->left)
        i2d_node_print(node->left, level + 1);
    if(node->right)
        i2d_node_print(node->right, level + 1);
}

int i2d_node_copy(i2d_node * source, i2d_node * target) {
    int status = I2D_OK;

    i2d_range_destroy(&source->range);
    i2d_deit(source->logic, i2d_logic_deit);

    if(i2d_range_copy(&source->range, &target->range)) {
        status = i2d_panic("failed to copy range list object");
    } else if(target->logic) {
        if(i2d_logic_copy(&source->logic, target->logic))
            status = i2d_panic("failed to copy logic object");
    }

    return status;
}

int i2d_node_get_arguments(i2d_node * node, i2d_node ** nodes, size_t required, size_t optional) {
    int status = I2D_OK;
    size_t i;
    size_t size;
    i2d_node * iter;

    if(!node) {
        if(required)
            status = i2d_panic("failed on empty argument list");
    } else {
        if(node->type == I2D_NODE)
            node = node->left;

        if(required || optional) {
            if(!node) {
                if(required)
                    status = i2d_panic("failed on empty argument list");
            } else {
                size = 1;
                iter = node;
                while(I2D_COMMA == iter->tokens->type) {
                    iter = iter->left;
                    size++;
                }

                if(size < required) {
                    status = i2d_panic("failed on insufficient argument list");
                } else if(required + optional < size) {
                    status = i2d_panic("failed on excessive argument list");
                } else {
                    i = size - 1;
                    while(i > 0 && I2D_COMMA == node->tokens->type) {
                        nodes[i] = node->right;
                        node = node->left;
                        i--;
                    }

                    if(i || !node) {
                        status = i2d_panic("failed on insufficient argument list");
                    } else if(I2D_COMMA == node->tokens->type) {
                        status = i2d_panic("failed on excessive argument list");
                    } else {
                        nodes[0] = node;
                    }
                }
            }
        } else if(node) {
            status = i2d_panic("failed on excessive argument list");
        }
    }

    return status;
}

int i2d_node_get_constant(i2d_node * node, long * result) {
    int status = I2D_OK;

    long min;
    long max;

    if(node->constant) {
        *result = node->constant->value;
    } else {
        i2d_range_get_range(&node->range, &min, &max);
        if(min != max) {
            status = i2d_panic("failed on invalid range");
        } else {
            *result = min;
        }
    }

    return status;
}

int i2d_node_set_constant(i2d_node * node, i2d_constant * constant) {
    int status = I2D_OK;

    node->constant = constant;
    if(i2d_range_copy(&node->range, &constant->range)) {
        status = i2d_panic("failed to copy range");
    } else if(constant->name.length && i2d_node_set_string(node, &constant->name)) {
        status = i2d_panic("failed to copy name");
    }

    return status;
}

int i2d_node_get_string(i2d_node * node, i2d_string * result) {
    return i2d_token_get_string(node->tokens, result);
}

int i2d_node_set_string(i2d_node * node, i2d_string * result) {
    return i2d_token_set_string(node->tokens, result);
}

int i2d_node_get_predicate(i2d_node * node, i2d_string * result) {
    int status = I2D_FAIL;

    if(I2D_VARIABLE == node->type) {
        status = i2d_node_get_string(node, result);
    } else if(I2D_FUNCTION == node->type) {
        status = i2d_node_get_string(node, result);
    } else {
        if(node->left)
            status = i2d_node_get_predicate(node->left, result);
        if(node->right && status)
            status = i2d_node_get_predicate(node->right, result);
    }

    return status;
}

int i2d_node_get_predicate_all(i2d_node * node, i2d_string_stack * stack) {
    int status = I2D_OK;

    if(node->left && i2d_node_get_predicate_all_recursive(node->left, stack)) {
        status = i2d_panic("failed to get left node's predicate list");
    } else if(node->right && i2d_node_get_predicate_all_recursive(node->right, stack)) {
        status = i2d_panic("failed to get right node's predicate list");
    }

    return status;
}

int i2d_node_get_predicate_all_recursive(i2d_node * node, i2d_string_stack * stack) {
    int status = I2D_OK;
    i2d_string string;

    if( I2D_VARIABLE == node->type ||
        I2D_FUNCTION == node->type ) {
        if(i2d_node_get_string(node, &string)) {
            status = i2d_panic("failed to get node string");
        } else if(i2d_string_stack_push(stack, string.string, string.length)) {
            status = i2d_panic("failed to push node string");
        }
    } else {
        if(node->left)
            status = i2d_node_get_predicate_all_recursive(node->left, stack);
        if(!status && node->right)
            status = i2d_node_get_predicate_all_recursive(node->right, stack);
    }

    return status;
}

int i2d_node_is_conditional(i2d_node * node) {
    return (node->type == I2D_BINARY && node->tokens->type == I2D_CONDITIONAL) ? I2D_OK : I2D_FAIL;
}

int i2d_node_is_colon(i2d_node * node) {
    return (node->type == I2D_BINARY && node->tokens->type == I2D_COLON) ? I2D_OK : I2D_FAIL;
}

const char * i2d_block_string[] = {
    "block",
    "statement",
    "if",
    "else",
    "for"
};

int i2d_block_init(i2d_block ** result, enum i2d_block_type type, i2d_token * tokens, i2d_block * parent) {
    int status = I2D_OK;
    i2d_block * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_buffer_create(&object->buffer, BUFFER_SIZE_LARGE)) {
                status = i2d_panic("failed to create buffer object");
            } else {
                object->type = type;
                object->tokens = tokens;
                object->parent = parent;
                object->next = object;
                object->prev = object;
            }
            if(status)
                i2d_block_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_block_deit(i2d_block ** result) {
    i2d_block * object;

    object = *result;
    i2d_deit(object->child, i2d_block_list_deit);
    i2d_deit(object->logics, i2d_logic_deit);
    i2d_deit(object->nodes, i2d_node_deit);
    i2d_deit(object->tokens, i2d_token_list_deit);
    i2d_buffer_destroy(&object->buffer);
    i2d_free(object);
    *result = NULL;
}

void i2d_block_list_deit(i2d_block ** result) {
    i2d_block * object;
    i2d_block * block;

    object = *result;
    if(object) {
        while(object != object->next) {
            block = object->next;
            i2d_block_remove(block);
            i2d_block_deit(&block);
        }
    }
    i2d_deit(object, i2d_block_deit);
    *result = NULL;
}

void i2d_block_append(i2d_block * x, i2d_block * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_block_remove(i2d_block * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

void i2d_block_print(i2d_block * block, int level) {
    int i;
    i2d_string string;
    i2d_zero(string);

    for(i = 0; i < level; i++)
        fprintf(stdout, "    ");

    i2d_buffer_get(&block->buffer, &string.string, &string.length);

    if(block->statement)
        fprintf(stdout, "%s [%p] %s\n", block->statement->name.string, block, string.length ? string.string : "");
    else
        fprintf(stdout, "%s [%p] %s\n", i2d_block_string[block->type], block, string.length ? string.string : "");

    if(block->nodes)
        i2d_node_print(block->nodes, level + 1);
    if(block->child)
        i2d_block_list_print(block->child, level + 1);
}

void i2d_block_list_print(i2d_block * block, int level) {
    i2d_block * iterator;

    iterator = block;
    do {
        i2d_block_print(iterator, level);
        iterator = iterator->next;
    } while(iterator != block);
}

int i2d_parser_init(i2d_parser ** result) {
    int status = I2D_OK;
    i2d_parser * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_block_init(&object->block_cache, I2D_BLOCK, NULL, NULL)) {
                status = i2d_panic("failed to create block object");
            } else if(i2d_node_init(&object->node_cache, I2D_NODE, NULL)) {
                status = i2d_panic("failed to create node object");
            } else {
                object->node_cache->left = object->node_cache;
                object->node_cache->right = object->node_cache;
            }

            if(status)
                i2d_parser_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_parser_deit(i2d_parser ** result) {
    i2d_parser * object;

    object = *result;
    i2d_deit(object->node_cache, i2d_node_list_deit);
    i2d_deit(object->block_cache, i2d_block_list_deit);
    i2d_free(object);
    *result = NULL;
}

void i2d_parser_reset(i2d_parser * parser, i2d_lexer * lexer, i2d_block ** result) {
    i2d_block * block;

    block = *result;
    do {
        if(block->child)
            i2d_parser_reset(parser, lexer, &block->child);
        block->parent = NULL;
        i2d_buffer_clear(&block->buffer);
        block->statement = NULL;
        if(block->logics)
            i2d_logic_deit(&block->logics);
        if(block->nodes)
            i2d_parser_node_reset(parser, lexer, &block->nodes);
        if(block->tokens)
            i2d_lexer_reset(lexer, &block->tokens);
        block->type = I2D_BLOCK;
        block = block->next;
    } while(block != *result);
    i2d_block_append(block, parser->block_cache);
    *result = NULL;
}

void i2d_parser_node_reset(i2d_parser * parser, i2d_lexer * lexer, i2d_node ** result) {
    i2d_node * node;

    node = *result;
    if(node->right)
        i2d_parser_node_reset(parser, lexer, &node->right);
    node->right = node;
    if(node->left)
        i2d_parser_node_reset(parser, lexer, &node->left);
    if(node->index)
        i2d_parser_node_reset(parser, lexer, &node->index);
    node->left = node;
    if(node->tokens)
        i2d_lexer_reset(lexer, &node->tokens);
    i2d_deit(node->logic, i2d_logic_deit);
    i2d_range_destroy(&node->range);
    node->constant = NULL;
    node->type = I2D_NODE;
    i2d_node_append(node, parser->node_cache);
    *result = NULL;
}

int i2d_parser_block_init(i2d_parser * parser, i2d_block ** result, enum i2d_block_type type, i2d_token * tokens, i2d_block * parent) {
    int status = I2D_OK;
    i2d_block * block;

    if(parser->block_cache != parser->block_cache->next) {
        block = parser->block_cache->next;
        i2d_block_remove(block);
        block->type = type;
        block->tokens = tokens;
        block->parent = parent;
        *result = block;
    } else {
        status = i2d_block_init(result, type, tokens, parent);
    }

    return status;
}

int i2d_parser_node_init(i2d_parser * parser, i2d_node ** result, enum i2d_node_type type, i2d_token * tokens) {
    int status = I2D_OK;
    i2d_node * node;

    if(parser->node_cache != parser->node_cache->left) {
        node = parser->node_cache->left;
        i2d_node_remove(node);
        node->type = type;
        node->tokens = tokens;
        node->left = NULL;
        node->right = NULL;
        *result = node;
    } else {
        status = i2d_node_init(result, type, tokens);
    }

    return status;
}


int i2d_parser_get_statement(i2d_parser * parser, i2d_lexer * lexer, i2d_data_map * statements, i2d_block * block) {
    int status = I2D_OK;
    i2d_token * token;
    i2d_string name;

    token = block->tokens->next;
    if(I2D_LITERAL == token->type) {
        if(i2d_token_get_string(token, &name)) {
            status = i2d_panic("failed to get token string");
        } else {
            if(!i2d_data_map_get(statements, name.string, (void *) &block->statement)) {
                i2d_token_remove(token);
                i2d_lexer_reset(lexer, &token);
            }
        }
    }

    return status;
}

int i2d_parser_analysis(i2d_parser * parser, i2d_lexer * lexer, i2d_data_map * statements, i2d_token * tokens, i2d_block ** result) {
    int status = I2D_OK;
    i2d_token * token;

    token = tokens->prev;
    if(token->type == I2D_LINE_COMMENT) {
        i2d_token_remove(token);
        i2d_lexer_reset(lexer, &token);
    }

    if(I2D_CURLY_OPEN != tokens->next->type) {
        status = i2d_panic("script must start with a {");
    } else if(I2D_CURLY_CLOSE != tokens->prev->type) {
        status = i2d_panic("script must end with a }");
    } else if(i2d_parser_analysis_recursive(parser, lexer, statements, NULL, result, tokens->next)) {
        status = i2d_panic("failed to parse script");
    }

    return status;
}

int i2d_parser_analysis_recursive(i2d_parser * parser, i2d_lexer * lexer, i2d_data_map * statements, i2d_block * parent, i2d_block ** result, i2d_token * tokens) {
    int status = I2D_OK;
    i2d_block * root;
    i2d_block * block;
    i2d_block * state = NULL;
    i2d_token * anchor;
    i2d_token * token = NULL;
    i2d_string string;
    int parenthesis;

    root = NULL;
    block = NULL;
    anchor = tokens;
    while(I2D_TOKEN != tokens->type && I2D_CURLY_CLOSE != tokens->type && !status) {
        if(I2D_CURLY_OPEN == tokens->type) {
            if(i2d_parser_block_init(parser, &block, I2D_BLOCK, NULL, parent)) {
                status = i2d_panic("failed to create block object");
            } else if(i2d_parser_analysis_recursive(parser, lexer, statements, block, &block->child, tokens->next)) {
                status = i2d_panic("failed to parse script");
            } else if(I2D_CURLY_CLOSE != tokens->next->type) {
                status = i2d_panic("missing } after {");
            } else {
                tokens = tokens->next->next;
                i2d_token_append(anchor->prev, tokens);
                i2d_lexer_reset(lexer, &anchor);
                anchor = tokens;
            }
        } else if(I2D_SEMICOLON == tokens->type) {
            if(anchor == tokens) {
                /*
                 * ignore empty statements
                 */
                token = tokens;
                tokens = tokens->next;
                anchor = tokens;

                i2d_token_remove(token);
                i2d_lexer_reset(lexer, &token);
            } else if(i2d_parser_block_init(parser, &block, I2D_STATEMENT, anchor, parent)) {
                status = i2d_panic("failed to create block object");
            } else {
                i2d_token_append(anchor->prev, tokens);
                tokens = tokens->next;
                anchor = tokens;

                token = tokens->prev;
                i2d_token_remove(token);
                i2d_lexer_reset(lexer, &token);

                if(i2d_lexer_token_init(lexer, &token, I2D_TOKEN)) {
                    status = i2d_panic("failed to create token object");
                } else {
                    i2d_token_append(token, block->tokens);
                    block->tokens = token;
                    token = NULL;
                    if(i2d_parser_get_statement(parser, lexer, statements, block)) {
                        status = i2d_panic("failed to lookup block data object");
                    } else if(block->statement && block->tokens->next->type == I2D_SEMICOLON) {
                        /* support statements without arguments */
                    } else if(i2d_parser_expression_recursive(parser, lexer, block->tokens->next, &block->nodes)) {
                        status = i2d_panic("failed to parse expression");
                    }
                }
            }
        } else if(I2D_LITERAL == tokens->type) {
            if(i2d_token_get_string(tokens, &string)) {
                status = i2d_panic("failed to get token string");
            } else if(!strcmp("if", string.string)) {
                if(i2d_parser_block_init(parser, &block, I2D_IF, anchor, parent)) {
                    status = i2d_panic("failed to create block object");
                } else {
                    tokens = tokens->next;
                    i2d_token_remove(anchor);
                    anchor = tokens;

                    if(I2D_PARENTHESIS_OPEN != tokens->type) {
                        status = i2d_panic("missing ( after if");
                    } else {
                        parenthesis = 1;
                        while(I2D_TOKEN != tokens->type && parenthesis) {
                            tokens = tokens->next;
                            switch(tokens->type) {
                                case I2D_PARENTHESIS_OPEN:  parenthesis++; break;
                                case I2D_PARENTHESIS_CLOSE: parenthesis--; break;
                                default: break;
                            }
                        }
                        if(I2D_PARENTHESIS_CLOSE != tokens->type) {
                            status = i2d_panic("missing ) after (");
                        } else if(anchor->next == tokens) {
                            status = i2d_panic("empty if expression");
                        } else {
                            if(i2d_lexer_token_init(lexer, &token, I2D_TOKEN)) {
                                status = i2d_panic("failed to create token object");
                            } else {
                                tokens = tokens->next;
                                i2d_token_append(anchor->prev, tokens);
                                i2d_token_append(token, anchor);
                                anchor = tokens;

                                if(i2d_parser_expression_recursive(parser, lexer, token->next, &block->nodes))
                                    status = i2d_panic("failed to parse expression");

                                i2d_lexer_reset(lexer, &token);
                            }
                        }
                    }
                }
            } else if(!strcmp("else", string.string)) {
                if(i2d_parser_block_init(parser, &block, I2D_ELSE, anchor, parent)) {
                    status = i2d_panic("failed to create block object");
                } else {
                    tokens = tokens->next;
                    i2d_token_remove(anchor);
                    anchor = tokens;
                }
            } else if(!strcmp("for", string.string)) {
                if(i2d_parser_block_init(parser, &block, I2D_FOR, anchor, parent)) {
                    status = i2d_panic("failed to create block object");
                } else {
                    tokens = tokens->next;
                    i2d_token_remove(anchor);
                    anchor = tokens;

                    if(I2D_PARENTHESIS_OPEN != tokens->type) {
                        status = i2d_panic("missing ( after if");
                    } else {
                        parenthesis = 1;
                        while(I2D_TOKEN != tokens->type && parenthesis) {
                            tokens = tokens->next;
                            switch(tokens->type) {
                                case I2D_PARENTHESIS_OPEN:  parenthesis++; break;
                                case I2D_PARENTHESIS_CLOSE: parenthesis--; break;
                                default: break;
                            }
                        }
                        if(I2D_PARENTHESIS_CLOSE != tokens->type) {
                            status = i2d_panic("missing ) after (");
                        } else {
                            token = anchor;
                            tokens = tokens->next;
                            i2d_token_append(anchor->prev, tokens);
                            anchor = tokens;
                            i2d_lexer_reset(lexer, &token);
                        }
                    }
                }
            } else {
                tokens = tokens->next;
            }
        } else {
            tokens = tokens->next;
        }

        if(block) {
            if(!root) {
                root = block;
            } else {
                if((I2D_IF == state->type || I2D_ELSE == state->type || I2D_FOR == state->type) && !state->child) {
                    state->child = block;
                    block->parent = state;
                } else if(I2D_IF == state->parent->type && I2D_ELSE == block->type) {
                    i2d_block_append(block, state);
                } else {
                    i2d_block_append(block, root);
                }
            }
            state = block;
            block = NULL;
        }
    }

    if(status) {
        i2d_deit(root, i2d_block_list_deit);
    } else {
        *result = root;
    }

    return status;
}

int i2d_parser_expression_recursive(i2d_parser * parser, i2d_lexer * lexer, i2d_token * tokens, i2d_node ** result) {
    int status = I2D_OK;
    i2d_node * root = NULL;
    i2d_node * iter = NULL;
    i2d_node * node = NULL;

    int parenthesis;
    int bracket;
    i2d_token * anchor;
    i2d_token * token = NULL;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        while(tokens->type != I2D_TOKEN && !status) {
            switch(tokens->type) {
                case I2D_PARENTHESIS_OPEN:
                    parenthesis = 1;
                    anchor = tokens;
                    while(I2D_TOKEN != tokens->type && parenthesis) {
                        tokens = tokens->next;
                        switch(tokens->type) {
                            case I2D_PARENTHESIS_OPEN:  parenthesis++; break;
                            case I2D_PARENTHESIS_CLOSE: parenthesis--; break;
                            default: break;
                        }
                    }
                    if(I2D_PARENTHESIS_CLOSE != tokens->type) {
                        status = i2d_panic("missing ) after (");
                    } else if(anchor->next == tokens) {
                        if(i2d_lexer_token_init(lexer, &token, I2D_TOKEN)) {
                            status = i2d_panic("failed to create token object");
                        } else {
                            if(i2d_parser_node_init(parser, &node, I2D_NODE, token)) {
                                status = i2d_panic("failed to create node object");
                            } else {
                                tokens = tokens->next;
                            }

                            if(status) {
                                i2d_lexer_reset(lexer, &token);
                            } else {
                                token = NULL;
                            }
                        }
                    } else {
                        anchor = anchor->next;
                        i2d_token_append(anchor->prev, tokens);
                        tokens = tokens->next;

                        if(i2d_lexer_token_init(lexer, &token, I2D_TOKEN)) {
                            status = i2d_panic("failed to create token object");
                        } else {
                            i2d_token_append(token, anchor);
                            if(i2d_parser_expression_recursive(parser, lexer, anchor, &node))
                                status = i2d_panic("failed to parse expression");

                            i2d_lexer_reset(lexer, &token);
                        }
                    }

                    if(!status && root) {
                        iter = root;
                        while(iter->right)
                            iter = iter->right;

                        if(I2D_VARIABLE == iter->type) {
                            iter->left = node;
                            iter->type = I2D_FUNCTION;
                            node = NULL;
                        }
                    }
                    break;
                case I2D_BRACKET_OPEN:
                    bracket = 1;
                    anchor = tokens;
                    while(I2D_TOKEN != tokens->type && bracket) {
                        tokens = tokens->next;
                        switch(tokens->type) {
                            case I2D_BRACKET_OPEN:  bracket++; break;
                            case I2D_BRACKET_CLOSE: bracket--; break;
                            default: break;
                        }
                    }
                    if(I2D_BRACKET_CLOSE != tokens->type) {
                        status = i2d_panic("missing ] after [");
                    } else if(anchor->next == tokens) {
                        status = i2d_panic("empty index expression");
                    } else {
                        anchor = anchor->next;
                        i2d_token_append(anchor->prev, tokens);
                        tokens = tokens->next;

                        if(i2d_lexer_token_init(lexer, &token, I2D_TOKEN)) {
                            status = i2d_panic("failed to create token object");
                        } else {
                            i2d_token_append(token, anchor);
                            if(i2d_parser_expression_recursive(parser, lexer, anchor, &node))
                                status = i2d_panic("failed to parse expression");

                            i2d_lexer_reset(lexer, &token);
                        }
                    }

                    if(!status && root) {
                        iter = root;
                        while(iter->right)
                            iter = iter->right;

                        if(I2D_VARIABLE == iter->type) {
                            iter->index = node;
                            iter->type = I2D_INDEX;
                            node = NULL;
                        }
                    }
                    break;
                case I2D_LITERAL:
                    if(i2d_parser_node_init(parser, &node, I2D_VARIABLE, tokens)) {
                        status = i2d_panic("failed to create node object");
                    } else {
                        tokens = tokens->next;
                        i2d_token_remove(tokens->prev);
                    }
                    break;
                case I2D_NOT:
                case I2D_BIT_NOT:
                case I2D_INCREMENT:
                case I2D_DECREMENT:
                    if(i2d_parser_node_init(parser, &node, I2D_UNARY, tokens)) {
                        status = i2d_panic("failed to create node object");
                    } else {
                        tokens = tokens->next;
                        i2d_token_remove(tokens->prev);
                    }
                    break;
                case I2D_ADD:
                case I2D_SUBTRACT:
                    if(root) {
                        iter = root;
                        while(iter->right)
                            iter = iter->right;

                        if(I2D_UNARY == iter->type || I2D_BINARY == iter->type) {
                            if(i2d_parser_node_init(parser, &node, I2D_UNARY, tokens)) {
                                status = i2d_panic("failed to create node object");
                            } else {
                                tokens = tokens->next;
                                i2d_token_remove(tokens->prev);

                                if(I2D_ADD == node->tokens->type) {
                                    node->tokens->type = I2D_ADD_UNARY;
                                } else {
                                    node->tokens->type = I2D_SUBTRACT_UNARY;
                                }
                            }
                        } else {
                            if(i2d_parser_node_init(parser, &node, I2D_BINARY, tokens)) {
                                status = i2d_panic("failed to create node object");
                            } else {
                                tokens = tokens->next;
                                i2d_token_remove(tokens->prev);
                            }
                        }
                    } else {
                        if(i2d_parser_node_init(parser, &node, I2D_UNARY, tokens)) {
                            status = i2d_panic("failed to create node object");
                        } else {
                            tokens = tokens->next;
                            i2d_token_remove(tokens->prev);

                            if(I2D_ADD == node->tokens->type) {
                                node->tokens->type = I2D_ADD_UNARY;
                            } else {
                                node->tokens->type = I2D_SUBTRACT_UNARY;
                            }
                        }
                    }
                    break;
                case I2D_COMMA:
                case I2D_MULTIPLY:
                case I2D_DIVIDE:
                case I2D_MODULUS:
                case I2D_ADD_ASSIGN:
                case I2D_SUBTRACT_ASSIGN:
                case I2D_MULTIPLY_ASSIGN:
                case I2D_DIVIDE_ASSIGN:
                case I2D_MODULUS_ASSIGN:
                case I2D_GREATER:
                case I2D_LESS:
                case I2D_EQUAL:
                case I2D_GREATER_EQUAL:
                case I2D_LESS_EQUAL:
                case I2D_NOT_EQUAL:
                case I2D_RIGHT_SHIFT:
                case I2D_LEFT_SHIFT:
                case I2D_BIT_AND:
                case I2D_BIT_OR:
                case I2D_BIT_XOR:
                case I2D_RIGHT_SHIFT_ASSIGN:
                case I2D_LEFT_SHIFT_ASSIGN:
                case I2D_BIT_AND_ASSIGN:
                case I2D_BIT_OR_ASSIGN:
                case I2D_BIT_XOR_ASSIGN:
                case I2D_AND:
                case I2D_OR:
                case I2D_CONDITIONAL:
                case I2D_COLON:
                case I2D_ASSIGN:
                    if(i2d_parser_node_init(parser, &node, I2D_BINARY, tokens)) {
                        status = i2d_panic("failed to create node object");
                    } else {
                        tokens = tokens->next;
                        i2d_token_remove(tokens->prev);
                    }
                    break;
                default:
                    status = i2d_panic("invalid token - %d", tokens->type);
            }

            if(node) {
                if(!root) {
                    if(I2D_BINARY == node->type) {
                        status = i2d_panic("binary operator without operand");
                    } else {
                        root = node;
                    }
                } else if(i2d_token_precedence[node->tokens->type] < i2d_token_precedence[root->tokens->type]) {
                    if(I2D_BINARY == node->type) {
                        iter = root;
                        while(iter->type == I2D_BINARY && iter->right && i2d_token_precedence[node->tokens->type] < i2d_token_precedence[iter->right->tokens->type])
                            iter = iter->right;

                        if(!iter->right) {
                            status = i2d_panic("binary operator without operand");
                        } else {
                            node->left = iter->right;
                            iter->right = node;
                        }
                    } else {
                        iter = root;
                        while(iter->right)
                            iter = iter->right;

                        if(I2D_BINARY != iter->type && I2D_UNARY != iter->type) {
                            status = i2d_panic("operand without binary operator");
                        } else {
                            iter->right = node;
                        }
                    }
                } else {
                    iter = root;
                    while(iter->right && i2d_token_right_to_left[iter->right->tokens->type])
                        iter = iter->right;

                    if(i2d_token_right_to_left[iter->tokens->type]) {
                        node->left = iter->right;
                        iter->right = node;
                    } else {
                        if(I2D_UNARY == node->type) {
                            node->right = root;
                            root = node;
                        } else if(I2D_BINARY == node->type) {
                            node->left = root;
                            root = node;
                        } else {
                            status = i2d_panic("operand without operator");
                        }
                    }
                }

                if(status)
                    i2d_node_deit(&node);
                else
                    node = NULL;
            }
        }

        if(!status) {
            if(root && I2D_BINARY == root->type && !root->right) {
                status = i2d_panic("binary operator missing right operand");
            } else if(i2d_lexer_token_init(lexer, &token, I2D_TOKEN)) {
                status = i2d_panic("failed to create token object");
            } else {
                if(i2d_node_init(&node, I2D_NODE, token)) {
                    status = i2d_panic("failed to create node object");
                } else {
                    node->left = root;
                    root = node;
                }

                if(status)
                    i2d_lexer_reset(lexer, &token);
            }
        }

        if(status) {
            i2d_deit(root, i2d_node_deit);
        } else {
            *result = root;
        }
    }

    return status;
}

static int i2d_rbt_add_variable(i2d_rbt * variables, i2d_node * node) {
    int status = I2D_OK;
    i2d_node * last;

    if(I2D_VARIABLE != node->type && I2D_INDEX != node->type) {
        status = i2d_panic("invalid node type -- %d", node->type);
    } else {
        if(!i2d_rbt_search(variables, node, (void **) &last) &&
            i2d_rbt_delete(variables, last) ) {
            status = i2d_panic("failed to replace variable");
        } else if(i2d_rbt_insert(variables, node, node)) {
            status = i2d_panic("failed to insert variable");
        }
    }

    return status;
}

static int i2d_rbt_get_variable(i2d_rbt * variables, i2d_node * key, i2d_node ** result) {
    return i2d_rbt_search(variables, key, (void **) result);
}

int i2d_script_init(i2d_script ** result, i2d_config * config, i2d_json * json) {
    int status = I2D_OK;
    i2d_script * object;
    size_t i;
    size_t size;
    i2d_handler * handler;

    if(i2d_is_invalid(result) || !config) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_db_init(&object->db, config->renewal ? i2d_renewal : i2d_pre_renewal, &config->source_path)) {
                status = i2d_panic("failed to create database object");
            } else if(i2d_lexer_init(&object->lexer)) {
                status = i2d_panic("failed to create lexer object");
            } else if(i2d_parser_init(&object->parser)) {
                status = i2d_panic("failed to create parser object");
            } else if(i2d_constant_db_init(&object->constant_db, json->constants)) {
                status = i2d_panic("failed to create constant db object");
            } else if(i2d_constant_index_mob_races(object->constant_db, object->db->mob_race_db)) {
                status = i2d_panic("failed to index mob race db");
            } else if(i2d_value_map_init(&object->getiteminfo, json->getiteminfo_type)) {
                status = i2d_panic("failed to load getiteminfo");
            } else if(i2d_value_map_init(&object->strcharinfo, json->strcharinfo_type)) {
                status = i2d_panic("failed to load strcharinfo");
            } else if(i2d_value_map_init(&object->weapons, json->weapon_type)) {
                status = i2d_panic("failed to load weapons");
            } else if(i2d_value_map_init(&object->ammos, json->ammo_type)) {
                status = i2d_panic("failed to load ammos");
            } else if(i2d_value_map_init(&object->skill_flags, json->skill_flag)) {
                status = i2d_panic("failed to load skill_flags");
            } else if(i2d_value_map_init(&object->searchstore_effect, json->searchstore_effect)) {
                status = i2d_panic("failed to load searchstore_effect");
            } else if(i2d_value_map_init(&object->bonus_script_flag, json->bonus_script_flag)) {
                status = i2d_panic("failed to load bonus_script_flag");
            } else if(i2d_data_map_init(&object->bonus, data_map_by_constant, json->bonus, object->constant_db)) {
                status = i2d_panic("failed to load bonus");
            } else if(i2d_data_map_init(&object->bonus2, data_map_by_constant, json->bonus2, object->constant_db)) {
                status = i2d_panic("failed to load bonus2");
            } else if(i2d_data_map_init(&object->bonus3, data_map_by_constant, json->bonus3, object->constant_db)) {
                status = i2d_panic("failed to load bonus3");
            } else if(i2d_data_map_init(&object->bonus4, data_map_by_constant, json->bonus4, object->constant_db)) {
                status = i2d_panic("failed to load bonus4");
            } else if(i2d_data_map_init(&object->bonus5, data_map_by_constant, json->bonus5, object->constant_db)) {
                status = i2d_panic("failed to load bonus5");
            } else if(i2d_data_map_init(&object->sc_start, data_map_by_constant, json->sc_start, object->constant_db)) {
                status = i2d_panic("failed to load sc_start");
            } else if(i2d_data_map_init(&object->sc_start2, data_map_by_constant, json->sc_start2, object->constant_db)) {
                status = i2d_panic("failed to load sc_start2");
            } else if(i2d_data_map_init(&object->sc_start4, data_map_by_constant, json->sc_start4, object->constant_db)) {
                status = i2d_panic("failed to load sc_start4");
            } else if(i2d_data_map_init(&object->functions, data_map_by_name, json->functions, object->constant_db)) {
                status = i2d_panic("failed to load functions");
            } else if(i2d_data_map_init(&object->arguments, data_map_by_name, json->arguments, object->constant_db)) {
                status = i2d_panic("failed to load arguments");
            } else if(i2d_data_map_init(&object->statements, data_map_by_name, json->statements, object->constant_db)) {
                status = i2d_panic("failed to load statements");
            } else if(i2d_buffer_cache_init(&object->buffer_cache)) {
                status = i2d_panic("failed to create buffer cache object");
            } else if(i2d_string_stack_cache_init(&object->stack_cache)) {
                status = i2d_panic("failed to create string stack cache object");
            } else if(i2d_rbt_init(&object->function_handlers, i2d_rbt_cmp_str)) {
                status = i2d_panic("failed to create read black tree object");
            } else if(i2d_rbt_init(&object->argument_handlers, i2d_rbt_cmp_str)) {
                status = i2d_panic("failed to create read black tree object");
            } else if(i2d_rbt_init(&object->statement_handlers, i2d_rbt_cmp_str)) {
                status = i2d_panic("failed to create read black tree object");
            } else {
                size = i2d_size(function_handlers);
                for(i = 0; i < size && !status; i++)
                    if(i2d_rbt_insert(object->function_handlers, function_handlers[i].name, &function_handlers[i]))
                        status = i2d_panic("failed to map handler object");

                size = i2d_size(argument_handlers);
                for(i = 0; i < size && !status; i++)
                    if(i2d_rbt_insert(object->argument_handlers, argument_handlers[i].name, &argument_handlers[i]))
                        status = i2d_panic("failed to map handler object");

                size = i2d_size(statement_handlers);
                for(i = 0; i < size && !status; i++)
                    if(i2d_rbt_insert(object->statement_handlers, statement_handlers[i].name, &statement_handlers[i]))
                        status = i2d_panic("failed to map handler object");

                for(i = 0; i < object->arguments->size && !status; i++) {
                    if(i2d_rbt_search(object->argument_handlers, object->arguments->list[i].handler.string, (void **) &handler)) {
                        status = i2d_panic("failed to find handler -- %s", object->arguments->list[i].name.string);
                    } else if(i2d_handler_list_append((i2d_handler **) &object->handlers, handler->type, &object->arguments->list[i], handler->ptr)) {
                        status = i2d_panic("failed to append handler object");
                    }
                }

                if(object->handlers) {
                    handler = object->handlers;
                    do {
                        if(i2d_rbt_insert(object->argument_handlers, handler->name, handler))
                            status = i2d_panic("failed to map handler object");
                        handler = handler->next;
                    } while(!status && handler != object->handlers);
                }
            }

            if(status)
                i2d_script_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_script_deit(i2d_script ** result) {
    i2d_script * object;
    i2d_handler * handlers;

    object = *result;
    handlers = object->handlers;
    i2d_deit(handlers, i2d_handler_list_deit);
    i2d_deit(object->statement_handlers, i2d_rbt_deit);
    i2d_deit(object->argument_handlers, i2d_rbt_deit);
    i2d_deit(object->function_handlers, i2d_rbt_deit);
    i2d_deit(object->stack_cache, i2d_string_stack_cache_deit);
    i2d_deit(object->buffer_cache, i2d_buffer_cache_deit);
    i2d_deit(object->statements, i2d_data_map_deit);
    i2d_deit(object->arguments, i2d_data_map_deit);
    i2d_deit(object->functions, i2d_data_map_deit);
    i2d_deit(object->sc_start4, i2d_data_map_deit);
    i2d_deit(object->sc_start2, i2d_data_map_deit);
    i2d_deit(object->sc_start, i2d_data_map_deit);
    i2d_deit(object->bonus5, i2d_data_map_deit);
    i2d_deit(object->bonus4, i2d_data_map_deit);
    i2d_deit(object->bonus3, i2d_data_map_deit);
    i2d_deit(object->bonus2, i2d_data_map_deit);
    i2d_deit(object->bonus, i2d_data_map_deit);
    i2d_deit(object->bonus_script_flag, i2d_value_map_deit);
    i2d_deit(object->searchstore_effect, i2d_value_map_deit);
    i2d_deit(object->skill_flags, i2d_value_map_deit);
    i2d_deit(object->ammos, i2d_value_map_deit);
    i2d_deit(object->weapons, i2d_value_map_deit);
    i2d_deit(object->strcharinfo, i2d_value_map_deit);
    i2d_deit(object->getiteminfo, i2d_value_map_deit);
    i2d_deit(object->constant_db, i2d_constant_db_deit);
    i2d_deit(object->parser, i2d_parser_deit);
    i2d_deit(object->lexer, i2d_lexer_deit);
    i2d_deit(object->db, i2d_db_deit);
    i2d_free(object);
    *result = NULL;
}

int i2d_script_compile(i2d_script * script, i2d_string * source, i2d_string * target, i2d_rbt * inherit_variables) {
    int status = I2D_OK;
    i2d_token * tokens = NULL;
    i2d_block * blocks = NULL;
    i2d_rbt * variables = NULL;
    i2d_string description;

    i2d_zero(description);

    if(!strcmp("{}", source->string)) {
        if(i2d_string_create(target, "", 0))
            status = i2d_panic("failed to create string object");
    } else {
        if( inherit_variables ?
                i2d_rbt_copy(&variables, inherit_variables) :
                i2d_rbt_init(&variables, i2d_rbt_cmp_node) ) {
            status = i2d_panic("failed to create red black tree object");
        } else {
            if(i2d_lexer_tokenize(script->lexer, source, &tokens)) {
                status = i2d_panic("failed to tokenize -- %s", source->string);
            } else {
                if(i2d_parser_analysis(script->parser, script->lexer, script->statements, tokens, &blocks)) {
                    status = i2d_panic("failed to parse -- %s", source->string);
                } else {
                    if(i2d_script_translate(script, blocks, variables, NULL)) {
                        status = i2d_panic("failed to translate -- %s", source->string);
                    } else if(i2d_script_generate(script, blocks, &blocks->buffer)) {
                        status = i2d_panic("failed to generate -- %s", source->string);
                    } else {
                        i2d_buffer_get(&blocks->buffer, &description.string, &description.length);
                        if(i2d_string_create(target, description.string, description.length))
                            status = i2d_panic("failed to create string object");
                    }
                    i2d_parser_reset(script->parser, script->lexer, &blocks);
                }
                i2d_lexer_reset(script->lexer, &tokens);
            }
            i2d_rbt_deit(&variables);
        }
    }

    return status;
}

int i2d_script_compile_node(i2d_script * script, const char * string, i2d_node ** result, i2d_rbt * inherit_variables) {
    int status = I2D_OK;
    i2d_string source;
    i2d_token * tokens = NULL;
    i2d_node * nodes = NULL;
    i2d_rbt * variables = NULL;

    i2d_zero(source);

    if(inherit_variables ?
            i2d_rbt_copy(&variables, inherit_variables) :
            i2d_rbt_init(&variables, i2d_rbt_cmp_node) ) {
        status = i2d_panic("failed to create red black tree object");
    } else {
        if(i2d_string_create(&source, string, strlen(string))) {
            status = i2d_panic("failed to create string object");
        } else {
            if(i2d_lexer_tokenize(script->lexer, &source, &tokens)) {
                status = i2d_panic("failed to tokenize -- %s", source.string);
            } else if(i2d_parser_expression_recursive(script->parser, script->lexer, tokens->next, &nodes)) {
                status = i2d_panic("failed to parse -- %s", source.string);
            } else if(i2d_script_expression(script, nodes, I2D_FLAG_NONE, variables, NULL)) {
                status = i2d_panic("failed to evaluate expression");
            } else {
                *result = nodes->left;
                nodes->left = NULL;
            }
            i2d_string_destroy(&source);
        }
        i2d_rbt_deit(&variables);
    }

    if(tokens)
        i2d_lexer_reset(script->lexer, &tokens);

    if(nodes)
        i2d_parser_node_reset(script->parser, script->lexer, &nodes);

    return status;
}

int i2d_script_compile_item_combo(i2d_script * script, i2d_item * item, i2d_string * result) {
    int status = I2D_OK;
    i2d_item_combo_list * item_combo_list;
    i2d_buffer buffer;

    size_t i;
    i2d_item_combo * item_combo;
    i2d_string list;
    i2d_string description;
    i2d_string output;

    if(i2d_item_combo_db_search_by_id(script->db->item_combo_db, item->id, &item_combo_list)) {
        if(i2d_string_create(result, "", 0))
            status = i2d_panic("failed to create string object");
    } else {
        if(i2d_buffer_create(&buffer, BUFFER_SIZE_LARGE)) {
            status = i2d_panic("failed to create buffer object");
        } else {
            for(i = 0; i < item_combo_list->size && !status; i++) {
                item_combo = item_combo_list->list[i];
                if(i2d_item_combo_get_string(item_combo, script->db->item_db, &list)) {
                    status = i2d_panic("failed to get item combo list string");
                } else {
                    if(i2d_script_compile(script, &item_combo->script, &description, NULL)) {
                        status = i2d_panic("failed to compile item combo script -- %ld", item->id);
                    } else {
                        if(description.length > 0 && i2d_buffer_printf(&buffer, "[%s Combo]\n%s", list.string, description.string))
                            status = i2d_panic("failed to write buffer object");
                        i2d_string_destroy(&description);
                    }
                    i2d_string_destroy(&list);
                }
            }

            if(!status) {
                i2d_buffer_get(&buffer, &output.string, &output.length);
                if(i2d_string_create(result, output.string, output.length))
                    status = i2d_panic("failed to create string object");
            }

            i2d_buffer_destroy(&buffer);
        }
    }

    return status;
}

int i2d_script_translate(i2d_script * script, i2d_block * blocks, i2d_rbt * variables, i2d_logic * logics) {
    int status = I2D_OK;
    i2d_block * block;

    if(blocks) {
        block = blocks;
        do {
            switch(block->type) {
                case I2D_BLOCK:
                    status = i2d_script_translate(script, block->child, variables, logics);
                    break;
                case I2D_STATEMENT:
                    if(i2d_script_expression(script, block->nodes, block->statement ? I2D_FLAG_NONE : I2D_FLAG_CONDITIONAL, variables, logics)) {
                        status = i2d_panic("failed to evaluate expression");
                    } else if(block->statement) {
                        status = i2d_script_statement(script, block, variables, logics);
                    }
                    break;
                case I2D_IF:
                    if(i2d_script_expression(script, block->nodes, I2D_FLAG_CONDITIONAL, variables, logics)) {
                        status = i2d_panic("failed to evaluate expression");
                    } else {
                        if(!block->nodes->logic && logics) {
                            if(i2d_logic_copy(&block->logics, logics))
                                status = i2d_panic("failed to copy logic object");
                        } else if(block->nodes->logic && !logics) {
                            if(i2d_logic_copy(&block->logics, block->nodes->logic))
                                status = i2d_panic("failed to copy logic object");
                        } else if(block->nodes->logic && logics) {
                            if(i2d_logic_or(&block->logics, block->nodes->logic, logics))
                                status = i2d_panic("failed to or logic object");
                        }
                        if(!status)
                            status = i2d_script_translate(script, block->child, variables, block->logics);
                    }
                    break;
                case I2D_ELSE:
                    if(logics && i2d_logic_not(&block->logics, logics))
                            status = i2d_panic("failed to not logic object");

                    if(!status)
                        status = i2d_script_translate(script, block->child, variables, block->logics);
                    break;
                case I2D_FOR:
                    /* for is unsupported */
                    break;
                default:
                    status = i2d_panic("invalid block type -- %d", block->type);
                    break;
            }
            block = block->next;
        } while(block != blocks && !status);
    }

    return status;
}

int i2d_script_generate(i2d_script * script, i2d_block * blocks, i2d_buffer * buffer) {
    int status = I2D_OK;

    i2d_block * block;
    i2d_string string;

    if(blocks) {
        block = blocks;
        do {
            switch(block->type) {
                case I2D_BLOCK:
                    status = i2d_script_generate(script, block->child, buffer);
                    break;
                case I2D_STATEMENT:
                    i2d_buffer_get(&block->buffer, &string.string, &string.length);
                    if(string.length && i2d_buffer_printf(buffer, "%s\n", string.string))
                        status = i2d_panic("failed to write buffer object");
                    break;
                case I2D_IF:
                case I2D_ELSE:
                    if(block->logics) {
                        status = i2d_buffer_printf(buffer, "[") ||
                                 i2d_script_generate_or(script, block->logics, buffer) ||
                                 i2d_buffer_printf(buffer, "]\n") ||
                                 i2d_script_generate(script, block->child, buffer);
                    } else {
                        status = i2d_script_generate(script, block->child, buffer);
                    }
                    break;
                case I2D_FOR:
                    /* for is unsupported */
                    break;
                default:
                    status = i2d_panic("invalid block type -- %d", block->type);
                    break;
            }
            block = block->next;
        } while(block != blocks && !status);
    }

    return status;
}

int i2d_script_generate_or(i2d_script * script, i2d_logic * logic, i2d_buffer * buffer) {
    int status = I2D_OK;

    switch(logic->type) {
        case or:
            status = i2d_script_generate_or(script, logic->left, buffer) ||
                     i2d_buffer_printf(buffer, " or ") ||
                     i2d_script_generate_or(script, logic->right, buffer);
            break;
        case and:
            status = i2d_script_generate_and(script, logic, buffer);
            break;
        case var:
            status = i2d_script_generate_var(script, logic, buffer);
            break;
        default:
            status = i2d_panic("invalid logic type");
            break;
    }

    return status;
}

int i2d_script_generate_and(i2d_script * script, i2d_logic * logic, i2d_buffer * buffer) {
    int status = I2D_OK;

    switch(logic->type) {
        case and:
            status = i2d_script_generate_and(script, logic->left, buffer) ||
                     i2d_buffer_printf(buffer, " and ") ||
                     i2d_script_generate_and(script, logic->right, buffer);
            break;
        case var:
            status = i2d_script_generate_var(script, logic, buffer);
            break;
        default:
            status = i2d_panic("invalid logic type");
            break;
    }

    return status;
}

int i2d_script_generate_var(i2d_script * script, i2d_logic * logic, i2d_buffer * buffer) {
    int status = I2D_OK;
    i2d_range_node * walk;

    if(i2d_buffer_printf(buffer, "%s is ", logic->name.string))
        status = i2d_panic("failed to write buffer object");
    if(logic->range.list) {
        walk = logic->range.list;
        do {
            if(walk != logic->range.list)
                if(i2d_buffer_printf(buffer, ", "))
                    status = i2d_panic("failed to write buffer object");
            if(walk->min == walk->max) {
                if(i2d_buffer_printf(buffer, "%ld", walk->min))
                    status = i2d_panic("failed to write buffer object");
            } else {
                if(i2d_buffer_printf(buffer, "%ld - %ld", walk->min, walk->max))
                    status = i2d_panic("failed to write buffer object");
            }
            walk = walk->next;
        } while(walk != logic->range.list);
    }

    return status;
}

int i2d_script_statement(i2d_script * script, i2d_block * block, i2d_rbt * variables, i2d_logic * logics) {
    int status = I2D_OK;
    i2d_handler * handler;

    if(!block->statement->handler.string) {
        status = i2d_panic("invalid handler string");
    } else if(i2d_rbt_search(script->statement_handlers, block->statement->handler.string, (void **) &handler)) {
        status = i2d_panic("failed to find handler -- %s", block->statement->handler.string);
    } else {
        switch(handler->type) {
            case block_statement:
                status = handler->block_statement(script, block, variables, block->statement);
                break;
            default:
                status = i2d_panic("invalid handler type -- %d", handler->type);
        }
    }

    return status;
}

int i2d_script_statement_ignore(i2d_script * script, i2d_block * block, i2d_rbt * variables, i2d_data * statement) {
    return I2D_OK;
}

int i2d_script_statement_set(i2d_script * script, i2d_block * block, i2d_rbt * variables, i2d_data * statement) {
    int status = I2D_OK;
    i2d_node * node;

    node = block->nodes->left;

    if(i2d_node_copy(node, node->right)) {
        status = i2d_panic("failed to copy node object");
    } else if(i2d_node_copy(node->left, node)) {
        status = i2d_panic("failed to copy node object");
    } else if(i2d_rbt_add_variable(variables, node->left)) {
        status = i2d_panic("failed to add variable");
    } else {
        /*
         * assign the expression (node->right) to the variable (node->left)
         */
        node->left->left = node->right;
        node->right = NULL;
    }

    return status;
}

int i2d_script_statement_generic(i2d_script * script, i2d_block * block, i2d_rbt * variables, i2d_data * statement) {
    int status = I2D_OK;
    i2d_node * arguments[MAX_ARGUMENT];

    size_t i;
    i2d_string * list;
    size_t size;
    i2d_node * defaults[MAX_ARGUMENT];

    i2d_zero(arguments);
    i2d_zero(defaults);

    if(MAX_ARGUMENT <= statement->required + statement->optional) {
        status = i2d_panic("MAX_ARGUMENT overflow");
    } else if(i2d_node_get_arguments(block->nodes->tokens->type == I2D_TOKEN ? block->nodes->left : block->nodes, arguments, statement->required, statement->optional)) {
        status = i2d_panic("failed to get arguments");
    } else {
        i2d_string_stack_get(&statement->argument_default, &list, &size);
        for(i = 0; i < (size_t) statement->optional && i < size; i++) {
            if(!arguments[i + statement->required]) {
                if(i2d_script_compile_node(script, list[i].string, &defaults[i], variables)) {
                    status = i2d_panic("failed to create default node object");
                } else {
                    arguments[i + statement->required] = defaults[i];
                }
            }
        }

        if(i2d_script_statement_evaluate(script, variables, arguments, statement, &block->buffer))
            status = i2d_panic("failed to handle statement arguments");

        for(i = 0; i < (size_t) statement->optional && i < size; i++)
            if(defaults[i])
                i2d_parser_node_reset(script->parser, script->lexer, &defaults[i]);
    }

    return status;
}

int i2d_script_statement_evaluate(i2d_script * script, i2d_rbt * variables, i2d_node ** arguments, i2d_data * statement, i2d_buffer * buffer) {
    int status = I2D_OK;
    i2d_local local;

    size_t i;
    size_t size;
    size_t last;
    i2d_string * list;
    i2d_handler * handler;
    int is_empty;

    i2d_zero(local);

    if(i2d_local_create(&local, script)) {
        status = i2d_panic("failed to create local object");
    } else {
        if(i2d_string_stack_get(&statement->argument_type, &list, &size)) {
            status = i2d_panic("failed to get argument type array");
        } else {
            if(statement->argument_order.list) {
                /*
                 * specified order
                 */
                if(size != statement->argument_order.size) {
                    status = i2d_panic("argument type and order size mismatch");
                } else {
                    for(i = 0; i < size && !status; i++) {
                        i2d_buffer_clear(local.buffer);

                        if(i2d_rbt_search(script->argument_handlers, list[i].string, (void **) &handler)) {
                            status = i2d_panic("failed to find handler -- %s", list[i].string);
                        } else if(!arguments[statement->argument_order.list[i]]) {
                            break;
                        } else {
                            switch(handler->type) {
                                case single_node:
                                    status = handler->single_node(script, variables, arguments[statement->argument_order.list[i]], &local);
                                    break;
                                case multiple_node:
                                    status = handler->multiple_node(script, variables, &arguments[statement->argument_order.list[i]], &local);
                                    break;
                                case single_node_data:
                                    status = handler->single_node_data(handler->data, script, variables, arguments[statement->argument_order.list[i]], &local);
                                    break;
                                default:
                                    status = i2d_panic("invalid handler type -- %d", handler->type);
                            }
                        }
                    }
                }
            } else {
                /*
                 * default order
                 */
                for(i = 0; i < size && arguments[i] && !status; i++) {
                    i2d_buffer_clear(local.buffer);

                    if(i2d_rbt_search(script->argument_handlers, list[i].string, (void **) &handler)) {
                        status = i2d_panic("failed to find handler -- %s", list[i].string);
                    } else {
                        switch(handler->type) {
                            case single_node:
                                status = handler->single_node(script, variables, arguments[i], &local);
                                break;
                            case multiple_node:
                                status = handler->multiple_node(script, variables, &arguments[i], &local);
                                break;
                            case single_node_data:
                                status = handler->single_node_data(handler->data, script, variables, arguments[i], &local);
                                break;
                            default:
                                status = i2d_panic("invalid handler type -- %d", handler->type);
                        }
                    }
                }
            }
        }

        if(statement->dump_stack_instead_of_description) {
            if(i2d_string_stack_get(local.stack, &list, &size)) {
                status = i2d_panic("failed to get argument stack array");
            } else {
                for(i = 0, last = 0; i < size && !status; i++)
                    if(list[i].length) {
                        if( (i && list[last].length && i2d_buffer_printf(buffer, "\n")) ||
                            i2d_buffer_printf(buffer, "%s", list[i].string) ) {
                            status = i2d_panic("failed to write buffer object");
                        } else {
                            last = i;
                        }
                    }
            }
        } else if(statement->empty_description_on_empty_string) {
            if(i2d_string_stack_get(local.stack, &list, &size)) {
                status = i2d_panic("failed to get argument stack array");
            } else {
                is_empty = I2D_OK;
                for(i = 0; i < size && !status && !is_empty; i++)
                    if(!list[i].length)
                        is_empty = I2D_FAIL;

                if(!is_empty && i2d_format_write(&statement->description, local.stack, buffer))
                    status = i2d_panic("failed to write bonus type description");
            }
        } else if(!status && i2d_format_write(&statement->description, local.stack, buffer)) {
            status = i2d_panic("failed to write bonus type description");
        }

        if(i2d_local_destroy(&local, script))
            status = i2d_panic("failed to destroy local object");
    }

    return status;
}

int i2d_script_expression(i2d_script * script, i2d_node * node, int flag, i2d_rbt * variables, i2d_logic * logics) {
    int status = I2D_OK;
    i2d_logic * conditional = NULL;
    i2d_string name;

    if(node->left && i2d_script_expression(script, node->left, flag | (i2d_node_is_conditional(node) ? 0 : I2D_FLAG_CONDITIONAL), variables, logics)) {
        status = i2d_panic("failed to evaluate left expression");
    } else {
        if(node->right && i2d_script_expression(script, node->right, flag, variables, i2d_script_expression_conditional(script, node, logics, &conditional) ? logics : conditional)) {
            status = i2d_panic("failed to evaluate right expression");
        } else {
            switch(node->type) {
                case I2D_NODE:
                    status = (node->left) ? i2d_node_copy(node, node->left) : i2d_range_create_add(&node->range, 0, 0);
                    break;
                case I2D_VARIABLE:
                    status = i2d_script_expression_variable(script, node, variables, logics);
                    break;
                case I2D_FUNCTION:
                    status = i2d_script_expression_function(script, node, variables);
                    break;
                case I2D_INDEX:
                    /* index node is unsupported */
                    break;
                case I2D_UNARY:
                    status = i2d_script_expression_unary(script, node, flag);
                    break;
                case I2D_BINARY:
                    status = i2d_script_expression_binary(script, node, flag, variables);
                    break;
                default:
                    status = i2d_panic("invalid node type -- %d", node->type);
                    break;
            }

            if(logics && (I2D_VARIABLE == node->type || I2D_FUNCTION == node->type)) {
                if(i2d_node_get_string(node, &name)) {
                    status = i2d_panic("failed to get string from node object");
                } else if(i2d_logic_search(logics, name.string, &node->range)) {
                    status = i2d_panic("failed to search logic object");
                }
            }
        }
    }

    i2d_deit(conditional, i2d_logic_deit);
    return status;
}

int i2d_script_expression_conditional(i2d_script * script, i2d_node * node, i2d_logic * logics, i2d_logic ** result) {
    int status = I2D_FAIL;
    i2d_logic * logic = NULL;

    if(node->left && node->left->logic) {
        if(!i2d_node_is_conditional(node)) {
            if( logics ?
                    i2d_logic_or(result, node->left->logic, logics) :
                    i2d_logic_copy(result, node->left->logic) ) {
                status = i2d_panic("failed to or logic object");
            } else {
                status = I2D_OK;
            }
        } else if(!i2d_node_is_colon(node)) {
            if(i2d_logic_not(&logic, node->left->logic)) {
                status = i2d_panic("failed to not logic object");
            } else {
                if( logics ?
                        i2d_logic_or(result, logic, logics) :
                        i2d_logic_copy(result, logic) ) {
                    status = i2d_panic("failed to or logic object");
                } else {
                    status = I2D_OK;
                }
                i2d_logic_deit(&logic);
            }
        }
    }

    return status;
}

int i2d_script_expression_variable_predicate(i2d_script * script, i2d_node * node, i2d_node * variable) {
    int status = I2D_OK;
    i2d_local local;
    i2d_string predicate;

    i2d_zero(local);
    i2d_zero(predicate);

    if(i2d_local_create(&local, script)) {
        status = i2d_panic("failed to create local object");
    } else {
        if( i2d_node_get_predicate_all(variable, local.stack) ||
            i2d_string_stack_get_unique(local.stack, local.buffer) ) {
            status = i2d_panic("failed to get predicate list");
        } else {
            i2d_buffer_get(local.buffer, &predicate.string, &predicate.length);
            if(predicate.length && i2d_node_set_string(node, &predicate)) {
                status = i2d_panic("failed to write predicate list");
            }
        }
        if(i2d_local_destroy(&local, script))
            status = i2d_panic("failed to destroy local object");
    }

    return status;
}

int i2d_script_expression_variable(i2d_script * script, i2d_node * node, i2d_rbt * variables, i2d_logic * logics) {
    int status = I2D_OK;
    i2d_node * variable;
    long number;
    i2d_string name;
    i2d_constant * constant;

    if(!i2d_rbt_get_variable(variables, node, &variable)) {
        if(i2d_node_copy(node, variable)) {
            status = i2d_panic("failed to copy variable");
        } else if(i2d_script_expression_variable_predicate(script, node, variable)) {
            status = i2d_panic("failed to copy predicate list");
        }
    } else if(i2d_node_get_string(node, &name)) {
        status = i2d_panic("failed to get variable string");
    } else {
        if(!i2d_is_number(&name) && !i2d_token_get_constant(node->tokens, &number)) {
            node->type = I2D_NUMBER;
            if(i2d_range_create_add(&node->range, number, number))
                status = i2d_panic("failed to create range object");
        } else if(!i2d_constant_get_by_macro(script->constant_db, name.string, &constant)) {
            if(i2d_node_set_constant(node, constant))
                status = i2d_panic("failed to set constant on node object");
        } else {
            if(i2d_range_create_add(&node->range, 0, 0))
                status = i2d_panic("failed to create range object");
        }
    }

    return status;
}


int i2d_script_expression_function(i2d_script * script, i2d_node * node, i2d_rbt * variables) {
    int status = I2D_OK;
    i2d_string name;
    i2d_handler * handler;
    i2d_local local;

    i2d_zero(local);

    if(i2d_local_create(&local, script)) {
        status = i2d_panic("failed to create local object");
    } else {
        if(i2d_node_get_string(node, &name)) {
            status = i2d_panic("failed to get function string");
        } else if(i2d_rbt_search(script->function_handlers, name.string, (void **) &handler)) {
            status = i2d_panic("failed to get function handler -- %s", name.string);
        } else {
            status = handler->single_node(script, variables, node, &local);
        }
        if(i2d_local_destroy(&local, script))
            status = i2d_panic("failed to destroy local object");
    }

    return status;
}

int i2d_script_expression_unary(i2d_script * script, i2d_node * node, int flag) {
    int status = I2D_OK;

    long min;
    long max;

    if(!node->right) {
        status = i2d_panic("unary operator missing operand");
    } else {
        switch(node->tokens->type) {
            case I2D_NOT:
                if(flag & I2D_FLAG_CONDITIONAL) {
                    if(i2d_range_not(&node->range, &node->right->range)) {
                        status = i2d_panic("failed to not range object");
                    } else if(node->right->logic && i2d_logic_not(&node->logic, node->right->logic)) {
                        status = i2d_panic("failed to not logic object");
                    }
                } else {
                    status = i2d_range_create_add(&node->range, 0, 1);
                }
                break;
            case I2D_BIT_NOT:
                if(i2d_range_bitnot(&node->range, &node->right->range))
                    status = i2d_panic("failed to bit not range object");
                break;
            case I2D_ADD_UNARY:
                if(i2d_node_copy(node, node->right))
                    status = i2d_panic("failed to copy range object");
                break;
            case I2D_SUBTRACT_UNARY:
                if(i2d_range_negate(&node->range, &node->right->range))
                    status = i2d_panic("failed to negate range object");
                break;
            case I2D_INCREMENT:
                i2d_range_get_range(&node->right->range, &min, &max);
                status = i2d_range_create_add(&node->range, min++, max++);
                break;
            case I2D_DECREMENT:
                i2d_range_get_range(&node->right->range, &min, &max);
                status = i2d_range_create_add(&node->range, min--, max--);
                break;
            default:
                status = i2d_panic("invalid node type -- %d", node->tokens->type);
        }
    }

    return status;
}

int i2d_script_expression_binary_assign(i2d_node * node, int operator, i2d_rbt * variables) {
    int status = I2D_OK;

    if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, operator)) {
        status = i2d_panic("failed to compute range -- %d", operator);
    } else if(i2d_node_copy(node->left, node)) {
        status = i2d_panic("failed to copy node to left node");
    } else if(i2d_rbt_add_variable(variables, node->left)) {
        status = i2d_panic("failed to add variable");
    } else {
        /*
         * assign the expression (node->right) to the variable (node->left)
         */
        node->left->left = node->right;
        node->right = NULL;
    }

    return status;
}

int i2d_script_expression_binary_relational(i2d_node * node, int operator, int flag) {
    int status = I2D_OK;
    i2d_string predicate;
    i2d_zero(predicate);

    if(flag & I2D_FLAG_CONDITIONAL) {
        if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, operator)) {
            status = i2d_panic("failed to compute range -- %d", operator);
        } else if(!i2d_node_get_predicate(node, &predicate) && i2d_logic_init(&node->logic, &predicate, &node->range)) {
            status = i2d_panic("failed to create logic object");
        }
    } else {
        status = i2d_range_create_add(&node->range, 0, 1);
    }

    return status;
}

int i2d_script_expression_binary_logical(i2d_node * node, int operator, int flag) {
    int status = I2D_OK;

    if(flag & I2D_FLAG_CONDITIONAL) {
        if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, operator)) {
            status = i2d_panic("failed to compute range -- %d", operator);
        } else if(node->left->logic && node->right->logic) {
            if( operator == '|' + '|' ?
                    i2d_logic_or(&node->logic, node->left->logic, node->right->logic) :
                    i2d_logic_and(&node->logic, node->left->logic, node->right->logic) )
                status = i2d_panic("failed to create logic object");
        }
    } else {
        status = i2d_range_create_add(&node->range, 0, 1);
    }

    return status;
}

int i2d_script_expression_binary(i2d_script * script, i2d_node * node, int flag, i2d_rbt * variables) {
    int status = I2D_OK;

    if(!node->left) {
        status = i2d_panic("binary operator missing left operand");
    } else if(!node->right) {
        status = i2d_panic("binary operator missing right operand");
    } else {
        switch(node->tokens->type) {
            case I2D_ADD_ASSIGN:
                if(i2d_script_expression_binary_assign(node, '+', variables))
                    status = i2d_panic("failed to add assign left operand");
                break;
            case I2D_ADD:
                if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, '+'))
                    status = i2d_panic("failed to add left and right operand");
                break;
            case I2D_SUBTRACT_ASSIGN:
                if(i2d_script_expression_binary_assign(node, '-', variables))
                    status = i2d_panic("failed to subtract assign left operand");
                break;
            case I2D_SUBTRACT:
                if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, '-'))
                    status = i2d_panic("failed to subtract left and right operand");
                break;
            case I2D_MULTIPLY_ASSIGN:
                if(i2d_script_expression_binary_assign(node, '*', variables))
                    status = i2d_panic("failed to multiply assign left operand");
                break;
            case I2D_MULTIPLY:
                if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, '*'))
                    status = i2d_panic("failed to multiply left and right operand");
                break;
            case I2D_DIVIDE_ASSIGN:
                if(i2d_script_expression_binary_assign(node, '/', variables))
                    status = i2d_panic("failed to divide assign left operand");
                break;
            case I2D_DIVIDE:
                if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, '/'))
                    status = i2d_panic("failed to divide left and right operand");
                break;
            case I2D_MODULUS_ASSIGN:
                if(i2d_script_expression_binary_assign(node, '%', variables))
                    status = i2d_panic("failed to modulus assign left operand");
                break;
            case I2D_MODULUS:
                if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, '%'))
                    status = i2d_panic("failed to modulus left and right operand");
                break;
            case I2D_COMMA:
                if(i2d_node_copy(node, node->right))
                    status = i2d_panic("failed to copy right operand");
                break;
            case I2D_RIGHT_SHIFT_ASSIGN:
                if(i2d_script_expression_binary_assign(node, '>' + '>' + 'b', variables))
                    status = i2d_panic("failed to right shift assign left operand");
                break;
            case I2D_RIGHT_SHIFT:
                if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, '>' + '>' + 'b'))
                    status = i2d_panic("failed to right shift operand");
                break;
            case I2D_LEFT_SHIFT_ASSIGN:
                if(i2d_script_expression_binary_assign(node, '<' + '<' + 'b', variables))
                    status = i2d_panic("failed to left shift assign left operand");
                break;
            case I2D_LEFT_SHIFT:
                if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, '<' + '<' + 'b'))
                    status = i2d_panic("failed to left shift operand");
                break;
            case I2D_BIT_AND_ASSIGN:
                if(i2d_script_expression_binary_assign(node, '&', variables))
                    status = i2d_panic("failed to bit and assign left operand");
                break;
            case I2D_BIT_AND:
                if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, '&'))
                    status = i2d_panic("failed to bit and left and right operand");
                break;
            case I2D_BIT_OR_ASSIGN:
                if(i2d_script_expression_binary_assign(node, '|', variables))
                    status = i2d_panic("failed to bit or assign left operand");
                break;
            case I2D_BIT_OR:
                if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, '|'))
                    status = i2d_panic("failed to bit or left and right operand");
                break;
            case I2D_BIT_XOR_ASSIGN:
                if(i2d_script_expression_binary_assign(node, '^' + 'b', variables))
                    status = i2d_panic("failed to bit xor assign left operand");
                break;
            case I2D_BIT_XOR:
                if(i2d_range_compute(&node->range, &node->left->range, &node->right->range, '^' + 'b'))
                    status = i2d_panic("failed to bit xor left and right operand");
                break;
            case I2D_GREATER:
                if(i2d_script_expression_binary_relational(node, '>', flag))
                    status = i2d_panic("failed to compute left is greater than right operand");
                break;
            case I2D_GREATER_EQUAL:
                if(i2d_script_expression_binary_relational(node, '>' + '=', flag))
                    status = i2d_panic("failed to compute left is greater or equal than right operand");
                break;
            case I2D_LESS:
                if(i2d_script_expression_binary_relational(node, '<', flag))
                    status = i2d_panic("failed to compute left is less than right operand");
                break;
            case I2D_LESS_EQUAL:
                if(i2d_script_expression_binary_relational(node, '<' + '=', flag))
                    status = i2d_panic("failed to compute left is less or equal than right operand");
                break;
            case I2D_EQUAL:
                if(i2d_script_expression_binary_relational(node, '=' + '=', flag))
                    status = i2d_panic("failed to compute left is equal to right operand");
                break;
            case I2D_NOT_EQUAL:
                if(i2d_script_expression_binary_relational(node, '!' + '=', flag))
                    status = i2d_panic("failed to compute left is equal to right operand");
                break;
            case I2D_AND:
                if(i2d_script_expression_binary_logical(node, '&' + '&', flag))
                    status = i2d_panic("failed to compute left and right operand");
                break;
            case I2D_OR:
                if(i2d_script_expression_binary_logical(node, '|' + '|', flag))
                    status = i2d_panic("failed to compute left or right operand");
                break;
            case I2D_CONDITIONAL:
                if(i2d_node_copy(node, node->right)) {
                    status = i2d_panic("failed to copy node object");
                } else if(node->left->logic && i2d_logic_copy(&node->logic, node->left->logic)) {
                    status = i2d_panic("failed to copy logic object");
                }
                break;
            case I2D_COLON:
                if(i2d_script_expression_binary_logical(node, '|' + '|', flag | I2D_FLAG_CONDITIONAL))
                    status = i2d_panic("failed to compute left or right operand");
                break;
            case I2D_ASSIGN:
                if(i2d_node_copy(node, node->right)) {
                    status = i2d_panic("failed to copy node object");
                } else if(i2d_node_copy(node->left, node)) {
                    status = i2d_panic("failed to copy node object");
                } else if(i2d_rbt_add_variable(variables, node->left)) {
                    status = i2d_panic("failed to add variable");
                } else {
                    /*
                     * assign the expression (node->right) to the variable (node->left)
                     */
                    node->left->left = node->right;
                    node->right = NULL;
                }
                break;
            default:
                status = i2d_panic("invalid token type -- %d", node->tokens->type);
        }
    }

    return status;
}

int i2d_item_script_create(i2d_item_script * result, i2d_script * script, i2d_item * item) {
    int status = I2D_OK;
    i2d_item_script output;
    i2d_zero(output);

    if(i2d_script_compile(script, &item->script, &output.script, NULL)) {
        status = i2d_panic("failed to compile script -- %ld", item->id);
    } else if(i2d_script_compile(script, &item->onequip_script, &output.onequip, NULL)) {
        status = i2d_panic("failed to compile onequip script -- %ld", item->id);
    } else if(i2d_script_compile(script, &item->onunequip_script, &output.onunequip, NULL)) {
        status = i2d_panic("failed to compile onunequip script -- %ld", item->id);
    } else if(i2d_script_compile_item_combo(script, item, &output.combo)) {
        status = i2d_panic("failed to compile item combo script -- %ld", item->id);
    } else {
        output.item = item;
    }

    if(status)
        i2d_item_script_destroy(&output);
    else
        *result = output;

    return status;
}

void i2d_item_script_destroy(i2d_item_script * result) {
    i2d_string_destroy(&result->script);
    i2d_string_destroy(&result->onequip);
    i2d_string_destroy(&result->onunequip);
    i2d_string_destroy(&result->combo);
}

int i2d_local_create(i2d_local * result, i2d_script * script) {
    int status = I2D_OK;

    if(i2d_buffer_cache_get(script->buffer_cache, &result->buffer)) {
        status = i2d_panic("failed to create buffer object");
    } else if(i2d_string_stack_cache_get(script->stack_cache, &result->stack)) {
        status = i2d_panic("failed to create string stack object");
    }

    return status;
}

int i2d_local_destroy(i2d_local * result, i2d_script * script) {
    int status = I2D_OK;

    if(i2d_string_stack_cache_put(script->stack_cache, &result->stack)) {
        status = i2d_panic("failed to cache string stack object");
    } else if(i2d_buffer_cache_put(script->buffer_cache, &result->buffer)) {
        status = i2d_panic("failed to cache buffer object");
    }

    return status;
}

static int i2d_handler_init(i2d_handler ** result, enum i2d_handler_type type, i2d_data * data, void * handler) {
    int status = I2D_OK;
    i2d_handler * object = NULL;

    if(i2d_is_invalid(result) || !data) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_string_copy(&object->name, data->name.string, data->name.length)) {
                status = i2d_panic("failed to copy string object");
            } else {
                object->type = type;
                switch(object->type) {
                    case single_node:
                        object->single_node = handler;
                        break;
                    case multiple_node:
                        object->multiple_node = handler;
                        break;
                    case single_node_data:
                        object->single_node_data = handler;
                        break;
                    case block_statement:
                        object->block_statement = handler;
                        break;
                    default:
                        status = i2d_panic("invalid handler type -- %d", object->type);
                        break;
                }
                object->data = data;
                object->next = object;
                object->prev = object;
            }

            if(status)
                i2d_handler_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

static void i2d_handler_deit(i2d_handler ** result) {
    i2d_handler * object;

    object = *result;
    i2d_free(object->name);
    i2d_free(object);
    *result = NULL;
}

static void i2d_handler_list_deit(i2d_handler ** result) {
    i2d_handler * object;
    i2d_handler * token;

    object = *result;
    if(object) {
        while(object != object->next) {
            token = object->next;
            i2d_handler_remove(token);
            i2d_handler_deit(&token);
        }
        i2d_handler_deit(&object);
    }
    *result = NULL;
}

static void i2d_handler_append(i2d_handler * x, i2d_handler * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

static void i2d_handler_remove(i2d_handler * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

static int i2d_handler_list_append(i2d_handler ** result, enum i2d_handler_type type, i2d_data * data, void * cb) {
    int status = I2D_OK;
    i2d_handler * handler = NULL;

    if(i2d_handler_init(&handler, type, data, cb)) {
        status = i2d_panic("failed to create handler object");
    } else {
        if(!*result) {
            *result = handler;
        } else {
            i2d_handler_append(handler, *result);
        }
    }

    return status;
}


static int i2d_handler_general(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    i2d_string name;
    i2d_data * data;

    if(i2d_node_get_string(node, &name)) {
        status = i2d_panic("failed to get function string");
    } else if(i2d_data_map_get(script->functions, name.string, &data)) {
        status = i2d_panic("failed to get function data -- %s", name.string);
    } else {
        i2d_buffer_clear(&node->tokens->buffer);

        if(i2d_format_write(&data->description, local->stack, &node->tokens->buffer)) {
            status = i2d_panic("failed to write function format");
        } else if(i2d_range_copy(&node->range, &data->range)) {
            status = i2d_panic("failed to copy function range");
        }
    }

    return status;
}

static int i2d_handler_readparam(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    i2d_node * arguments[2];
    long value;
    i2d_constant * constant;

    i2d_zero(arguments);

    if(i2d_node_get_arguments(node->left, arguments, 1, 1)) {
        status = i2d_panic("failed to get readparam arguments");
    } else if(i2d_node_get_constant(arguments[0], &value)) {
        status = i2d_panic("failed to get paramater number");
    } else if(i2d_constant_get_by_readparam(script->constant_db, value, &constant)) {
        status = i2d_panic("failed to get constant by paramater number -- %ld", value);
    } else if(i2d_node_set_constant(node, constant)) {
        status = i2d_panic("failed to write paramater");
    }

    return status;
}

static int i2d_handler_getskilllv(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    i2d_node * arguments;
    long id;
    i2d_string name;
    i2d_skill * skill = NULL;

    if(i2d_node_get_arguments(node->left, &arguments, 1, 0)) {
        status = i2d_panic("failed to get getskilllv arguments");
    } else if(i2d_node_get_constant(arguments, &id)) {
        status = i2d_panic("failed to get skill id");
    } else if(i2d_skill_db_search_by_id(script->db->skill_db, id, &skill)) {
        if(i2d_node_get_string(arguments, &name)) {
            status = i2d_panic("failed to get skill string");
        } else if(i2d_skill_db_search_by_macro(script->db->skill_db, name.string, &skill)) {
            status = i2d_panic("failed to get skill by id and name -- %ld %s", id, name.string);
        }
    }

    if(!status) {
        if(i2d_node_set_string(node, &skill->name)) {
            status = i2d_panic("failed to write skill string");
        } else if(i2d_range_create_add(&node->range, 0, skill->maxlv)) {
            status = i2d_panic("failed to create skill range");
        }
    }

    return status;
}

static int i2d_handler_isequipped(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    i2d_node * arguments[MAX_ARGUMENT];
    size_t i;
    size_t size;
    long id;
    i2d_item * item;

    i2d_zero(arguments);
    size = i2d_size(arguments);

    if(i2d_node_get_arguments(node->left, arguments, 1, size - 1)) {
        status = i2d_panic("failed to get arguments");
    } else {
        for(i = 0; i < size && arguments[i] && !status; i++) {
            if(i2d_node_get_constant(arguments[i], &id)) {
                status = i2d_panic("failed to get item id");
            } else if(i2d_item_db_search_by_id(script->db->item_db, id, &item)) {
                status = i2d_panic("failed to get item by id -- %ld", id);
            } else {
                if( i ?
                    i2d_buffer_printf(local->buffer, ", %s", item->name.string) :
                    i2d_buffer_printf(local->buffer, "%s", item->name.string) )
                    status = i2d_panic("failed to write expression buffer");
            }
        }
    }

    if(!status) {
        if(i2d_string_stack_push_buffer(local->stack, local->buffer)) {
            status = i2d_panic("failed to push item list");
        } else {
            status = i2d_handler_general(script, variables, node, local);
        }
    }

    return status;
}

static int i2d_handler_countitem(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    i2d_node * arguments;
    long id;
    i2d_string name;
    i2d_item * item = NULL;

    if(i2d_node_get_arguments(node->left, &arguments, 1, 0)) {
        status = i2d_panic("failed to get countitem arguments");
    } else if(i2d_node_get_constant(arguments, &id)) {
        status = i2d_panic("failed to get item id");
    } else if(i2d_item_db_search_by_id(script->db->item_db, id, &item)) {
        if(i2d_node_get_string(arguments, &name)) {
            status = i2d_panic("failed to get item string");
        } else if(i2d_item_db_search_by_name(script->db->item_db, name.string, &item)) {
            status = i2d_panic("failed to get item by id and string -- %ld %s", id, name.string);
        }
    }

    if(!status) {
        if(i2d_string_stack_push(local->stack, item->name.string, item->name.length)) {
            status = i2d_panic("failed to push item name");
        } else {
            status = i2d_handler_general(script, variables, node, local);
        }
    }

    return status;
}

static int i2d_handler_gettime(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    i2d_node * arguments;
    long value;
    i2d_constant * constant;

    if(i2d_node_get_arguments(node->left, &arguments, 1, 0)) {
        status = i2d_panic("failed to get gettime arguments");
    } else if(i2d_node_get_constant(arguments, &value)) {
        status = i2d_panic("failed to get tick type");
    } else if(i2d_constant_get_by_gettime(script->constant_db, value, &constant)) {
        status = i2d_panic("failed to get constant tick type -- %ld", value);
    } else if(i2d_node_set_constant(node, constant)) {
        status = i2d_panic("failed to write tick type");
    }

    return status;
}

static int i2d_handler_strcharinfo(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    i2d_node * arguments[2];
    long value;
    i2d_string string;

    i2d_zero(arguments);

    if(i2d_node_get_arguments(node->left, arguments, 1, 1)) {
        status = i2d_panic("failed to get strcharinfo arguments");
    } else if(i2d_node_get_constant(arguments[0], &value)) {
        status = i2d_panic("failed to get type");
    } else if(i2d_value_map_get(script->strcharinfo, value, &string)) {
        status = i2d_panic("failed to get strcharinfo by type -- %ld", value);
    } else if(i2d_node_set_string(node, &string)) {
        status = i2d_panic("failed to write strcharinfo string");
    } else if(i2d_range_copy(&node->range, &arguments[0]->range)) {
        status = i2d_panic("failed to copy strcharinfo range");
    }

    return status;
}

static int i2d_handler_getequipid(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    i2d_node * arguments[2];
    long value;
    i2d_constant * constant;

    i2d_zero(arguments);

    if(i2d_node_get_arguments(node->left, arguments, 0, 2)) {
        status = i2d_panic("failed to get getequipid arguments");
    } else {
        if(arguments[0]) {
            if(i2d_node_get_constant(arguments[0], &value)) {
                status = i2d_panic("failed to get equipment slot");
            } else if(i2d_constant_get_by_location(script->constant_db, value, &constant)) {
                status = i2d_panic("failed to get constant by equipment slot -- %ld", value);
            } else if(i2d_node_set_constant(node, constant)) {
                status = i2d_panic("failed to write equipment slot");
            }
        } else {
            status = i2d_handler_general(script, variables, node, local);
        }
    }

    return status;
}

static int i2d_handler_getiteminfo(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    i2d_node * arguments[2];
    long value;
    i2d_string string;
    i2d_item * item;

    if(i2d_node_get_arguments(node->left, arguments, i2d_size(arguments), 0)) {
        status = i2d_panic("failed to get getiteminfo arguments");
    } else {
        if(I2D_FUNCTION == arguments[0]->type) {
            if(i2d_node_get_string(arguments[0], &string)) {
                status = i2d_panic("failed to get function string");
            } else if(i2d_string_stack_push(local->stack, string.string, string.length)) {
                status = i2d_panic("failed to push function string");
            }
        } else {
            if(i2d_node_get_constant(arguments[0], &value)) {
                status = i2d_panic("failed to get item id");
            } else if(i2d_item_db_search_by_id(script->db->item_db, value, &item)) {
                status = i2d_panic("failed to get item by id -- %ld", value);
            } else if(i2d_string_stack_push(local->stack, item->name.string, item->name.length)) {
                status = i2d_panic("failed to push function string");
            }
        }

        if(!status) {
            if(i2d_node_get_constant(arguments[1], &value)) {
                status = i2d_panic("failed to get type");
            } else if(i2d_value_map_get(script->getiteminfo, value, &string)) {
                status = i2d_panic("failed to get getiteminfo by type -- %ld", value);
            } else if(i2d_string_stack_push(local->stack, string.string, string.length)) {
                status = i2d_panic("failed to push getiteminfo string");
            } else {
                status = i2d_handler_general(script, variables, node, local);
            }
        }
    }

    return status;
}

static int i2d_handler_getmapflag(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    i2d_node * arguments[3];
    long value;
    i2d_string string;
    i2d_constant * constant;

    i2d_zero(arguments);

    if(i2d_node_get_arguments(node->left, arguments, 2, 1)) {
        status = i2d_panic("failed to get getmapflag arguments");
    } else {
        if(i2d_node_get_string(arguments[0], &string)) {
            status = i2d_panic("failed to get function string");
        } else if(i2d_string_stack_push(local->stack, string.string, string.length)) {
            status = i2d_panic("failed to push function string");
        }

        if(!status) {
            if(i2d_node_get_constant(arguments[1], &value)) {
                status = i2d_panic("failed to get map flag");
            } else if(i2d_constant_get_by_mapflag(script->constant_db, value, &constant)) {
                status = i2d_panic("failed to get constant by map flag -- %ld", value);
            } else if(i2d_string_stack_push(local->stack, constant->name.string, constant->name.length)) {
                status = i2d_panic("failed to push map flag string");
            } else {
                status = i2d_handler_general(script, variables, node, local);
            }
        }
    }

    return status;
}

static int i2d_handler_max(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;

    i2d_node * arguments[2];

    long xmin;
    long xmax;
    long ymin;
    long ymax;

    if(i2d_node_get_arguments(node->left, arguments, 2, 0)) {
        status = i2d_panic("failed to get max arguments");
    } else {
        i2d_range_get_range(&arguments[0]->range, &xmin, &xmax);
        i2d_range_get_range(&arguments[1]->range, &ymin, &ymax);
        if(i2d_range_create_add(&node->range, max(xmin, ymin), max(xmax, ymax))) {
            status = i2d_panic("failed to create range object");
        } else if(i2d_script_expression_variable_predicate(script, node, node->left)) {
            status = i2d_panic("failed to copy predicate list");
        }
    }

    return status;
}

static int i2d_handler_min(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;

    i2d_node * arguments[2];

    long xmin;
    long xmax;
    long ymin;
    long ymax;

    if(i2d_node_get_arguments(node->left, arguments, 2, 0)) {
        status = i2d_panic("failed to get min arguments");
    } else {
        i2d_range_get_range(&arguments[0]->range, &xmin, &xmax);
        i2d_range_get_range(&arguments[1]->range, &ymin, &ymax);
        if(i2d_range_create_add(&node->range, min(xmin, ymin), min(xmax, ymax))) {
            status = i2d_panic("failed to create range object");
        } else if(i2d_script_expression_variable_predicate(script, node, node->left)) {
            status = i2d_panic("failed to copy predicate list");
        }
    }

    return status;
}

static int i2d_handler_getequiprefinerycnt_cb(i2d_script * script, i2d_string_stack * stack, long location) {
    int status = I2D_OK;
    i2d_constant * constant;

    if(i2d_constant_get_by_location(script->constant_db, location, &constant)) {
        status = i2d_panic("failed to get location -- %ld", location);
    } else if(i2d_string_stack_push(stack, constant->name.string, constant->name.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_getequiprefinerycnt(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    i2d_node * argument;

    if(i2d_node_get_arguments(node->left, &argument, 1, 0)) {
        status = i2d_panic("failed to getequiprefinerycnt argument");
    } else if(i2d_handler_range(script, argument, local, i2d_handler_getequiprefinerycnt_cb)) {
        status = i2d_panic("failed to resolve location range");
    } else if(i2d_string_stack_push_buffer(local->stack, local->buffer)) {
        status = i2d_panic("failed to write location list to stack");
    } else {
        status = i2d_handler_general(script, variables, node, local);
    }

    return status;
}

static int i2d_handler_pow(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    i2d_node * arguments[2];

    long min;
    long max;
    long pow_min;
    long pow_max;

    if(i2d_node_get_arguments(node->left, arguments, 2, 0)) {
        status = i2d_panic("failed to get pow arguments");
    } else {
        i2d_range_get_range(&arguments[0]->range, &min, &max);
        i2d_range_get_range(&arguments[1]->range, &pow_min, &pow_max);
        if(i2d_range_create_add(&node->range, (long) pow(min, pow_min), (long) pow(max, pow_max))) {
            status = i2d_panic("failed to create range object");
        } else if(i2d_script_expression_variable_predicate(script, node, node->left)) {
            status = i2d_panic("failed to copy predicate list");
        }
    }

    return status;
}

struct i2d_checkoption {
    i2d_script * script;
    i2d_local * local;
};

typedef struct i2d_checkoption i2d_checkoption;

static int i2d_handler_checkoption_loop(uint64_t flag, void * data) {
    int status = I2D_OK;
    i2d_checkoption * context = data;
    i2d_constant * constant = NULL;

    if(i2d_constant_get_by_options(context->script->constant_db, (long) flag, &constant)) {
        status = i2d_panic("failed to get option by value -- %" PRIu64, flag);
    } else if(i2d_string_stack_push(context->local->stack, constant->name.string, constant->name.length)) {
        status = i2d_panic("failed to push on stack");
    }

    return status;
}

static int i2d_handler_checkoption(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    i2d_node * argument;
    long flag;
    i2d_checkoption context;

    if(i2d_node_get_arguments(node->left, &argument, 1, 0)) {
        status = i2d_panic("failed to checkoption argument");
    } else if(i2d_node_get_constant(argument, &flag)) {
        status = i2d_panic("failed to get option flag");
    } else {
        context.script = script;
        context.local = local;
        if(i2d_by_bit64(flag, i2d_handler_checkoption_loop, &context)) {
            status = i2d_panic("failed to get option by flag -- %ld", flag);
        } else if(i2d_string_stack_get_unique(local->stack, local->buffer)) {
            status = i2d_panic("failed to get option list from stack");
        } else {
            i2d_string_stack_clear(local->stack);
            if(i2d_string_stack_push_buffer(local->stack, local->buffer)) {
                status = i2d_panic("failed to write option list to stack");
            } else {
                status = i2d_handler_general(script, variables, node, local);
            }
        }
    }

    return status;
}

static int i2d_handler_rand(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    i2d_node * arguments[2];

    long lmin;
    long lmax;
    long rmin;
    long rmax;

    i2d_zero(arguments);

    if(i2d_handler_general(script, variables, node, local)) {
        status = i2d_panic("failed to handle rand function");
    } else if(i2d_node_get_arguments(node->left, arguments, 1, 1)) {
        status = i2d_panic("failed to rand arguments");
    } else {
        i2d_range_destroy(&node->range);
        i2d_range_get_range(&arguments[0]->range, &lmin, &lmax);
        if(arguments[1]) {
            i2d_range_get_range(&arguments[1]->range, &rmin, &rmax);
            if(i2d_range_create_add(&node->range, min(lmin, rmin), max(lmax, rmax)))
                status = i2d_panic("failed to create range object");
        } else {
            if(i2d_range_create_add(&node->range, 0, lmax - 1))
                status = i2d_panic("failed to create range object");
        }

        if(!status && i2d_script_expression_variable_predicate(script, node, node->left))
            status = i2d_panic("failed to copy predicate list");
    }

    return status;
}

static int i2d_handler_callfunc(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    i2d_node * arguments[MAX_ARGUMENT];
    i2d_string function;

    size_t i;
    i2d_range result;

    i2d_zero(arguments);
    i2d_zero(result);

    if(i2d_node_get_arguments(node->left, arguments, 1, MAX_ARGUMENT - 1)) {
        status = i2d_panic("failed to get callfunc arguments");
    } else if(i2d_node_get_string(arguments[0], &function)) {
        status = i2d_panic("failed to get function string");
    } else if(!strcmp(function.string, "F_Rand")) {
        for(i = 1; i < MAX_ARGUMENT; i++) {
            if(!arguments[i]) {
                break;
            } else {
                if(i2d_range_or(&result, &node->range, &arguments[i]->range)) {
                    status = i2d_panic("failed to or range object");
                } else {
                    i2d_range_destroy(&node->range);
                    node->range = result;
                    i2d_zero(result);
                }
            }
        }
    } else {
        /* unsupported callfunc function */
    }

    return status;
}

static int i2d_handler_getequipweaponlv(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    i2d_node * arguments[2];
    long value;
    i2d_constant * constant;

    i2d_zero(arguments);

    if(i2d_node_get_arguments(node->left, arguments, 1, 1)) {
        status = i2d_panic("failed to get getequipweaponlv arguments");
    } else if(i2d_node_get_constant(arguments[0], &value)) {
        status = i2d_panic("failed to get equipment slot");
    } else if(i2d_constant_get_by_location(script->constant_db, value, &constant)) {
        status = i2d_panic("failed to get location by value -- %ld", value);
    } else if(i2d_string_stack_push(local->stack, constant->name.string, constant->name.length)) {
        status = i2d_panic("failed to push location string");
    } else {
        status = i2d_handler_general(script, variables, node, local);
    }

    return status;
}

static int i2d_handler_getexp2(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    i2d_node * arguments[3];

    long min;
    long max;

    i2d_zero(arguments);

    if(i2d_node_get_arguments(node->left, arguments, 2, 1)) {
        status = i2d_panic("failed to get getexp2 arguments");
    } else {
        i2d_range_get_range(&arguments[0]->range, &min, &max);
        if( min == max ?
            i2d_buffer_printf(local->buffer, "%ld", min) :
            i2d_buffer_printf(local->buffer, "%ld ~ %ld", min, max) ) {
            status = i2d_panic("failed to write integer range");
        } else if(i2d_string_stack_push_buffer(local->stack, local->buffer)) {
            status = i2d_panic("failed to push buffer on stack");
        } else {
            i2d_buffer_clear(local->buffer);

            i2d_range_get_range(&arguments[1]->range, &min, &max);
            if( min == max ?
                i2d_buffer_printf(local->buffer, "%ld", min) :
                i2d_buffer_printf(local->buffer, "%ld ~ %ld", min, max) ) {
                status = i2d_panic("failed to write integer range");
            } else if(i2d_string_stack_push_buffer(local->stack, local->buffer)) {
                status = i2d_panic("failed to push buffer on stack");
            } else {
                status = i2d_handler_general(script, variables, node, local);
            }
        }
    }

    return status;
}

static int i2d_handler_range(i2d_script * script, i2d_node * node, i2d_local * local, i2d_handler_range_cb cb) {
    int status = I2D_OK;
    i2d_string_stack * stack = NULL;
    i2d_range_node * range = NULL;
    long i;

    if(i2d_string_stack_cache_get(script->stack_cache, &stack)) {
        status = i2d_panic("failed to create string stack object");
    } else {
        if(!node->range.list) {
            status = i2d_panic("empty range list");
        } else {
            range = node->range.list;
            do {
                for( i = range->min; i <= range->max && !status; i++)
                    status = cb(script, stack, i);
                range = range->next;
            } while(range != node->range.list && !status);

            if(!status)
                if(i2d_string_stack_get_unique(stack, local->buffer))
                    status = i2d_panic("failed to push buffer on stack");
        }

        if(i2d_string_stack_cache_put(script->stack_cache, &stack))
            status = i2d_panic("failed to cache string stack object");
    }

    return status;
}

static int i2d_handler_expression(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    i2d_local predicate;
    i2d_string string;

    i2d_zero(predicate);
    i2d_zero(string);

    if(i2d_local_create(&predicate, script)) {
        status = i2d_panic("failed to create local object");
    } else {
        if( i2d_node_get_predicate_all(node, predicate.stack) ||
            i2d_string_stack_get_unique(predicate.stack, predicate.buffer) ) {
            status = i2d_panic("failed to get predicate list");
        } else {
            i2d_buffer_get(predicate.buffer, &string.string, &string.length);
            if(string.length && i2d_buffer_printf(local->buffer, " (%s)", string.string)) {
                status = i2d_panic("failed to write predicate string");
            } else if(i2d_string_stack_push_buffer(local->stack, local->buffer)) {
                status = i2d_panic("failed to push string on stack");
            }
        }
        if(i2d_local_destroy(&predicate, script))
            status = i2d_panic("failed to destroy local object");
    }

    return status;
}

static int i2d_handler_milliseconds(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long min;
    long max;

    char * suffix = NULL;
    long unit = 0;

    i2d_range_get_range(&node->range, &min, &max);

    if (min / 86400000 > 0) {
        suffix = "day";
        unit = 86400000;
    } else if (min / 3600000 > 0) {
        suffix = "hour";
        unit = 3600000;
    } else if (min / 60000 > 0) {
        suffix = "minute";
        unit = 60000;
    } else if (min / 1000 > 0) {
        suffix = "second";
        unit = 1000;
    } else {
        suffix = "millisecond";
        unit = 1;
    }

    min /= unit;
    max /= unit;

    if( min == max ?
        i2d_buffer_printf(local->buffer, "%ld %s%s", min, suffix, min > 1 ? "s" : "") :
        i2d_buffer_printf(local->buffer, "%ld ~ %ld %s%s", min, max, suffix, max > 1 ? "s" : "") ) {
        status = i2d_panic("failed to write time range");
    } else if(i2d_handler_expression(script, variables, node, local)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_handler_seconds(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long min;
    long max;

    char * suffix = NULL;
    long unit = 0;

    i2d_range_get_range(&node->range, &min, &max);

    if (min / 86400 > 0) {
        suffix = "day";
        unit = 86400;
    } else if (min / 3600 > 0) {
        suffix = "hour";
        unit = 3600;
    } else if (min / 60 > 0) {
        suffix = "minute";
        unit = 60;
    } else {
        suffix = "second";
        unit = 1;
    }

    min /= unit;
    max /= unit;

    if( min == max ?
        i2d_buffer_printf(local->buffer, "%ld %s%s", min, suffix, min > 1 ? "s" : "") :
        i2d_buffer_printf(local->buffer, "%ld ~ %ld %s%s", min, max, suffix, max > 1 ? "s" : "") ) {
        status = i2d_panic("failed to write time range");
    } else if(i2d_handler_expression(script, variables, node, local)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_handler_regen(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long constant;

    if(i2d_node_get_constant(node, &constant)) {
        status = i2d_panic("failed to get constant");
    } else {
        switch(constant) {
            case 1: status = i2d_buffer_printf(local->buffer, "HP"); break;
            case 2: status = i2d_buffer_printf(local->buffer, "SP"); break;
            default: status = i2d_panic("unsupported regen value -- %ld", constant); break;
        }
        if(status) {
            status = i2d_panic("failed to write regen");
        } else if(i2d_handler_expression(script, variables, node, local)) {
            status = i2d_panic("failed to write expression");
        }
    }

    return status;
}

static int i2d_handler_splash(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long min;
    long max;

    i2d_range_get_range(&node->range, &min, &max);
    min = min * 2 + 1;
    max = max * 2 + 1;

    if( min == max ?
        i2d_buffer_printf(local->buffer, "%ld x %ld", min, min) :
        i2d_buffer_printf(local->buffer, "%ld x %ld ~ %ld x %ld", min, min, max, max) ) {
        status = i2d_panic("failed to write splash range");
    } else if(i2d_handler_expression(script, variables, node, local)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_handler_elements_cb(i2d_script * script, i2d_string_stack * stack, long id) {
    int status = I2D_OK;
    i2d_constant * constant;

    if(i2d_constant_get_by_element(script->constant_db, id, &constant)) {
        status = i2d_panic("failed to get element -- %ld", id);
    } else if(i2d_string_stack_push(stack, constant->name.string, constant->name.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_elements(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    return  i2d_handler_range(script, node, local, i2d_handler_elements_cb) ||
            i2d_handler_expression(script, variables, node, local);
}

static int i2d_handler_races_cb(i2d_script * script, i2d_string_stack * stack, long id) {
    int status = I2D_OK;
    i2d_constant * constant;

    if(i2d_constant_get_by_race(script->constant_db, id, &constant)) {
        status = i2d_panic("failed to get race -- %ld", id);
    } else if(i2d_string_stack_push(stack, constant->name.string, constant->name.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_races(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    return  i2d_handler_range(script, node, local, i2d_handler_races_cb) ||
            i2d_handler_expression(script, variables, node, local);
}

static int i2d_handler_classes_cb(i2d_script * script, i2d_string_stack * stack, long id) {
    int status = I2D_OK;
    i2d_constant * constant;

    if(i2d_constant_get_by_class(script->constant_db, id, &constant)) {
        status = i2d_panic("failed to get class -- %ld", id);
    } else if(i2d_string_stack_push(stack, constant->name.string, constant->name.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_classes(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    return  i2d_handler_range(script, node, local, i2d_handler_classes_cb) ||
            i2d_handler_expression(script, variables, node, local);
}

static int i2d_handler_integer(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long min;
    long max;

    i2d_range_get_range(&node->range, &min, &max);
    if( min == max ?
        i2d_buffer_printf(local->buffer, "%ld", min) :
        i2d_buffer_printf(local->buffer, "%ld ~ %ld", min, max) ) {
        status = i2d_panic("failed to write integer range");
    } else if(i2d_handler_expression(script, variables, node, local)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_handler_integer_sign(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long min;
    long max;

    i2d_range_get_range(&node->range, &min, &max);
    if( min == max ?
        i2d_buffer_printf(local->buffer, "%+ld", min) :
        i2d_buffer_printf(local->buffer, "%+ld ~ %+ld", min, max) ) {
        status = i2d_panic("failed to write integer range");
    } else if(i2d_handler_expression(script, variables, node, local)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_handler_integer_absolute(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long min;
    long max;

    i2d_range_get_range_absolute(&node->range, &min, &max);
    if( min == max ?
        i2d_buffer_printf(local->buffer, "%ld", min) :
        i2d_buffer_printf(local->buffer, "%ld ~ %ld", min, max) ) {
        status = i2d_panic("failed to write integer range");
    } else if(i2d_handler_expression(script, variables, node, local)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_handler_percent(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long min;
    long max;

    i2d_range_get_range(&node->range, &min, &max);
    if( min == max ?
        i2d_buffer_printf(local->buffer, "%ld%%", min) :
        i2d_buffer_printf(local->buffer, "%ld%% ~ %ld%%", min, max) ) {
        status = i2d_panic("failed to write percent range");
    } else if(i2d_handler_expression(script, variables, node, local)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_handler_percent_sign(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long min;
    long max;

    i2d_range_get_range(&node->range, &min, &max);
    if( min == max ?
        i2d_buffer_printf(local->buffer, "%+ld%%", min) :
        i2d_buffer_printf(local->buffer, "%+ld%% ~ %+ld%%", min, max) ) {
        status = i2d_panic("failed to write percent range");
    } else if(i2d_handler_expression(script, variables, node, local)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_handler_percent_sign_inverse(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long min;
    long max;

    i2d_range_get_range(&node->range, &min, &max);

    min *= -1;
    max *= -1;

    if( min == max ?
        i2d_buffer_printf(local->buffer, "%+ld%%", min) :
        i2d_buffer_printf(local->buffer, "%+ld%% ~ %+ld%%", min, max) ) {
        status = i2d_panic("failed to write percent range");
    } else if(i2d_handler_expression(script, variables, node, local)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_handler_percent_absolute(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long min;
    long max;

    i2d_range_get_range_absolute(&node->range, &min, &max);
    if( min == max ?
        i2d_buffer_printf(local->buffer, "%ld%%", min) :
        i2d_buffer_printf(local->buffer, "%ld%% ~ %ld%%", min, max) ) {
        status = i2d_panic("failed to write percent range");
    } else if(i2d_handler_expression(script, variables, node, local)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_handler_percent10(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long min;
    long max;

    i2d_range_get_range(&node->range, &min, &max);

    min /= 10;
    max /= 10;

    if( min == max ?
        i2d_buffer_printf(local->buffer, "%ld%%", min) :
        i2d_buffer_printf(local->buffer, "%ld%% ~ %ld%%", min, max) ) {
        status = i2d_panic("failed to write percent range");
    } else if(i2d_handler_expression(script, variables, node, local)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_handler_percent100(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long min;
    long max;

    i2d_range_get_range(&node->range, &min, &max);

    min /= 100;
    max /= 100;

    if( min == max ?
        i2d_buffer_printf(local->buffer, "%ld%%", min) :
        i2d_buffer_printf(local->buffer, "%ld%% ~ %ld%%", min, max) ) {
        status = i2d_panic("failed to write percent range");
    } else if(i2d_handler_expression(script, variables, node, local)) {
        status = i2d_panic("failed to write expression");
    }

    return status;
}

static int i2d_handler_ignore(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;

    if(i2d_string_stack_push(local->stack, "ignore", 6))
        status = i2d_panic("failed to push string on stack");

    return status;
}

static int i2d_handler_sizes_cb(i2d_script * script, i2d_string_stack * stack, long id) {
    int status = I2D_OK;
    i2d_constant * constant;

    if(i2d_constant_get_by_size(script->constant_db, id, &constant)) {
        status = i2d_panic("failed to get size -- %ld", id);
    } else if(i2d_string_stack_push(stack, constant->name.string, constant->name.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_sizes(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    return  i2d_handler_range(script, node, local, i2d_handler_sizes_cb) ||
            i2d_handler_expression(script, variables, node, local);
}

static int i2d_handler_skill_cb(i2d_script * script, i2d_string_stack * stack, long id) {
    int status = I2D_OK;
    i2d_skill * skill = NULL;

    if(i2d_skill_db_search_by_id(script->db->skill_db, id, &skill)) {
        status = i2d_panic("failed to get skill by id -- %ld", id);
    } else if(i2d_string_stack_push(stack, skill->name.string, skill->name.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_skill(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    i2d_string name;
    i2d_skill * skill = NULL;

    if(i2d_node_get_string(node, &name)) {
        status = i2d_panic("failed to get skill name");
    } else if(i2d_skill_db_search_by_macro(script->db->skill_db, name.string, &skill)) {
        status =    i2d_handler_range(script, node, local, i2d_handler_skill_cb) ||
                    i2d_handler_expression(script, variables, node, local);
    } else if(i2d_string_stack_push(local->stack, skill->name.string, skill->name.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_mob_cb(i2d_script * script, i2d_string_stack * stack, long id) {
    int status = I2D_OK;
    i2d_mob * mob;
    i2d_constant * constant;

    if(!i2d_mob_db_search_by_id(script->db->mob_db, id, &mob)) {
        if(i2d_string_stack_push(stack, mob->kro.string, mob->kro.length))
            status = i2d_panic("failed to push string on stack");
    } else if(!i2d_constant_get_by_job(script->constant_db, id, &constant)) {
        if(i2d_string_stack_push(stack, constant->name.string, constant->name.length))
            status = i2d_panic("failed to push string on stack");
    } else {
        status = i2d_panic("failed to get mob or job by id -- %ld", id);
    }

    return status;
}

static int i2d_handler_mob(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    return  i2d_handler_range(script, node, local, i2d_handler_mob_cb) ||
            i2d_handler_expression(script, variables, node, local);
}

static int i2d_handler_effects_cb(i2d_script * script, i2d_string_stack * stack, long id) {
    int status = I2D_OK;
    i2d_constant * constant;

    if(i2d_constant_get_by_effect(script->constant_db, id, &constant)) {
        status = i2d_panic("failed to get effect -- %ld", id);
    } else if(i2d_string_stack_push(stack, constant->name.string, constant->name.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_effects(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    return  i2d_handler_range(script, node, local, i2d_handler_effects_cb) ||
            i2d_handler_expression(script, variables, node, local);
}

static int i2d_handler_mob_races_cb(i2d_script * script, i2d_string_stack * stack, long id) {
    int status = I2D_OK;
    i2d_constant * constant;

    if(i2d_constant_get_by_mob_races(script->constant_db, id, &constant)) {
        status = i2d_panic("failed to get mob race by id -- %ld", id);
    } else if(i2d_string_stack_push(stack, constant->name.string, constant->name.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_mob_races(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    return  i2d_handler_range(script, node, local, i2d_handler_mob_races_cb) ||
            i2d_handler_expression(script, variables, node, local);
}

static int i2d_handler_weapons_cb(i2d_script * script, i2d_string_stack * stack, long id) {
    int status = I2D_OK;
    i2d_string weapon;

    if(i2d_value_map_get(script->weapons, id, &weapon)) {
        status = i2d_panic("failed to get weapon by weapon type -- %ld", id);
    } else if(i2d_string_stack_push(stack, weapon.string, weapon.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_weapons(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    return  i2d_handler_range(script, node, local, i2d_handler_weapons_cb) ||
            i2d_handler_expression(script, variables, node, local);
}

static int i2d_handler_zeny(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;

    long min;
    long max;

    i2d_range_get_range(&node->range, &min, &max);

    if(is_negative(min)) {
        if(is_positive(max)) {
            status = i2d_panic("zeny range cannot be both negative and positive");
        } else {
            min *= -1;
            max *= -1;
            if( min == max ?
                i2d_buffer_printf(local->buffer, "%ld * Monster Level", min) :
                i2d_buffer_printf(local->buffer, "(%ld ~ %ld) * Monster Level", min, max) ) {
                status = i2d_panic("failed to write zeny formula");
            } else if(i2d_handler_expression(script, variables, node, local)) {
                status = i2d_panic("failed to write expression");
            }
        }
    } else {
        if( min == max ?
            i2d_buffer_printf(local->buffer, "%ld", min) :
            i2d_buffer_printf(local->buffer, "%ld ~ %ld", min, max) ) {
            status = i2d_panic("failed to write zeny range");
        } else if(i2d_handler_expression(script, variables, node, local)) {
            status = i2d_panic("failed to write expression");
        }
    }

    return status;
}

static int i2d_handler_item_cb(i2d_script * script, i2d_string_stack * stack, long id) {
    int status = I2D_OK;
    i2d_item * item;

    if(i2d_item_db_search_by_id(script->db->item_db, id, &item)) {
        status = i2d_panic("failed to get item by id -- %ld", id);
    } else if(i2d_string_stack_push(stack, item->name.string, item->name.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_item(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    return  i2d_handler_range(script, node, local, i2d_handler_item_cb) ||
            i2d_handler_expression(script, variables, node, local);
}

static int i2d_handler_itemgroups_cb(i2d_script * script, i2d_string_stack * stack, long id) {
    int status = I2D_OK;
    i2d_constant * constant;

    if(i2d_constant_get_by_itemgroups(script->constant_db, id, &constant)) {
        status = i2d_panic("failed to get item group by id -- %ld", id);
    } else if(i2d_string_stack_push(stack, constant->name.string, constant->name.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_itemgroups(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    return  i2d_handler_range(script, node, local, i2d_handler_itemgroups_cb) ||
            i2d_handler_expression(script, variables, node, local);
}

static int i2d_handler_bf_type(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long mask;
    i2d_constant * BF_SHORT;
    i2d_constant * BF_LONG;
    i2d_constant * BF_WEAPON;
    i2d_constant * BF_MAGIC;
    i2d_constant * BF_MISC;
    i2d_constant * BF_NORMAL;
    i2d_constant * BF_SKILL;

    if(i2d_node_get_constant(node, &mask)) {
        status = i2d_panic("failed to get bf mask");
    } else {
        BF_SHORT = script->constant_db->BF_SHORT;
        BF_LONG = script->constant_db->BF_LONG;
        BF_WEAPON = script->constant_db->BF_WEAPON;
        BF_MAGIC = script->constant_db->BF_MAGIC;
        BF_MISC = script->constant_db->BF_MISC;
        BF_NORMAL = script->constant_db->BF_NORMAL;
        BF_SKILL = script->constant_db->BF_SKILL;

        /*
         * default is BF_SHORT and BF_LONG
         */
        if(!(mask & BF_SHORT->value) && !(mask & BF_LONG->value))
            mask |= (BF_SHORT->value | BF_LONG->value);

        /*
         * default is BF_WEAPON
         */
        if(!(mask & BF_WEAPON->value) && !(mask & BF_MAGIC->value) && !(mask & BF_MISC->value))
            mask |= BF_WEAPON->value;

        /*
         * if BF_WEAPON then BF_NORMAL and if BF_SKILL then BF_SKILL
         */
        if(!(mask & BF_NORMAL->value) && !(mask & BF_SKILL->value))
            mask |= (mask & BF_WEAPON->value) ? BF_NORMAL->value : BF_SKILL->value;

        if(!((BF_SHORT->value | BF_LONG->value) == (mask & (BF_SHORT->value | BF_LONG->value)))) {
            if(mask & BF_SHORT->value) {
                status = i2d_buffer_printf(local->buffer, "%s ", BF_SHORT->name.string);
            } else if(mask & BF_LONG->value) {
                status = i2d_buffer_printf(local->buffer, "%s ", BF_LONG->name.string);
            }
        }

        if(status) {
            status = i2d_panic("failed to write buffer");
        } else {
            if((BF_NORMAL->value | BF_SKILL->value) == (mask & (BF_NORMAL->value | BF_SKILL->value))) {
                status = i2d_buffer_printf(local->buffer, "%s / %s", BF_NORMAL->name.string, BF_SKILL->name.string);
            } else if(mask & BF_NORMAL->value) {
                status = i2d_buffer_printf(local->buffer, "%s", BF_NORMAL->name.string);
            } else if(mask & BF_SKILL->value) {
                status = i2d_buffer_printf(local->buffer, "%s", BF_SKILL->name.string);
            }

            if(status) {
                status = i2d_panic("failed to write buffer");
            } else if(i2d_string_stack_push_buffer(local->stack, local->buffer)) {
                status = i2d_panic("failed to push string on stack");
            }
        }
    }

    return status;
}

static int i2d_handler_bf_damage(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long mask;

    i2d_constant * BF_WEAPON;
    i2d_constant * BF_MAGIC;
    i2d_constant * BF_MISC;

    if(i2d_node_get_constant(node, &mask)) {
        status = i2d_panic("failed to get bf mask");
    } else {
        BF_WEAPON = script->constant_db->BF_WEAPON;
        BF_MAGIC = script->constant_db->BF_MAGIC;
        BF_MISC = script->constant_db->BF_MISC;

        /*
         * default is BF_WEAPON
         */
        if(!(mask & BF_WEAPON->value) && !(mask & BF_MAGIC->value) && !(mask & BF_MISC->value))
            mask |= BF_WEAPON->value;

        if(mask & BF_WEAPON->value)
            status = i2d_buffer_printf(local->buffer, "%s", BF_WEAPON->name.string);
        if(!status && mask & BF_MAGIC->value)
            status = i2d_buffer_printf(local->buffer, "%s%s", local->buffer->offset ? ", " : "", BF_MAGIC->name.string);
        if(!status && mask & BF_MISC->value)
            status = i2d_buffer_printf(local->buffer, "%s%s", local->buffer->offset ? ", " : "", BF_MISC->name.string);

        if(status) {
            status = i2d_panic("failed to write buffer");
        } else if(i2d_string_stack_push_buffer(local->stack, local->buffer)) {
            status = i2d_panic("failed to push string on stack");
        }
    }

    return status;
}

static int i2d_handler_atf_target(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long mask;

    i2d_constant * ATF_SELF;
    i2d_constant * ATF_TARGET;

    if(i2d_node_get_constant(node, &mask)) {
        status = i2d_panic("failed to get bf mask");
    } else {
        ATF_SELF = script->constant_db->ATF_SELF;
        ATF_TARGET = script->constant_db->ATF_TARGET;

        /*
         * default is ATF_TARGET
         */
        if(!(mask & ATF_SELF->value) && !(mask & ATF_TARGET->value))
            mask |= ATF_TARGET->value;

        if(mask & ATF_SELF->value) {
            if(i2d_string_stack_push(local->stack, ATF_SELF->name.string, ATF_SELF->name.length))
                status = i2d_panic("failed to push string on stack");
        } else if(mask & ATF_TARGET->value) {
            if(i2d_string_stack_push(local->stack, ATF_TARGET->name.string, ATF_TARGET->name.length))
                status = i2d_panic("failed to push string on stack");
        }
    }

    return status;
}

static int i2d_handler_atf_type(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long mask;

    i2d_constant * ATF_SHORT;
    i2d_constant * ATF_LONG;
    i2d_constant * ATF_WEAPON;
    i2d_constant * ATF_MAGIC;
    i2d_constant * ATF_MISC;

    if(i2d_node_get_constant(node, &mask)) {
        status = i2d_panic("failed to get bf mask");
    } else {
        ATF_SHORT = script->constant_db->ATF_SHORT;
        ATF_LONG = script->constant_db->ATF_LONG;
        ATF_WEAPON = script->constant_db->ATF_WEAPON;
        ATF_MAGIC = script->constant_db->ATF_MAGIC;
        ATF_MISC = script->constant_db->ATF_MISC;

        /*
         * default is ATF_SHORT and ATF_LONG
         */
        if(!(mask & ATF_SHORT->value) && !(mask & ATF_LONG->value))
            mask |= (ATF_SHORT->value | ATF_LONG->value);

        /*
         * default is AFT_WEAPON
         */
        if(!(mask & ATF_WEAPON->value) && !(mask & ATF_MAGIC->value) && !(mask & ATF_MISC->value))
            mask |= ATF_WEAPON->value;

        if(!((ATF_SHORT->value | ATF_LONG->value) == (mask & (ATF_SHORT->value | ATF_LONG->value)))) {
            if(mask & ATF_SHORT->value) {
                status = i2d_buffer_printf(local->buffer, "%s ", ATF_SHORT->name.string);
            } else if(mask & ATF_LONG->value) {
                status = i2d_buffer_printf(local->buffer, "%s ", ATF_LONG->name.string);
            }
        }
        if(status) {
            status = i2d_panic("failed to write buffer");
        } else {
            if(mask & ATF_WEAPON->value)
                status = i2d_buffer_printf(local->buffer, "%s", ATF_WEAPON->name.string);
            if(!status && mask & ATF_MAGIC->value)
                status = i2d_buffer_printf(local->buffer, "%s%s", local->buffer->offset ? ", " : "", ATF_MAGIC->name.string);
            if(!status && mask & ATF_MISC->value)
                status = i2d_buffer_printf(local->buffer, "%s%s", local->buffer->offset ? ", " : "", ATF_MISC->name.string);

            if(status) {
                status = i2d_panic("failed to write buffer");
            } else if(i2d_string_stack_push_buffer(local->stack, local->buffer)) {
                status = i2d_panic("failed to push string on stack");
            }
        }
    }

    return status;
}

static int i2d_handler_script(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    i2d_string string;
    i2d_string description;

    if(i2d_node_get_string(node, &string)) {
        status = i2d_panic("failed to get script");
    } else {
        if( string.string[0] != '{' ?
            i2d_buffer_printf(local->buffer, "{%s}", string.string) :
            i2d_buffer_printf(local->buffer, "%s", string.string) ) {
            status = i2d_panic("failed to write buffer object");
        } else {
            i2d_buffer_get(local->buffer, &string.string, &string.length);
            if(i2d_script_compile(script, &string, &description, variables)) {
                status = i2d_panic("failed to compile script -- %s", string.string);
            } else {
                if(i2d_string_stack_push(local->stack, description.string, description.length))
                    status = i2d_panic("failed to push string on stack");

                i2d_string_destroy(&description);
            }
        }
    }

    return status;
}

static int i2d_handler_skill_flags(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long flag;
    i2d_string string;

    if(i2d_node_get_constant(node, &flag)) {
        status = i2d_panic("failed to get skill flag");
    } else if(i2d_value_map_get(script->skill_flags, flag, &string)) {
        status = i2d_panic("failed to get skill flag by flag -- %ld", flag);
    } else if(i2d_string_stack_push(local->stack, string.string, string.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_string(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    i2d_string string;

    if(i2d_node_get_string(node, &string)) {
        status = i2d_panic("failed to get string object");
    } else if(i2d_string_stack_push(local->stack, string.string, string.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_searchstore_effect(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long flag;
    i2d_string string;

    if(i2d_node_get_constant(node, &flag)) {
        status = i2d_panic("failed to get searchstore effect");
    } else if(i2d_value_map_get(script->searchstore_effect, flag, &string)) {
        status = i2d_panic("failed to get searchstore effect by flag -- %ld", flag);
    } else if(i2d_string_stack_push(local->stack, string.string, string.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_announce_flag(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long flag;
    i2d_constant * constant;

    if(i2d_node_get_constant(node, &flag)) {
        status = i2d_panic("failed to get announce flag");
    } else if(i2d_constant_get_by_announces(script->constant_db, flag, &constant)) {
        status = i2d_panic("failed to get announce flag by flag -- %ld", flag);
    } else if(i2d_string_stack_push(local->stack, constant->name.string, constant->name.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_mercenary_cb(i2d_script * script, i2d_string_stack * stack, long id) {
    int status = I2D_OK;
    i2d_mercenary * mercenary;

    if(i2d_mercenary_db_search_by_id(script->db->mercenary_db, id, &mercenary)) {
        status = i2d_panic("failed to get mercenary by id -- %ld", id);
    } else if(i2d_string_stack_push(stack, mercenary->name.string, mercenary->name.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_mercenary(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    return  i2d_handler_range(script, node, local, i2d_handler_mercenary_cb) ||
            i2d_handler_expression(script, variables, node, local);
}

struct i2d_bonus_script {
    i2d_script * script;
    i2d_string_stack * stack;
};

typedef struct i2d_bonus_script i2d_bonus_script;

static int i2d_handler_bonus_script_flag_cb(uint64_t bit, void * data) {
    int status = I2D_OK;
    i2d_bonus_script * context = data;
    long flag = (long) bit;
    i2d_string string;

    i2d_zero(string);

    if(i2d_value_map_get(context->script->bonus_script_flag, flag, &string)) {
        status = i2d_panic("failed to get bonus script by flag -- %ld", flag);
    } else if(i2d_string_stack_push(context->stack, string.string, string.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_bonus_script_flag(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long flag;
    i2d_string_stack * stack = NULL;
    i2d_bonus_script context;
    i2d_string * list;
    size_t size;
    size_t i;

    if(i2d_node_get_constant(node, &flag)) {
        status = i2d_panic("failed to get bonus script flag");
    } else if(i2d_string_stack_cache_get(script->stack_cache, &stack)) {
        status = i2d_panic("failed to create string stack object");
    } else {
        context.script = script;
        context.stack = stack;
        if(i2d_by_bit64(flag, i2d_handler_bonus_script_flag_cb, &context)) {
            status = i2d_panic("failed to get bonus script by flag -- %ld", flag);
        } else if(i2d_string_stack_get(stack, &list, &size)) {
            status = i2d_panic("failed to get string stack");
        } else {
            for(i = 0; i < size && !status; i++)
                if(list[i].length && i2d_buffer_printf(local->buffer, "%s\n", list[i].string))
                    status = i2d_panic("failed to write buffer object");

            if(!status && i2d_string_stack_push_buffer(local->stack, local->buffer))
                status = i2d_panic("failed to push string on stack");
        }

        if(i2d_string_stack_cache_put(script->stack_cache, &stack))
            status = i2d_panic("failed to cache string stack object");
    }


    return status;
}

static int i2d_handler_pet_cb(i2d_script * script, i2d_string_stack * stack, long id) {
    int status = I2D_OK;
    i2d_pet * pet;

    if(i2d_pet_db_search_by_id(script->db->pet_db, id, &pet)) {
        status = i2d_panic("failed to get pet by id -- %ld", id);
    } else if(i2d_string_stack_push(stack, pet->jname.string, pet->jname.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_pet(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    return  i2d_handler_range(script, node, local, i2d_handler_pet_cb) ||
            i2d_handler_expression(script, variables, node, local);
}

static int i2d_handler_pet_script(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long id;
    i2d_pet * pet;
    i2d_string pet_script;

    i2d_zero(pet_script);

    if(i2d_node_get_constant(node, &id)) {
        status = i2d_panic("failed to get pet id");
    } else if(i2d_pet_db_search_by_id(script->db->pet_db, id, &pet)) {
        status = i2d_panic("failed to get pet by id -- %ld", id);
    } else if(i2d_script_compile(script, &pet->pet_script, &pet_script, variables)) {
        status = i2d_panic("failed to compile script -- %s", pet->pet_script.string);
    } else {
        if(i2d_string_stack_push(local->stack, pet_script.string, pet_script.length))
            status = i2d_panic("failed to push string on stack");
        i2d_string_destroy(&pet_script);
    }

    return status;
}

static int i2d_handler_pet_loyal_script(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long id;
    i2d_pet * pet;
    i2d_string loyal_script;

    i2d_zero(loyal_script);

    if(i2d_node_get_constant(node, &id)) {
        status = i2d_panic("failed to get pet id");
    } else if(i2d_pet_db_search_by_id(script->db->pet_db, id, &pet)) {
        status = i2d_panic("failed to get pet by id -- %ld", id);
    } else if(i2d_script_compile(script, &pet->loyal_script, &loyal_script, variables)) {
        status = i2d_panic("failed to compile script -- %s", pet->loyal_script.string);
    } else {
        if(i2d_string_stack_push(local->stack, loyal_script.string, loyal_script.length))
            status = i2d_panic("failed to push string on stack");
        i2d_string_destroy(&loyal_script);
    }

    return status;
}

static int i2d_handler_produce(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long item_level;
    i2d_produce_list * produce_list;
    i2d_produce * produce;
    size_t i;
    i2d_item * item;
    i2d_skill * skill;
    size_t j;
    long item_id;
    long item_amount;
    i2d_item * material;

    if(i2d_node_get_constant(node, &item_level)) {
        status = i2d_panic("failed to get item level");
    } else if(i2d_produce_db_search_by_item_level(script->db->produce_db, item_level, &produce_list)) {
        status = i2d_panic("failed to get produce list by item level -- %ld", item_level);
    } else {
        for(i = 0; i < produce_list->size && !status; i++) {
            item = NULL;
            skill = NULL;
            produce = produce_list->list[i];
            if(i2d_item_db_search_by_id(script->db->item_db, produce->item_id, &item)) {
                status = i2d_panic("failed to get item by id -- %ld", produce->item_id);
            } else if(produce->skill_id && i2d_skill_db_search_by_id(script->db->skill_db, produce->skill_id, &skill)) {
                status = i2d_panic("failed to get skill by id -- %ld", produce->skill_id);
            } else {
                if( skill ?
                    i2d_buffer_printf(local->buffer, "[%s - Level %ld %s]\n", item->name.string, produce->skill_level, skill->name.string) :
                    i2d_buffer_printf(local->buffer, "[%s]\n", item->name.string) ) {
                    status = i2d_panic("failed to write buffer object");
                } else {
                    for(j = 0; j < produce->material_count && !status; j += 2) {
                        item_id = produce->materials[j];
                        item_amount = produce->materials[j + 1];
                        if(i2d_item_db_search_by_id(script->db->item_db, item_id, &material)) {
                            status = i2d_panic("failed to get item by id -- %ld", item_id);
                        } else if(i2d_buffer_printf(local->buffer, "x%ld %s\n", item_amount, material->name.string)) {
                            status = i2d_panic("failed to write buffer object");
                        }
                    }
                }
            }
        }
        if(!status && i2d_string_stack_push_buffer(local->stack, local->buffer))
            status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_sc_end(i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;
    long id;
    i2d_constant * constant;

    if(i2d_node_get_constant(node, &id)) {
        status = i2d_panic("failed to get status id");
    } else if(i2d_constant_get_by_sc_end(script->constant_db, id, &constant)) {
        status = i2d_panic("failed to get status by id -- %ld", id);
    } else if(i2d_string_stack_push(local->stack, constant->name.string, constant->name.length)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_custom(i2d_data * data, i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;

    long min;
    long max;

    i2d_range_get_range(&node->range, &min, &max);

    if(data->empty_description_on_zero && !min && !max) {
        if(i2d_string_stack_push(local->stack, "", 0))
            status = i2d_panic("failed to push string on stack");
    } else if(i2d_script_statement_evaluate(script, variables, &node, data, local->buffer)) {
        status = i2d_panic("failed to evaluate data object");
    } else if(i2d_string_stack_push_buffer(local->stack, local->buffer)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_prefix(i2d_data * data, i2d_script * script, i2d_rbt * variables, i2d_node * node, i2d_local * local) {
    int status = I2D_OK;

    long min;
    long max;

    i2d_range_get_range(&node->range, &min, &max);

    if(!min && !max && data->zero.string) {
        if(i2d_string_stack_push(local->stack, data->zero.string, data->zero.length))
            status = i2d_panic("failed to push string on stack");
    } else {
        if(min > 0) {
            if(i2d_string_stack_push(local->stack, data->positive.string, data->positive.length))
                status = i2d_panic("failed to push string on stack");
        } else {
            if(i2d_string_stack_push(local->stack, data->negative.string, data->negative.length))
                status = i2d_panic("failed to push string on stack");
        }
    }

    return status;
}

static int i2d_handler_bonus(i2d_script * script, i2d_rbt * variables, i2d_node ** nodes, i2d_local * local) {
    int status = I2D_OK;
    long bonus_type;
    i2d_string bonus_name;
    i2d_data * data;

    if(i2d_node_get_constant(nodes[0], &bonus_type)) {
        status = i2d_panic("failed to get bonus type");
    } else if(i2d_data_map_get(script->bonus, &bonus_type, &data)) {
        if(i2d_node_get_string(nodes[0], &bonus_name)) {
            status = i2d_panic("failed to get bonus by type -- %ld", bonus_type);
        } else {
            status = i2d_panic("failed to get bonus by type -- %ld (%s)", bonus_type, bonus_name.string);
        }
    } else if(i2d_script_statement_evaluate(script, variables, &nodes[1], data, local->buffer)) {
        status = i2d_panic("failed to handle bonus arguments");
    } else if(i2d_string_stack_push_buffer(local->stack, local->buffer)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_bonus2(i2d_script * script, i2d_rbt * variables, i2d_node ** nodes, i2d_local * local) {
    int status = I2D_OK;
    long bonus_type;
    i2d_string bonus_name;
    i2d_data * data;

    if(i2d_node_get_constant(nodes[0], &bonus_type)) {
        status = i2d_panic("failed to get bonus type");
    } else if(i2d_data_map_get(script->bonus2, &bonus_type, &data)) {
        if(i2d_node_get_string(nodes[0], &bonus_name)) {
            status = i2d_panic("failed to get bonus by type -- %ld", bonus_type);
        } else {
            status = i2d_panic("failed to get bonus by type -- %ld (%s)", bonus_type, bonus_name.string);
        }
    } else if(i2d_script_statement_evaluate(script, variables, &nodes[1], data, local->buffer)) {
        status = i2d_panic("failed to handle bonus arguments");
    } else if(i2d_string_stack_push_buffer(local->stack, local->buffer)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_bonus3(i2d_script * script, i2d_rbt * variables, i2d_node ** nodes, i2d_local * local) {
    int status = I2D_OK;
    long bonus_type;
    i2d_string bonus_name;
    i2d_data * data;

    if(i2d_node_get_constant(nodes[0], &bonus_type)) {
        status = i2d_panic("failed to get bonus type");
    } else if(i2d_data_map_get(script->bonus3, &bonus_type, &data)) {
        if(i2d_node_get_string(nodes[0], &bonus_name)) {
            status = i2d_panic("failed to get bonus by type -- %ld", bonus_type);
        } else {
            status = i2d_panic("failed to get bonus by type -- %ld (%s)", bonus_type, bonus_name.string);
        }
    } else if(i2d_script_statement_evaluate(script, variables, &nodes[1], data, local->buffer)) {
        status = i2d_panic("failed to handle bonus arguments");
    } else if(i2d_string_stack_push_buffer(local->stack, local->buffer)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_bonus4(i2d_script * script, i2d_rbt * variables, i2d_node ** nodes, i2d_local * local) {
    int status = I2D_OK;
    long bonus_type;
    i2d_string bonus_name;
    i2d_data * data;

    if(i2d_node_get_constant(nodes[0], &bonus_type)) {
        status = i2d_panic("failed to get bonus type");
    } else if(i2d_data_map_get(script->bonus4, &bonus_type, &data)) {
        if(i2d_node_get_string(nodes[0], &bonus_name)) {
            status = i2d_panic("failed to get bonus by type -- %ld", bonus_type);
        } else {
            status = i2d_panic("failed to get bonus by type -- %ld (%s)", bonus_type, bonus_name.string);
        }
    } else if(i2d_script_statement_evaluate(script, variables, &nodes[1], data, local->buffer)) {
        status = i2d_panic("failed to handle bonus arguments");
    } else if(i2d_string_stack_push_buffer(local->stack, local->buffer)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_bonus5(i2d_script * script, i2d_rbt * variables, i2d_node ** nodes, i2d_local * local) {
    int status = I2D_OK;
    long bonus_type;
    i2d_string bonus_name;
    i2d_data * data;

    if(i2d_node_get_constant(nodes[0], &bonus_type)) {
        status = i2d_panic("failed to get bonus type");
    } else if(i2d_data_map_get(script->bonus5, &bonus_type, &data)) {
        if(i2d_node_get_string(nodes[0], &bonus_name)) {
            status = i2d_panic("failed to get bonus by type -- %ld", bonus_type);
        } else {
            status = i2d_panic("failed to get bonus by type -- %ld (%s)", bonus_type, bonus_name.string);
        }
    } else if(i2d_script_statement_evaluate(script, variables, &nodes[1], data, local->buffer)) {
        status = i2d_panic("failed to handle bonus arguments");
    } else if(i2d_string_stack_push_buffer(local->stack, local->buffer)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_sc_start(i2d_script * script, i2d_rbt * variables, i2d_node ** nodes, i2d_local * local) {
    int status = I2D_OK;
    long effect_type;
    i2d_data * data;

    if(i2d_node_get_constant(nodes[0], &effect_type)) {
        status = i2d_panic("failed to get effect type");
    } else if(i2d_data_map_get(script->sc_start, &effect_type, &data)) {
        if(!nodes[0]->constant) {
            status = i2d_panic("failed to get effect by type -- %ld", effect_type);
        } else {
            status = i2d_panic("failed to get effect by type -- %ld (%s)", effect_type, nodes[0]->constant->macro.string);
        }
    } else if(i2d_script_statement_evaluate(script, variables, &nodes[0], data, local->buffer)) {
        status = i2d_panic("failed to handle bonus arguments");
    } else if(i2d_string_stack_push_buffer(local->stack, local->buffer)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_sc_start2(i2d_script * script, i2d_rbt * variables, i2d_node ** nodes, i2d_local * local) {
    int status = I2D_OK;
    long effect_type;
    i2d_data * data;

    if(i2d_node_get_constant(nodes[0], &effect_type)) {
        status = i2d_panic("failed to get effect type");
    } else if(i2d_data_map_get(script->sc_start2, &effect_type, &data)) {
        if(!nodes[0]->constant) {
            status = i2d_panic("failed to get effect by type -- %ld", effect_type);
        } else {
            status = i2d_panic("failed to get effect by type -- %ld (%s)", effect_type, nodes[0]->constant->macro.string);
        }
    } else if(i2d_script_statement_evaluate(script, variables, &nodes[0], data, local->buffer)) {
        status = i2d_panic("failed to handle bonus arguments");
    } else if(i2d_string_stack_push_buffer(local->stack, local->buffer)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}

static int i2d_handler_sc_start4(i2d_script * script, i2d_rbt * variables, i2d_node ** nodes, i2d_local * local) {
    int status = I2D_OK;
    long effect_type;
    i2d_data * data;

    if(i2d_node_get_constant(nodes[0], &effect_type)) {
        status = i2d_panic("failed to get effect type");
    } else if(i2d_data_map_get(script->sc_start4, &effect_type, &data)) {
        if(!nodes[0]->constant) {
            status = i2d_panic("failed to get effect by type -- %ld", effect_type);
        } else {
            status = i2d_panic("failed to get effect by type -- %ld (%s)", effect_type, nodes[0]->constant->macro.string);
        }
    } else if(i2d_script_statement_evaluate(script, variables, &nodes[0], data, local->buffer)) {
        status = i2d_panic("failed to handle bonus arguments");
    } else if(i2d_string_stack_push_buffer(local->stack, local->buffer)) {
        status = i2d_panic("failed to push string on stack");
    }

    return status;
}
