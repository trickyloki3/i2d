#include "i2d_util.h"
#include "i2d_opt.h"
#include "i2d_item.h"
#include "i2d_rbt.h"

int main(int argc, char * argv[]) {
    int status = I2D_OK;
    i2d_option * option = NULL;
    i2d_item_db * item_db = NULL;

    if(i2d_option_init(&option, argc, argv)) {
        status = i2d_panic("failed to create option object");
    } else {
        if(i2d_item_db_init(&item_db, option->item_db_path)) {
            status = i2d_panic("failed to create item database object");
        } else {

            i2d_item_db_deit(&item_db);
        }
        i2d_option_deit(&option);
    }

    return status;
}
