#include "i2d_script.h"

static int i2d_item_compile_script(i2d_script *, i2d_item *);

int main(int argc, char * argv[]) {
    int status = I2D_OK;
    i2d_string path;
    i2d_config * config = NULL;
    i2d_script * script = NULL;
    i2d_item * item;

    if(argc < 2) {
        status = i2d_panic("%s <config.json>", argv[0]);
    } else if(i2d_string_vprintf(&path, argv[1])) {
        status = i2d_panic("failed to create string object");
    } else {
        if(i2d_config_init(&config, &path)) {
            status = i2d_panic("failed to create config object");
        } else {
            if(i2d_script_init(&script, config)) {
                status = i2d_panic("failed to create script object");
            } else {
                if(config->item_id) {
                    if(i2d_item_db_search_by_id(script->db->item_db, config->item_id, &item)) {
                        status = i2d_panic("failed to find item id -- %ld", config->item_id);
                    } else {
                        i2d_item_compile_script(script, item);
                    }
                } else {
                    item = script->db->item_db->list;
                    do {
                        i2d_item_compile_script(script, item);
                        item = item->next;
                    } while(item != script->db->item_db->list);
                }
                i2d_script_deit(&script);
            }
            i2d_config_deit(&config);
        }
        i2d_string_destroy(&path);
    }

    return status;
}

static int i2d_item_compile_script(i2d_script * script, i2d_item * item) {
    int status = I2D_OK;
    i2d_string normal_script;
    i2d_string onequip_script;
    i2d_string onunequip_script;
    i2d_string combo_script;

    i2d_zero(normal_script);
    i2d_zero(onequip_script);
    i2d_zero(onunequip_script);
    i2d_zero(combo_script);

    if(i2d_script_compile(script, &item->script, &normal_script, NULL)) {
        status = i2d_panic("failed to translate script for item %ld", item->id);
    } else {
        if(i2d_script_compile(script, &item->onequip_script, &onequip_script, NULL)) {
            status = i2d_panic("failed to translate onequip script for item %ld", item->id);
        } else {
            if(i2d_script_compile(script, &item->onunequip_script, &onunequip_script, NULL)) {
                status = i2d_panic("failed to translate onunequip script for item %ld", item->id);
            } else {
                if(i2d_script_compile_item_combo(script, item, &combo_script)) {
                    status = i2d_panic("failed to compile item combo script -- %ld", item->id);
                } else {
                    fprintf(stdout, "%ld - %s\n", item->id, item->name.string);
                    if(normal_script.length)
                        fprintf(stdout, "[script]\n%s", normal_script.string);
                    if(onequip_script.length)
                        fprintf(stdout, "[onequip]\n%s", onequip_script.string);
                    if(onunequip_script.length)
                        fprintf(stdout, "[onunequip]\n%s", onunequip_script.string);
                    if(combo_script.length)
                        fprintf(stdout, "[combo]\n%s", combo_script.string);
                    fprintf(stdout, "~\n");
                    i2d_string_destroy(&combo_script);
                }
                i2d_string_destroy(&onunequip_script);
            }
            i2d_string_destroy(&onequip_script);
        }
        i2d_string_destroy(&normal_script);
    }

    return status;
}
