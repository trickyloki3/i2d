#include "i2d_util.h"
#include "i2d_opt.h"
#include "i2d_db.h"
#include "i2d_rbt.h"
#include "i2d_script.h"
#include "i2d_json.h"

int main(int argc, char * argv[]) {
    int status = I2D_OK;
    i2d_option * option = NULL;
    i2d_script * script = NULL;
    i2d_item * item;

    if(i2d_option_init(&option, argc, argv)) {
        status = i2d_panic("failed to create option object");
    } else {
        if(i2d_script_init(&script, option)) {
            status = i2d_panic("failed to create script object");
        } else {
            if(option->item_id) {
                if(i2d_item_db_search_by_id(script->db->item_db, option->item_id, &item)) {
                    status = i2d_panic("failed to find item id -- %ld", option->item_id);
                } else {
#if i2d_debug
                    i2d_script_test(script, item);
#endif
                }
            } else {
                item = script->db->item_db->list;
                do {
#if i2d_debug
                    i2d_script_test(script, item);
#endif
                    item = item->next;
                } while(item != script->db->item_db->list);
            }
            i2d_script_deit(&script);
        }
        i2d_option_deit(&option);
    }

    return status;
}
