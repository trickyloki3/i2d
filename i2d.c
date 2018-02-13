#include "i2d_util.h"
#include "i2d_opt.h"
#include "i2d_item.h"
#include "i2d_rbt.h"
#include "i2d_script.h"
#include "i2d_json.h"

int main(int argc, char * argv[]) {
    int status = I2D_OK;
    i2d_option * option = NULL;
    i2d_json * json = NULL;
    i2d_item_db * item_db = NULL;
    i2d_lexer * lexer = NULL;
    i2d_item * item = NULL;

    if(i2d_option_init(&option, argc, argv)) {
        status = i2d_panic("failed to create option object");
    } else {
        if(i2d_json_init(&json, option->json_path)) {
            status = i2d_panic("failed to create json object");
        } else {
            if(i2d_item_db_init(&item_db, option->item_db_path)) {
                status = i2d_panic("failed to create item database object");
            } else {
                if(i2d_lexer_init(&lexer)) {
                    status = i2d_panic("failed to create lexer object");
                } else {
                    item = item_db->list->next;
                    while(item != item_db->list) {
                        if(!i2d_lexer_tokenize(lexer, item->script))
                            i2d_lexer_print(lexer);
                        if(!i2d_lexer_tokenize(lexer, item->onequip_script))
                            i2d_lexer_print(lexer);
                        if(!i2d_lexer_tokenize(lexer, item->onunequip_script))
                            i2d_lexer_print(lexer);
                        item = item->next;
                    }
                    i2d_lexer_deit(&lexer);
                }
                i2d_item_db_deit(&item_db);
            }
            i2d_json_deit(&json);
        }
        i2d_option_deit(&option);
    }

    return status;
}
