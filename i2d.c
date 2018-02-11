#include "i2d_util.h"
#include "i2d_opt.h"
#include "i2d_item.h"
#include "i2d_rbt.h"
#include "i2d_script.h"

int main(int argc, char * argv[]) {
    int status = I2D_OK;

#if i2d_debug
    status = i2d_lexer_test();
#endif

    return status;
}
