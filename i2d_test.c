#include "i2d_range.h"

int main(int argc, char * argv[]) {
    int status = I2D_OK;
    i2d_range_list * range1 = NULL;
    i2d_range_list * range2 = NULL;

    i2d_range_list_init(&range1, 1, 3);
    i2d_range_list_add(range1, 6, 9);
    i2d_range_list_add(range1, 13, 15);
    i2d_range_list_copy(&range2, range1);
    i2d_range_list_print(range1);
    i2d_range_list_print(range2);
    i2d_range_list_deit(&range2);
    i2d_range_list_deit(&range1);

    return status;
}
