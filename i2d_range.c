#include "i2d_range.h"

#define min(x, y) ((x < y) ? x : y)
#define max(x, y) ((x < y) ? y : x)

int i2d_range_init(i2d_range ** result, long min, long max) {
    int status = I2D_OK;
    i2d_range * object;

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
                i2d_range_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_range_deit(i2d_range ** result) {
    i2d_range * object;

    object = *result;
    i2d_free(object);
    *result = NULL;
}

void i2d_range_append(i2d_range * x, i2d_range * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

void i2d_range_remove(i2d_range * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

int i2d_range_list_init(i2d_range_list ** result, long x, long y) {
    int status = I2D_OK;
    i2d_range_list * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            if(i2d_range_init(&object->list, min(x, y), max(x, y))) {
                status = i2d_panic("failed to create range object");
            }

            if(status)
                i2d_range_list_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_range_list_deit(i2d_range_list ** result) {
    i2d_range_list * object;
    i2d_range * range;

    object = *result;
    if(object->list) {
        while(object->list != object->list->next) {
            range = object->list->next;
            i2d_range_remove(range);
            i2d_range_deit(&range);
        }
        i2d_range_deit(&object->list);
    }
    i2d_free(object);
    *result = NULL;
}

void i2d_range_list_print(i2d_range_list * list) {
    i2d_range * walk;

    walk = list->list;
    do {
        fprintf(stdout, "[%ld,%ld]", walk->min, walk->max);
        walk = walk->next;
    } while(walk != list->list);
    fprintf(stdout, "\n");
}

int i2d_range_list_add(i2d_range_list * list, long x, long y) {
    int status = I2D_OK;
    long min;
    long max;
    i2d_range * walk;
    i2d_range * range = NULL;

    min = min(x, y);
    max = max(x, y);
    walk = list->list;
    while(walk->max < min - 1 && walk->next != list->list)
        walk = walk->next;

    if(max < walk->min - 1) {
        if(i2d_range_init(&range, min, max)) {
            status = i2d_panic("failed to create range object");
        } else {
            i2d_range_append(range, walk);
            if(walk == list->list)
                list->list = range;
        }
    } else if(walk->max < min - 1) {
        if(i2d_range_init(&range, min, max)) {
            status = i2d_panic("failed to create range object");
        } else {
            i2d_range_append(walk, range);
        }
    } else {
        walk->min = min(walk->min, min);
        walk->max = max(walk->max, max);
    }

    return status;
}
