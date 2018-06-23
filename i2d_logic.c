#include "i2d_logic.h"

int i2d_logic_init(i2d_logic ** result, i2d_string * name, i2d_range * range) {
    int status = I2D_OK;
    i2d_logic * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_string_create(&object->name, name->string, name->length)) {
                status = i2d_panic("failed to create string object");
            } else if(i2d_range_copy(&object->range, range)) {
                status = i2d_panic("failed to create range list object");
            } else {
                object->type = var;
            }

            if(status)
                i2d_logic_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_logic_deit(i2d_logic ** result) {
    i2d_logic * object;

    object = *result;
    i2d_deit(object->right, i2d_logic_deit);
    i2d_deit(object->left, i2d_logic_deit);
    i2d_range_destroy(&object->range);
    i2d_free(object->name.string);
    i2d_free(object);
    *result = NULL;
}

void i2d_logic_print(i2d_logic * logic, int level) {
    int i;

    if(logic) {
        for(i = 0; i < level * 4; i++)
            putc('-', stdout);
        switch(logic->type) {
            case var: fprintf(stdout, "[%s] ", logic->name.string); break;
            case and: fprintf(stdout, "[and] "); break;
            case or:  fprintf(stdout, "[or] ");  break;
        }

        i2d_range_print(&logic->range, NULL);
        i2d_logic_print(logic->left, level + 1);
        i2d_logic_print(logic->right, level + 1);
    }
}

