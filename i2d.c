#include "i2d_util.h"
#include "i2d_opt.h"

int main(int argc, char * argv[]) {
    int status = I2D_OK;
    i2d_option * option = NULL;

    if(i2d_option_init(&option, argc, argv)) {
        status = i2d_panic("failed to create option object");
    } else {

        i2d_option_deit(&option);
    }

    return status;
}