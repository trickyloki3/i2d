#include "i2d_json.h"
#include "i2d_script.h"
#include "i2d_print.h"

int main(int argc, char * argv[]) {
    int status = I2D_OK;
    i2d_string path;
    i2d_config * config = NULL;
    i2d_json * json = NULL;
    i2d_script * script = NULL;
    i2d_item * item;
    i2d_item_script item_script;

    if(argc < 2) {
        status = i2d_panic("%s <config.json>", argv[0]);
    } else if(i2d_string_vprintf(&path, argv[1])) {
        status = i2d_panic("failed to create string object");
    } else {
        if(i2d_config_init(&config, &path)) {
            status = i2d_panic("failed to create config object");
        } else {
            if(i2d_json_init(&json, &config->data_path)) {
                status = i2d_panic("failed to create json object");
            } else {
                if(i2d_script_init(&script, config, json)) {
                    status = i2d_panic("failed to create script object");
                } else {
                    if(config->item_id) {
                        if(i2d_item_db_search_by_id(script->db->item_db, config->item_id, &item)) {
                            status = i2d_panic("failed to find item -- %ld", config->item_id);
                        } else {
                            if(i2d_item_script_create(&item_script, item, script)) {
                                status = i2d_panic("failed to get compile item -- %ld", item->id);
                            } else {
                                i2d_item_script_destroy(&item_script);
                            }
                        }
                    } else {
                        item = script->db->item_db->list;
                        do {
                            if(i2d_item_script_create(&item_script, item, script)) {
                                status = i2d_panic("failed to get compile item -- %ld", item->id);
                            } else {
                                i2d_item_script_destroy(&item_script);
                            }
                            item = item->next;
                        } while(item != script->db->item_db->list);
                    }
                    i2d_script_deit(&script);
                }
                i2d_json_deit(&json);
            }
            i2d_config_deit(&config);
        }
        i2d_string_destroy(&path);
    }

    return status;
}
