#include "i2d_util.h"
#include "i2d_range.h"
#include "i2d_logic.h"
#include "i2d_rbt.h"
#include "i2d_opt.h"
#include "i2d_item.h"

static void i2d_logic_test(void);
static void i2d_logic_or_test(i2d_logic *, i2d_logic *, i2d_logic *);
static void i2d_logic_and_test(i2d_logic *, i2d_logic *, i2d_logic *);
static void i2d_logic_not_test(i2d_logic *, i2d_logic *, i2d_logic *);

int main(int argc, char * argv[]) {
    i2d_logic_test();
    return 0;
}

static void i2d_logic_test(void) {
    i2d_string getrefine;
    i2d_string readparam;
    i2d_range getrefine_range;
    i2d_range readparam_range;
    i2d_logic * getrefine_logic = NULL;
    i2d_logic * readparam_logic = NULL;
    i2d_logic * and_logic = NULL;
    i2d_logic * or_logic = NULL;

    i2d_string_create(&getrefine, "getrefine", 9);
    i2d_string_create(&readparam, "readparam", 9);
    i2d_range_create(&getrefine_range);
    i2d_range_create(&readparam_range);
    i2d_range_add(&getrefine_range, 0, 15);
    i2d_range_add(&readparam_range, 1, 99);
    i2d_logic_init(&getrefine_logic, &getrefine, &getrefine_range);
    i2d_logic_init(&readparam_logic, &readparam, &readparam_range);
    i2d_logic_var(&and_logic, getrefine_logic, readparam_logic, and);
    i2d_logic_var(&or_logic, getrefine_logic, readparam_logic, or);
    i2d_logic_or_test(readparam_logic, and_logic, or_logic);
    i2d_logic_and_test(readparam_logic, and_logic, or_logic);
    i2d_logic_not_test(readparam_logic, and_logic, or_logic);
    i2d_logic_deit(&or_logic);
    i2d_logic_deit(&and_logic);
    i2d_logic_deit(&readparam_logic);
    i2d_logic_deit(&getrefine_logic);
    i2d_range_destroy(&readparam_range);
    i2d_range_destroy(&getrefine_range);
    i2d_string_destroy(&readparam);
    i2d_string_destroy(&getrefine);
}

void i2d_logic_or_test(i2d_logic * var_logic, i2d_logic * and_logic, i2d_logic * or_logic) {
    i2d_logic * or_var_var = NULL;
    i2d_logic * or_var_and = NULL;
    i2d_logic * or_var_or = NULL;
    i2d_logic * or_and_var = NULL;
    i2d_logic * or_and_and = NULL;
    i2d_logic * or_and_or = NULL;
    i2d_logic * or_or_var = NULL;
    i2d_logic * or_or_and = NULL;
    i2d_logic * or_or_or = NULL;
    i2d_logic_or(&or_var_var, var_logic, var_logic);
    i2d_logic_or(&or_var_and, var_logic, and_logic);
    i2d_logic_or(&or_var_or, var_logic, or_logic);
    i2d_logic_or(&or_and_var, and_logic, var_logic);
    i2d_logic_or(&or_and_and, and_logic, and_logic);
    i2d_logic_or(&or_and_or, and_logic, or_logic);
    i2d_logic_or(&or_or_var, or_logic, var_logic);
    i2d_logic_or(&or_or_and, or_logic, and_logic);
    i2d_logic_or(&or_or_or, or_logic, or_logic);
    fprintf(stderr, "or test results\n");
    i2d_logic_print(or_var_var, 0);
    i2d_logic_print(or_var_and, 0);
    i2d_logic_print(or_var_or, 0);
    i2d_logic_print(or_and_var, 0);
    i2d_logic_print(or_and_and, 0);
    i2d_logic_print(or_and_or, 0);
    i2d_logic_print(or_or_var, 0);
    i2d_logic_print(or_or_and, 0);
    i2d_logic_print(or_or_or, 0);
    i2d_logic_deit(&or_or_or);
    i2d_logic_deit(&or_or_and);
    i2d_logic_deit(&or_or_var);
    i2d_logic_deit(&or_and_or);
    i2d_logic_deit(&or_and_and);
    i2d_logic_deit(&or_and_var);
    i2d_logic_deit(&or_var_or);
    i2d_logic_deit(&or_var_and);
    i2d_logic_deit(&or_var_var);
}

static void i2d_logic_and_test(i2d_logic * var_logic, i2d_logic * and_logic, i2d_logic * or_logic) {
    i2d_logic * and_var_var = NULL;
    i2d_logic * and_var_and = NULL;
    i2d_logic * and_var_or = NULL;
    i2d_logic * and_and_var = NULL;
    i2d_logic * and_and_and = NULL;
    i2d_logic * and_and_or = NULL;
    i2d_logic * and_or_var = NULL;
    i2d_logic * and_or_and = NULL;
    i2d_logic * and_or_or = NULL;
    i2d_logic_and(&and_var_var, var_logic, var_logic);
    i2d_logic_and(&and_var_and, var_logic, and_logic);
    i2d_logic_and(&and_var_or, var_logic, or_logic);
    i2d_logic_and(&and_and_var, and_logic, var_logic);
    i2d_logic_and(&and_and_and, and_logic, and_logic);
    i2d_logic_and(&and_and_or, and_logic, or_logic);
    i2d_logic_and(&and_or_var, or_logic, var_logic);
    i2d_logic_and(&and_or_and, or_logic, and_logic);
    i2d_logic_and(&and_or_or, or_logic, or_logic);
    fprintf(stderr, "and test results\n");
    i2d_logic_print(and_var_var, 0);
    i2d_logic_print(and_var_and, 0);
    i2d_logic_print(and_var_or, 0);
    i2d_logic_print(and_and_var, 0);
    i2d_logic_print(and_and_and, 0);
    i2d_logic_print(and_and_or, 0);
    i2d_logic_print(and_or_var, 0);
    i2d_logic_print(and_or_and, 0);
    i2d_logic_print(and_or_or, 0);
    i2d_logic_deit(&and_or_or);
    i2d_logic_deit(&and_or_and);
    i2d_logic_deit(&and_or_var);
    i2d_logic_deit(&and_and_or);
    i2d_logic_deit(&and_and_and);
    i2d_logic_deit(&and_and_var);
    i2d_logic_deit(&and_var_or);
    i2d_logic_deit(&and_var_and);
    i2d_logic_deit(&and_var_var);
}

static void i2d_logic_not_test(i2d_logic * var_logic, i2d_logic * and_logic, i2d_logic * or_logic) {
    i2d_logic * not_var = NULL;
    i2d_logic * not_and = NULL;
    i2d_logic * not_or = NULL;
    i2d_logic_not(&not_var, var_logic);
    i2d_logic_not(&not_and, and_logic);
    i2d_logic_not(&not_or, or_logic);
    fprintf(stderr, "not test results\n");
    i2d_logic_print(not_var, 0);
    i2d_logic_print(not_and, 0);
    i2d_logic_print(not_or, 0);
    i2d_logic_deit(&not_or);
    i2d_logic_deit(&not_and);
    i2d_logic_deit(&not_var);
}