int i2d_logic_link(i2d_logic ** result, i2d_logic * left, i2d_logic * right, int type) {
    int status = I2D_OK;
    i2d_logic * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            object->type = type;
            object->left = left;
            object->right = right;
            object->left->parent = object;
            object->right->parent = object;

            if(status)
                i2d_logic_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

int i2d_logic_var_copy(i2d_logic ** result, i2d_logic * logic) {
    int status = I2D_OK;

    if(var != logic->type) {
        status = i2d_panic("invalid paramater");
    } else {
        status = i2d_logic_init(result, &logic->name, &logic->range);
    }

    return status;
}

int i2d_logic_and_copy(i2d_logic ** result, i2d_logic * logic) {
    int status = I2D_OK;
    i2d_logic * left = NULL;
    i2d_logic * right = NULL;

    if(and != logic->type) {
        status = i2d_panic("invalid paramater");
    } else {
        switch(logic->left->type) {
            case var: status = i2d_logic_var_copy(&left, logic->left); break;
            case and: status = i2d_logic_and_copy(&left, logic->left); break;
            case or:  status = i2d_logic_or_copy(&left, logic->left);  break;
        }
        if(status) {
            status = i2d_panic("failed to copy left logic object");
        } else {
            switch(logic->right->type) {
                case var: status = i2d_logic_var_copy(&right, logic->right); break;
                case and: status = i2d_logic_and_copy(&right, logic->right); break;
                case or:  status = i2d_logic_or_copy(&right, logic->right);  break;
            }
            if(status) {
                status = i2d_panic("failed to copy right logic object");
            } else {
                if(i2d_logic_link(result, left, right, and))
                    status = i2d_panic("failed to link logic object");
                if(status)
                    i2d_logic_deit(&right);
            }
            if(status)
                i2d_logic_deit(&left);
        }
    }

    return status;
}

int i2d_logic_or_copy(i2d_logic ** result, i2d_logic * logic) {
    int status = I2D_OK;
    i2d_logic * left = NULL;
    i2d_logic * right = NULL;

    if(or != logic->type) {
        status = i2d_panic("invalid paramater");
    } else {
        switch(logic->left->type) {
            case var: status = i2d_logic_var_copy(&left, logic->left); break;
            case and: status = i2d_logic_and_copy(&left, logic->left); break;
            case or:  status = i2d_logic_or_copy(&left, logic->left);  break;
        }
        if(status) {
            status = i2d_panic("failed to copy left logic object");
        } else {
            switch(logic->right->type) {
                case var: status = i2d_logic_var_copy(&right, logic->right); break;
                case and: status = i2d_logic_and_copy(&right, logic->right); break;
                case or:  status = i2d_logic_or_copy(&right, logic->right);  break;
            }
            if(status) {
                status = i2d_panic("failed to copy right logic object");
            } else {
                if(i2d_logic_link(result, left, right, or))
                    status = i2d_panic("failed to link logic object");
                if(status)
                    i2d_logic_deit(&right);
            }
            if(status)
                i2d_logic_deit(&left);
        }
    }

    return status;
}

int i2d_logic_copy(i2d_logic ** result, i2d_logic * logic) {
    int status = I2D_OK;

    switch(logic->type) {
        case var: status = i2d_logic_var_copy(result, logic); break;
        case and: status = i2d_logic_and_copy(result, logic); break;
        case or:  status = i2d_logic_or_copy(result, logic);  break;
    }

    return status;
}

int i2d_logic_var(i2d_logic ** result, i2d_logic * left, i2d_logic * right, int type) {
    int status = I2D_OK;
    i2d_range range;
    i2d_logic * left_copy = NULL;
    i2d_logic * right_copy = NULL;

    if(var != left->type || var != right->type) {
        status = i2d_panic("invalid paramater");
    } else if(!strcmp(left->name.string, right->name.string)) {
        switch(type) {
            case and: status = i2d_range_and(&range, &left->range, &right->range); break;
            case or:  status = i2d_range_or(&range, &left->range, &right->range);  break;
        }
        if(status) {
            status = i2d_panic("failed to merge range list");
        } else {
            if(i2d_logic_init(result, &left->name, &range))
                status = i2d_panic("failed to create logic object");
            i2d_range_destroy(&range);
        }
    } else {
        if(i2d_logic_var_copy(&left_copy, left)) {
            status = i2d_panic("failed to create logic object");
        } else {
            if(i2d_logic_var_copy(&right_copy, right)) {
                status = i2d_panic("failed to create logic object");
            } else {
                if(i2d_logic_link(result, left_copy, right_copy, type))
                    status = i2d_panic("failed to link logic object");
                if(status)
                    i2d_logic_deit(&right_copy);
            }
            if(status)
                i2d_logic_deit(&left_copy);
        }
    }

    return status;
}

int i2d_logic_or_search(i2d_logic ** result, i2d_logic * logic, i2d_string * name) {
    if(var == logic->type && !strcmp(logic->name.string, name->string)) {
        *result = logic;
    } else {
        if(or == logic->type) {
            if(logic->left && !(*result))
                i2d_logic_or_search(result, logic->left, name);
            if(logic->right && !(*result))
                i2d_logic_or_search(result, logic->right, name);
        }
    }

    return *result ? I2D_OK : I2D_FAIL;
}

int i2d_logic_or_merge_recursive(i2d_logic ** result, i2d_logic * logic) {
    int status = I2D_OK;
    i2d_logic * sibling = NULL;
    i2d_logic * parent = NULL;
    i2d_logic * twin = NULL;
    i2d_range range;

    if(var == logic->type) {
        if(var == (*result)->type) {
            if(i2d_logic_var(&parent, logic, *result, or)) {
                status = i2d_panic("failed to merge logic object");
            } else {
                i2d_logic_deit(result);
                *result = parent;
            }
        } else if(or == (*result)->type && !i2d_logic_or_search(&twin, *result, &logic->name)) {
            if(i2d_range_or(&range, &twin->range, &logic->range)) {
                status = i2d_panic("failed to merge range list");
            } else {
                i2d_range_destroy(&twin->range);
                twin->range = range;
            }
        } else {
            if(i2d_logic_copy(&sibling, logic)) {
                status = i2d_panic("failed to copy logic object");
            } else {
                if(i2d_logic_link(&parent, *result, sibling, or)) {
                    status = i2d_panic("failed to link logic object");
                } else {
                    *result = parent;
                }
                if(status)
                    i2d_logic_deit(&sibling);
            }
        }
    } else if(and == logic->type) {
        if(i2d_logic_copy(&sibling, logic)) {
            status = i2d_panic("failed to copy logic object");
        } else {
            if(i2d_logic_link(&parent, *result, sibling, or)) {
                status = i2d_panic("failed to link logic object");
            } else {
                *result = parent;
            }
            if(status)
                i2d_logic_deit(&sibling);
        }
    } else if(logic->left && i2d_logic_or_merge_recursive(result, logic->left)) {
        status = i2d_panic("failed to merge logic object");
    } else if(logic->right && i2d_logic_or_merge_recursive(result, logic->right)) {
        status = i2d_panic("failed to merge logic object");
    }

    return status;
}

int i2d_logic_or_merge(i2d_logic ** result, i2d_logic * left, i2d_logic * right) {
    int status = I2D_OK;

    if(i2d_logic_copy(result, left)) {
        status = i2d_panic("failed to copy logic object");
    } else {
        if(i2d_logic_or_merge_recursive(result, right))
            status = i2d_panic("failed to merge logic object");
        if(status)
            i2d_logic_deit(result);
    }

    return status;
}

int i2d_logic_or(i2d_logic ** result, i2d_logic * left, i2d_logic * right) {
    int status = I2D_OK;

    switch(left->type) {
        case var:
            switch(right->type) {
                case var: status = i2d_logic_var(result, left, right, or); break;
                case and: status = i2d_logic_or_merge(result, left, right); break;
                case or:  status = i2d_logic_or_merge(result, left, right); break;
            }
            break;
        case and:
            switch(right->type) {
                case var: status = i2d_logic_or_merge(result, left, right); break;
                case and: status = i2d_logic_or_merge(result, left, right); break;
                case or:  status = i2d_logic_or_merge(result, left, right); break;
            }
            break;
        case or:
            switch(right->type) {
                case var: status = i2d_logic_or_merge(result, left, right); break;
                case and: status = i2d_logic_or_merge(result, left, right); break;
                case or:  status = i2d_logic_or_merge(result, left, right); break;
            }
            break;
    }

    return status;
}

int i2d_logic_and_search(i2d_logic ** result, i2d_logic * logic, i2d_string * name) {
    if(var == logic->type && !strcmp(logic->name.string, name->string)) {
        *result = logic;
    } else {
        if(and == logic->type) {
            if(logic->left && !(*result))
                i2d_logic_and_search(result, logic->left, name);
            if(logic->right && !(*result))
                i2d_logic_and_search(result, logic->right, name);
        }
    }

    return *result ? I2D_OK : I2D_FAIL;
}

int i2d_logic_and_merge_recursive(i2d_logic ** result, i2d_logic * logic) {
    int status = I2D_OK;
    i2d_logic * sibling = NULL;
    i2d_logic * parent = NULL;
    i2d_logic * twin = NULL;
    i2d_range range;

    if(or == logic->type) {
        status = i2d_panic("invalid paramater");
    } else if(var == logic->type) {
        if(var == (*result)->type) {
            if(i2d_logic_var(&parent, logic, *result, and)) {
                status = i2d_panic("failed to merge logic object");
            } else {
                i2d_logic_deit(result);
                *result = parent;
            }
        } else if(and == (*result)->type && !i2d_logic_and_search(&twin, *result, &logic->name)) {
            if(i2d_range_and(&range, &twin->range, &logic->range)) {
                status = i2d_panic("failed to merge range list");
            } else {
                i2d_range_destroy(&twin->range);
                twin->range = range;
            }
        } else {
            if(i2d_logic_copy(&sibling, logic)) {
                status = i2d_panic("failed to copy logic object");
            } else {
                if(i2d_logic_link(&parent, *result, sibling, and)) {
                    status = i2d_panic("failed to link logic object");
                } else {
                    *result = parent;
                }
                if(status)
                    i2d_logic_deit(&sibling);
            }
        }
    } else if(logic->left && i2d_logic_and_merge_recursive(result, logic->left)) {
        status = i2d_panic("failed to merge logic object");
    } else if(logic->right && i2d_logic_and_merge_recursive(result, logic->right)) {
        status = i2d_panic("failed to merge logic object");
    }

    return status;
}

int i2d_logic_and_merge(i2d_logic ** result, i2d_logic * left, i2d_logic * right) {
    int status = I2D_OK;

    if(i2d_logic_copy(result, left)) {
        status = i2d_panic("failed to copy logic object");
    } else {
        if(i2d_logic_and_merge_recursive(result, right))
            status = i2d_panic("failed to merge logic object");
        if(status)
            i2d_logic_deit(result);
    }

    return status;
}

int i2d_logic_and_or_merge(i2d_logic ** result, i2d_logic * left, i2d_logic * right) {
    int status = I2D_OK;

    i2d_logic * left_sibling = NULL;
    i2d_logic * right_sibling = NULL;

    if(or != left->type) {
        status = i2d_panic("invalid paramater");
    } else {
        switch(left->left->type) {
            case or: status = i2d_logic_and_or_merge(&left_sibling, left->left, right); break;
            default: status = i2d_logic_and(&left_sibling, left->left, right); break;
        }
        if(status) {
            status = i2d_panic("failed to merge logic object");
        } else {
            switch(left->right->type) {
                case or: status = i2d_logic_and_or_merge(&right_sibling, left->right, right); break;
                default: status = i2d_logic_and(&right_sibling, left->right, right); break;
            }
            if(status) {
                status = i2d_panic("failed to merge logic object");
            } else {
                if(i2d_logic_or(result, left_sibling, right_sibling))
                    status = i2d_panic("failed to link logic object");
                i2d_logic_deit(&right_sibling);
            }
            i2d_logic_deit(&left_sibling);
        }
    }

    return status;
}

int i2d_logic_and(i2d_logic ** result, i2d_logic * left, i2d_logic * right) {
    int status = I2D_OK;

    switch(left->type) {
        case var:
            switch(right->type) {
                case var: status = i2d_logic_var(result, left, right, and); break;
                case and: status = i2d_logic_and_merge(result, left, right); break;
                case or:  status = i2d_logic_and_or_merge(result, right, left); break;
            }
            break;
        case and:
            switch(right->type) {
                case var: status = i2d_logic_and_merge(result, left, right); break;
                case and: status = i2d_logic_and_merge(result, left, right); break;
                case or:  status = i2d_logic_and_or_merge(result, right, left); break;
            }
            break;
        case or:
            switch(right->type) {
                case var: status = i2d_logic_and_or_merge(result, left, right); break;
                case and: status = i2d_logic_and_or_merge(result, left, right); break;
                case or:  status = i2d_logic_and_or_merge(result, left, right); break;
            }
            break;
    }

    return status;
}

int i2d_logic_not(i2d_logic ** result, i2d_logic * logic) {
    int status = I2D_OK;
    i2d_range range;
    i2d_logic * left = NULL;
    i2d_logic * right = NULL;

    if(var == logic->type) {
        if(i2d_range_not(&range, &logic->range)) {
            status = i2d_panic("failed to not range list");
        } else {
            if(i2d_logic_init(result, &logic->name, &range))
                status = i2d_panic("failed to create logic object");
            i2d_range_destroy(&range);
        }
    } else {
        if(i2d_logic_not(&left, logic->left)) {
            status = i2d_panic("failed to not logic object");
        } else {
            if(i2d_logic_not(&right, logic->right)) {
                status = i2d_panic("failed to not logic object");
            } else {
                /*
                 * De Morgan's Law
                 */
                if( and == logic->type ?
                    i2d_logic_or(result, left, right) :
                    i2d_logic_and(result, left, right) )
                status = i2d_panic("failed to not logic object");
                i2d_logic_deit(&right);
            }
            i2d_logic_deit(&left);
        }
    }

    return status;
}

int i2d_logic_search_recursive(i2d_logic * logic, const char * name, i2d_range * result) {
    int status = I2D_OK;
    i2d_range range;
    i2d_zero(range);

    if( (logic->left  && i2d_logic_search_recursive(logic->left,  name, result)) ||
        (logic->right && i2d_logic_search_recursive(logic->right, name, result)) ) {
        status = i2d_panic("failed to search logic object");
    } else if(logic->type == var && !strcmp(name, logic->name.string)) {
        if(!result->list) {
            if(i2d_range_copy(result, &logic->range))
                status = i2d_panic("failed to copy range object");
        } else {
            if(i2d_range_or(&range, &logic->range, result)) {
                status = i2d_panic("failed to or range object");
            } else {
                i2d_range_destroy(result);
                *result = range;
            }
        }
    }

    return status;
}

int i2d_logic_search(i2d_logic * logic, const char * name, i2d_range * result) {
    int status = I2D_OK;
    i2d_range range;
    i2d_range limit;

    i2d_zero(range);
    i2d_zero(limit);

    if(i2d_logic_search_recursive(logic, name, &range)) {
        status = i2d_panic("failed to search logic object");
    } else if(range.list) {
        if(i2d_range_and(&limit, &range, result)) {
            status = i2d_panic("failed to and range object");
        } else {
            i2d_range_destroy(result);
            *result = limit;
        }
    }

    i2d_range_destroy(&range);
    return status;
}
