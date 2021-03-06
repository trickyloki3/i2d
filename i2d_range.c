#include "i2d_range.h"

static int i2d_range_solution_space_size_cb(i2d_range_node *, void *);

int i2d_range_node_init(i2d_range_node ** result, long min, long max) {
    int status = I2D_OK;
    i2d_range_node * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            object->min = min;
            object->max = max;
            object->next = object;
            object->prev = object;

            if(status)
                i2d_range_node_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_range_node_deit(i2d_range_node ** result) {
    i2d_range_node * object;

    object = *result;
    i2d_free(object);
    *result = NULL;
}

void i2d_range_node_append(i2d_range_node * x, i2d_range_node * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_range_node_remove(i2d_range_node * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

int i2d_range_create(i2d_range * result) {
    result->list = NULL;
    return I2D_OK;
}

int i2d_range_create_add(i2d_range * result, long min, long max) {
    return i2d_range_create(result) || i2d_range_add(result, min, max);
}

void i2d_range_destroy(i2d_range * result) {
    i2d_range_node * range;

    if(result->list) {
        while(result->list != result->list->next) {
            range = result->list->next;
            i2d_range_node_remove(range);
            i2d_range_node_deit(&range);
        }
        i2d_range_node_deit(&result->list);
    }
}

void i2d_range_print(i2d_range * list, const char * tag) {
    i2d_range_node * walk;

    if(list->list) {
        walk = list->list;
        if(tag)
            fprintf(stdout, "(%s) domain(%ld,%ld)", tag, list->min, list->max);
        do {
            fprintf(stdout, "[%ld,%ld]", walk->min, walk->max);
            walk = walk->next;
        } while(walk != list->list);
    }
    fprintf(stdout, "\n");
}

int i2d_range_add(i2d_range * list, long x, long y) {
    int status = I2D_OK;
    long min;
    long max;
    i2d_range_node * walk;
    i2d_range_node * range = NULL;

    min = min(x, y);
    max = max(x, y);

    if(!list->list) {
        if(i2d_range_node_init(&list->list, min, max)) {
            status = i2d_panic("failed to create range object");
        } else {
            list->min = min;
            list->max = max;
        }
    } else {
        walk = list->list;
        while(walk->max < min - 1 && walk->next != list->list)
            walk = walk->next;

        if(max < walk->min - 1) {
            if(i2d_range_node_init(&range, min, max)) {
                status = i2d_panic("failed to create range object");
            } else {
                i2d_range_node_append(range, walk);
                if(walk == list->list)
                    list->list = range;
            }
        } else if(walk->max < min - 1) {
            if(i2d_range_node_init(&range, min, max)) {
                status = i2d_panic("failed to create range object");
            } else {
                i2d_range_node_append(walk, range);
            }
        } else {
            walk->min = min(walk->min, min);
            walk->max = max(walk->max, max);
            while(walk != walk->next && walk->max >= walk->next->min - 1) {
                range = walk->next;
                walk->max = max(walk->max, range->max);
                i2d_range_node_remove(range);
                i2d_range_node_deit(&range);
            }
        }

        list->min = min(min, list->min);
        list->max = max(max, list->max);
    }

    return status;
}

int i2d_range_copy(i2d_range * result, i2d_range * list) {
    int status = I2D_OK;
    i2d_range_node * walk;

    if(i2d_range_create(result)) {
        status = i2d_panic("failed to create range object");
    } else {
        if(list->list) {
            walk = list->list->next;
            if(i2d_range_add(result, list->list->min, list->list->max))
                status = i2d_panic("failed to add range object");
            while(walk != list->list && !status) {
                if(i2d_range_add(result, walk->min, walk->max))
                    status = i2d_panic("failed to add range object");
                walk = walk->next;
            }
        }

        if(status) {
            i2d_range_destroy(result);
        } else {
            result->min = list->min;
            result->max = list->max;
        }
    }

    return status;
}

int i2d_range_negate(i2d_range * result, i2d_range * list) {
    int status = I2D_OK;
    i2d_range_node * walk;

    if(i2d_range_create(result)) {
        status = i2d_panic("failed to create range object");
    } else {
        if(list->list) {
            walk = list->list->next;
            if(i2d_range_add(result, -1 * list->list->min, -1 * list->list->max))
                status = i2d_panic("failed to add range object");
            while(walk != list->list && !status) {
                if(i2d_range_add(result, -1 * walk->min, -1 * walk->max))
                    status = i2d_panic("failed to add range object");
                walk = walk->next;
            }
        }

        if(status) {
            i2d_range_destroy(result);
        } else {
            result->min = list->min;
            result->max = list->max;
        }
    }

    return status;
}

int i2d_range_bitnot(i2d_range * result, i2d_range * list) {
    int status = I2D_OK;
    i2d_range_node * walk;

    if(i2d_range_create(result)) {
        status = i2d_panic("failed to create range object");
    } else {
        if(list->list) {
            walk = list->list->next;
            if(i2d_range_add(result, ~list->list->min, ~list->list->max))
                status = i2d_panic("failed to add range object");
            while(walk != list->list && !status) {
                if(i2d_range_add(result, ~walk->min, ~walk->max))
                    status = i2d_panic("failed to add range object");
                walk = walk->next;
            }
        }

        if(status) {
            i2d_range_destroy(result);
        } else {
            result->min = list->min;
            result->max = list->max;
        }
    }

    return status;
}

int i2d_range_merge(i2d_range * list, i2d_range_node * left_root, i2d_range_node * right_root, i2d_range_merge_cb merge_cb) {
    int status = I2D_OK;
    i2d_range_node * left;
    i2d_range_node * right;
    i2d_range_node * next = NULL;
    i2d_range_node * last = NULL;

    left = left_root;
    right = right_root;
    while(left && right && !status) {
        if(left->min < right->min) {
            next = left;
            left = left->next;
            if(left == left_root)
                left = NULL;
        } else {
            next = right;
            right = right->next;
            if(right == right_root)
                right = NULL;
        }

        if(merge_cb(list, &last, next))
            status = i2d_panic("failed to merge range");
    }

    while(left && !status) {
        next = left;
        left = left->next;
        if(left == left_root)
            left = NULL;

        if(merge_cb(list, &last, next))
            status = i2d_panic("failed to merge range");
    }

    while(right && !status) {
        next = right;
        right = right->next;
        if(right == right_root)
            right = NULL;

        if(merge_cb(list, &last, next))
            status = i2d_panic("failed to merge range");
    }

    return status;
}

int i2d_range_merge_or(i2d_range * list, i2d_range_node ** last, i2d_range_node * next) {
    int status = I2D_OK;

    if(!(*last)) {
        if(i2d_range_add(list, next->min, next->max)) {
            status = i2d_panic("failed to add range object");
        } else {
            *last = list->list->prev;
        }
    } else {
        if(next->min - 1 > (*last)->max) {
            if(i2d_range_add(list, next->min, next->max)) {
                status = i2d_panic("failed to add range object");
            } else {
                *last = list->list->prev;
            }
        } else {
            (*last)->min = min((*last)->min, next->min);
            (*last)->max = max((*last)->max, next->max);
        }
    }

    return status;
}

int i2d_range_merge_and(i2d_range * list, i2d_range_node ** last, i2d_range_node * next) {
    int status = I2D_OK;

    if(!(*last)) {
        *last = next;
    } else {
        if((*last)->min <= next->max && (*last)->max >= next->min) {
            if(i2d_range_add(list, max((*last)->min, next->min), min((*last)->max, next->max)))
                status = i2d_panic("failed to add range object");
        } else if(next->min <= (*last)->max && next->max >= (*last)->min) {
            if(i2d_range_add(list, max((*last)->min, next->min), min((*last)->max, next->max)))
                status = i2d_panic("failed to add range object");
        }

        if((*last)->max < next->max)
            *last = next;
    }

    return status;
}

int i2d_range_or(i2d_range * result, i2d_range * left, i2d_range * right) {
    int status = I2D_OK;

    if(i2d_range_create(result)) {
        status = i2d_panic("failed to create range object");
    } else {
        if(i2d_range_merge(result, left->list, right->list, i2d_range_merge_or))
            status = i2d_panic("failed to merge range");

        if(status) {
            i2d_range_destroy(result);
        } else {
            result->min = min(left->min, right->min);
            result->max = max(left->max, right->max);
        }
    }

    return status;
}

int i2d_range_and(i2d_range * result, i2d_range * left, i2d_range * right) {
    int status = I2D_OK;

    if(i2d_range_create(result)) {
        status = i2d_panic("failed to create range object");
    } else {
        if(i2d_range_merge(result, left->list, right->list, i2d_range_merge_and))
            status = i2d_panic("failed to merge range");

        if(status) {
            i2d_range_destroy(result);
        } else {
            result->min = min(left->min, right->min);
            result->max = max(left->max, right->max);
        }
    }

    return status;
}

int i2d_range_not(i2d_range * result, i2d_range * list) {
    int status = I2D_OK;
    i2d_range_node * walk = NULL;

    if(i2d_range_create(result)) {
        status = i2d_panic("failed to create range object");
    } else {
        if(list->list) {
            if( (list->min < list->list->min && i2d_range_add(result, list->min, list->list->min - 1)) ||
                (list->max > list->list->prev->max && i2d_range_add(result, list->list->prev->max + 1, list->max)) ) {
                status = i2d_panic("failed to add range object");
            } else if(list->list != list->list->next) {
                walk = list->list;
                do {
                    if(i2d_range_add(result, walk->max + 1, walk->next->min - 1))
                        status = i2d_panic("failed to add range object");
                    walk = walk->next;
                } while(walk != list->list->prev && !status);
            }
        }

        if(!result->list && !status)
            if(i2d_range_add(result, 0, 0))
                status = i2d_panic("failed to add range object");

        if(status) {
            i2d_range_destroy(result);
        } else {
            result->min = list->min;
            result->max = list->max;
        }
    }

    return status;
}

void i2d_range_get_range(i2d_range * list, long * min, long * max) {
    if(!list->list) {
        *min = 0;
        *max = 0;
    } else {
        *min = list->list->min;
        *max = list->list->prev->max;
    }
}

void i2d_range_get_range_absolute(i2d_range * list, long * result_min, long * result_max) {
    long min;
    long max;

    i2d_range_get_range(list, &min, &max);

    if(min < 0)
        min *= -1;
    if(max < 0)
        max *= -1;

    *result_min = min(min, max);
    *result_max = max(min, max);
}

int i2d_range_compute(i2d_range * result, i2d_range * left, i2d_range * right, int operator) {
    int status = I2D_OK;
    i2d_range object;
    i2d_range intermediate;
    i2d_range_node * walk;

    long left_min;
    long left_max;
    long right_min;
    long right_max;

    i2d_range_get_range(left, &left_min, &left_max);
    i2d_range_get_range(right, &right_min, &right_max);

    switch(operator) {
        case '|' + '|':
            if(i2d_range_or(result, left, right))
                status = i2d_panic("failed to or range operand");
            break;
        case '&' + '&':
            if(i2d_range_and(result, left, right))
                status = i2d_panic("failed to and range operand");
            break;
        case '=' + '=':
            if(i2d_range_and(result, left, right))
                status = i2d_panic("failed to and range operand");
            break;
        case '!' + '=':
            if(i2d_range_and(&intermediate, left, right)) {
                status = i2d_panic("failed to and range operand");
            } else {
                if(i2d_range_not(&object, &intermediate)) {
                    status = i2d_panic("failed to invert range operand");
                } else {
                    if(i2d_range_and(result, &object, left))
                        status = i2d_panic("failed to and range operand");
                    i2d_range_destroy(&object);
                }
                i2d_range_destroy(&intermediate);
            }
            break;
        case '<':
            right_max--;
        case '<' + '=':
            if(i2d_range_create(&object)) {
                status = i2d_panic("failed to create range object");
            } else {
                if( right_max < left_min ?
                        i2d_range_add(&object, 0, 0) :
                        i2d_range_add(&object, left_min, right_max) ) {
                    status = i2d_panic("failed to add range object");
                } else if(i2d_range_and(result, left, &object)) {
                    status = i2d_panic("failed to and range operand");
                }
                i2d_range_destroy(&object);
            }
            break;
        case '>':
            right_max++;
        case '>' + '=':
            if(i2d_range_create(&object)) {
                status = i2d_panic("failed to create range object");
            } else {
                if( left_max < right_max ?
                        i2d_range_add(&object, 0, 0) :
                        i2d_range_add(&object, right_max, left_max) ) {
                    status = i2d_panic("failed to add range object");
                } else if(i2d_range_and(result, left, &object)) {
                    status = i2d_panic("failed to and range operand");
                }
                i2d_range_destroy(&object);
            }
            break;
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '>' + '>' + 'b':
        case '<' + '<' + 'b':
        case '&':
        case '|':
        case '^' + 'b':
            if(i2d_range_create(result)) {
                status = i2d_panic("failed to create range object");
            } else {
                if(left->list) {
                    walk = left->list;
                    do {
                        switch(operator) {
                            case '+': status = i2d_range_add(result, walk->min + right_min, walk->max + right_max); break;
                            case '-': status = i2d_range_add(result, walk->min - right_min, walk->max - right_max); break;
                            case '*': status = i2d_range_add(result, walk->min * right_min, walk->max * right_max); break;
                            case '/': status = i2d_range_add(result, right_min ? walk->min / right_min : 0, right_max ? walk->max / right_max : 0); break;
                            case '%': status = i2d_range_add(result, walk->min % right_min, walk->max % right_max); break;
                            case '>' + '>' + 'b': status = i2d_range_add(result, walk->min >> right_min, walk->max >> right_max); break;
                            case '<' + '<' + 'b': status = i2d_range_add(result, walk->min << right_min, walk->max << right_max); break;
                            case '&': status = i2d_range_add(result, walk->min & right_min, walk->max & right_max); break;
                            case '|': status = i2d_range_add(result, walk->min | right_min, walk->max | right_max); break;
                            case '^' + 'b': status = i2d_range_add(result, walk->min ^ right_min, walk->max ^ right_max); break;
                        }
                    } while(walk != left->list && !status);
                }

                if(status)
                    i2d_range_destroy(result);
            }
            break;
    }

    return status;
}

int i2d_range_iterate_by_number(i2d_range * list, i2d_range_iterate_by_number_cb cb, void * data) {
    int status = I2D_OK;
    i2d_range_node * walk;
    long i;

    if(list->list) {
        walk = list->list;
        do {
            for(i = walk->min; i <= walk->max && !status; i++)
                status = cb(i, data);
            walk = walk->next;
        } while(walk != list->list && !status);
    }

    return status;
}


int i2d_range_iterate_by_range(i2d_range * list, i2d_range_iterate_by_range_cb cb, void * data) {
    int status = I2D_OK;
    i2d_range_node * walk;

    if(list->list) {
        walk = list->list;
        do {
            status = cb(walk, data);
            walk = walk->next;
        } while(walk != list->list && !status);
    }

    return status;
}

static int i2d_range_solution_space_size_cb(i2d_range_node * range, void * data) {
    long * size = data;

    *size += range->max - range->min + 1;

    return I2D_OK;
}

int i2d_range_solution_space_size(i2d_range * list, long * result) {
    int status = I2D_OK;
    int size = 0;

    if(i2d_range_iterate_by_range(list, i2d_range_solution_space_size_cb, &size)) {
        status = i2d_panic("failed to iterate range object");
    } else {
        *result = size;
    }

    return status;
}
