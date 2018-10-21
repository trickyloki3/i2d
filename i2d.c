#include "i2d_script.h"

static int i2d_item_compile_script(i2d_script *, i2d_item *);
static int i2d_item_compile_combo_script(i2d_script *, i2d_item *);

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

    i2d_zero(normal_script);
    i2d_zero(onequip_script);
    i2d_zero(onunequip_script);

    if(i2d_script_compile(script, &item->script, &normal_script)) {
        status = i2d_panic("failed to translate script for item %ld", item->id);
    } else {
        if(i2d_script_compile(script, &item->onequip_script, &onequip_script)) {
            status = i2d_panic("failed to translate onequip script for item %ld", item->id);
        } else {
            if(i2d_script_compile(script, &item->onunequip_script, &onunequip_script)) {
                status = i2d_panic("failed to translate onunequip script for item %ld", item->id);
            } else {
                fprintf(stdout, "%ld - %s\n", item->id, item->name.string);
                if(normal_script.length) 
                    fprintf(stdout, "[script]\n%s", normal_script.string);
                if(onequip_script.length) 
                    fprintf(stdout, "[onequip]\n%s", onequip_script.string);
                if(onunequip_script.length) 
                    fprintf(stdout, "[onunequip]\n%s", onunequip_script.string);
                if(i2d_item_compile_combo_script(script, item))
                    status = i2d_panic("failed to translate item combo script(s) for item %ld", item->id);
                fprintf(stdout, "~\n");
                i2d_string_destroy(&onunequip_script);
            }
            i2d_string_destroy(&onequip_script);
        }
        i2d_string_destroy(&normal_script);
    }

    return status;
}

static int i2d_item_compile_combo_script(i2d_script * script, i2d_item * item) {
    int status = I2D_OK;
    i2d_item_combo_list * item_combo_list;
    i2d_item_combo * item_combo;

    size_t i;
    i2d_string description;
    i2d_string item_list;

    if(!i2d_item_combo_db_search_by_id(script->db->item_combo_db, item->id, &item_combo_list)) {
        for(i = 0; i < item_combo_list->size && !status; i++) {
            item_combo = item_combo_list->list[i];
            if(i2d_script_compile(script, &item_combo->script, &description)) {
                status = i2d_panic("failed to translate item combo script for item %ld", item->id);
            } else {
                if(i2d_item_combo_get_string(item_combo, script->db->item_db, &item_list)) {
                    status = i2d_panic("failed to get item combo list");
                } else {
                    if(description.length) 
                        fprintf(stdout, "[%s Combo]\n%s", item_list.string, description.string);
                    i2d_string_destroy(&item_list);
                }
                i2d_string_destroy(&description);
            }
        }
    }

    return status;
}