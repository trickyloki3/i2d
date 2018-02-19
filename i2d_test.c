#include "i2d_range.h"
#include "i2d_logic.h"

int main(int argc, char * argv[]) {
    int status = I2D_OK;
    i2d_str * name = NULL;
    i2d_range_list * range = NULL;
    i2d_logic * logic = NULL;

    i2d_str_init(&name, "getrefine", 9);
    i2d_range_list_init(&range);
    i2d_range_list_add(range, 0, 15);
    i2d_logic_init(&logic, name, range);
    i2d_logic_deit(&logic);
    i2d_range_list_deit(&range);
    i2d_str_deit(&name);
    return status;
}
