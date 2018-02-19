#include "i2d_range.h"
#include "i2d_logic.h"

int main(int argc, char * argv[]) {
    int status = I2D_OK;
    i2d_str * name = NULL;
    i2d_str * name2 = NULL;
    i2d_range_list * range = NULL;
    i2d_range_list * range2 = NULL;
    i2d_logic * logic = NULL;
    i2d_logic * logic2 = NULL;
    i2d_logic * logic3 = NULL;

    i2d_str_init(&name, "getrefine", 9);
    i2d_str_init(&name2, "readparam", 9);
    i2d_range_list_init(&range);
    i2d_range_list_add(range, 5, 15);
    i2d_range_list_init(&range2);
    i2d_range_list_add(range2, 0, 7);
    i2d_logic_init(&logic, name, range);
    i2d_logic_init(&logic2, name2, range2);
    i2d_logic_var(&logic3, logic, logic2, or);
    i2d_logic_print(logic3, 0);
    i2d_logic_deit(&logic3);
    i2d_logic_deit(&logic2);
    i2d_logic_deit(&logic);
    i2d_range_list_deit(&range2);
    i2d_range_list_deit(&range);
    i2d_str_deit(&name2);
    i2d_str_deit(&name);
    return status;
}
