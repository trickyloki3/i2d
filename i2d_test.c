#include "i2d_range.h"

int main(int argc, char * argv[]) {
    int status = I2D_OK;
    i2d_range_list * range = NULL;

    i2d_range_list_init(&range, 1, 3);
    i2d_range_list_add(range, 6, 9);
    i2d_range_list_add(range, 13, 15);
    i2d_range_list_print(range);
    i2d_range_list_deit(&range);

    return status;
}
