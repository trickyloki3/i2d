#include "i2d_rbt.h"

struct i2d_rbt_node {
    void * key;                 /* key */
    void * val;                 /* value */
    int c;                      /* color */
    struct i2d_rbt_node * l;    /* left */
    struct i2d_rbt_node * r;    /* right */
    struct i2d_rbt_node * p;    /* parent */
};

typedef struct i2d_rbt_node i2d_rbt_node;

struct i2d_rbt {
    i2d_rbt_node * root;
    i2d_rbt_cmp compare;
};

enum { black, red };
#define is_nil(x)           ((x) == NULL)
#define is_root(x)          ((x)->p == NULL)
#define is_lchild(x)        ((x)->p->l == (x))
#define is_rchild(x)        ((x)->p->r == (x))
#define is_black(x)         ((x) == NULL || (x)->c == black)
#define is_red(x)           ((x) != NULL && (x)->c == red)

static void right_rotate(i2d_rbt *, i2d_rbt_node *);
static void left_rotate(i2d_rbt *, i2d_rbt_node *);
static void change_parent(i2d_rbt *, i2d_rbt_node *, i2d_rbt_node *);
static int i2d_rbt_node_init(i2d_rbt_node **, void *, void *);
static void i2d_rbt_node_deit(i2d_rbt_node **);
static int i2d_rbt_node_insert(i2d_rbt *, i2d_rbt_node *);
static int i2d_rbt_node_delete(i2d_rbt *, i2d_rbt_node *);
static int i2d_rbt_node_search(i2d_rbt *, i2d_rbt_node **, void *);

/* right rotation on x shifts the tree's height
 * from the left sub tree to the right sub tree
 * by one
 *
 *     p            p
 *     |            |
 *     x            y
 *    / \          / \
 *   y   c   =>   a   x
 *  / \              / \
 * a   b            b   c
 *
 * the left-to-right ordering is preserved
 * y < b < x    ; b remains right of y and left of x
 * a < y < x    ; a remains left  of y and x
 * y < x < c    ; c remains right of x and y
 */
static void right_rotate(i2d_rbt * tree, i2d_rbt_node * x) {
    i2d_rbt_node * y = x->l;

    /* link x and b */
    x->l = y->r;
    if(x->l)
        x->l->p = x;

    /* link y and p */
    y->p = x->p;
    if(is_root(x))
        tree->root = y;
    else if(is_lchild(x))
        y->p->l = y;
    else
        y->p->r = y;

    /* link y and x */
    x->p = y;
    y->r = x;
}

/* left rotation on  x shifts the tree's height
 * from the right sub tree to the left sub tree
 * by one
 *
 *   p               p
 *   |               |
 *   x               y
 *  / \             / \
 * a   y     =>    x   c
 *    / \         / \
 *   b   c       a   b
 *
 * the left-to-right ordering is preserved
 * x < b < y    ; b remains left  of y and right of x
 * a < x < y    ; a remains left  of x and y
 * x < y < c    ; c remains right of y and x
 */
static void left_rotate(i2d_rbt * tree, i2d_rbt_node * x) {
    i2d_rbt_node * y = x->r;

    /* link x and b */
    x->r = y->l;
    if(x->r)
        x->r->p = x;

    /* link y and p */
    y->p = x->p;
    if(is_root(x))
        tree->root = y;
    else if(is_lchild(x))
        y->p->l = y;
    else
        y->p->r = y;

    /* link y and x */
    x->p = y;
    y->l = x;
}

/* change the parent of y to
 * p to stage y to replace x
 *     p          p
 *     |          |
 *     x   =>   x |
 *    / \      / \|
 *   l   y    l   y
 */
static void change_parent(i2d_rbt * tree, i2d_rbt_node * x, i2d_rbt_node * y) {
    if(is_nil(x->p))
        tree->root = y;
    else if(is_lchild(x))
        x->p->l = y;
    else
        x->p->r = y;

    if(y)
        y->p = x->p;
}

int i2d_rbt_cmp_long(void * left, void * right) {
    return *((long * ) left) <  *((long * ) right) ? -1 :
           *((long * ) left) == *((long * ) right) ?  0 : 1;
}

int i2d_rbt_cmp_str(void * left, void * right) {
    return strcmp(((i2d_str *) left)->string, ((i2d_str *) right)->string);
}

int i2d_rbt_init(i2d_rbt ** result, i2d_rbt_cmp compare) {
    int status = I2D_OK;
    i2d_rbt * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            object->root = NULL;
            object->compare = compare;

            if(status)
                i2d_rbt_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

void i2d_rbt_deit(i2d_rbt ** result) {
    i2d_rbt * object;


    object = *result;
    if(object)
        i2d_rbt_clear(object);
    i2d_free(object);
    *result = NULL;
}

void i2d_rbt_clear(i2d_rbt * tree) {
    i2d_rbt_node * node;

    while(tree->root) {
        node = tree->root;
        i2d_rbt_node_delete(tree, node);
        i2d_rbt_node_deit(&node);
    }
}

int i2d_rbt_insert(i2d_rbt * tree, void * key, void * value) {
    int status = I2D_OK;
    i2d_rbt_node * node = NULL;

    if(i2d_rbt_node_init(&node, key, value)) {
        status = i2d_panic("failed to create node object");
    } else {
        if(i2d_rbt_node_insert(tree, node))
            status = i2d_panic("failed to insert node object");
        if(status)
            i2d_rbt_node_deit(&node);
    }

    return status;
}

int i2d_rbt_delete(i2d_rbt * tree, void * key) {
    int status = I2D_OK;
    i2d_rbt_node * node = NULL;

    if(i2d_rbt_node_search(tree, &node, key)) {
        status = i2d_panic("failed to search for node object");
    } else if(i2d_rbt_node_delete(tree, node)) {
        status = i2d_panic("failed to delete node object");
    } else {
        i2d_rbt_node_deit(&node);
    }

    return status;
}

int i2d_rbt_search(i2d_rbt * tree, void * key, void ** value) {
    int status = I2D_OK;
    i2d_rbt_node * node = NULL;

    if(i2d_rbt_node_search(tree, &node, key)) {
        status = I2D_FAIL;
    } else {
        *value = node->val;
    }

    return status;
}

int i2d_rbt_exist(i2d_rbt * tree, void * key) {
    i2d_rbt_node * node = NULL;
    return i2d_rbt_node_search(tree, &node, key);
}

static int i2d_rbt_node_init(i2d_rbt_node ** result, void * key, void * val) {
    int status = I2D_OK;
    i2d_rbt_node * object;

    if(i2d_is_invalid(result)) {
        status = i2d_panic("invalid paramater");
    } else {
        object = calloc(1, sizeof(*object));
        if(!object) {
            status = i2d_panic("out of memory");
        } else {
            object->key = key;
            object->val = val;
            object->c   = red;

            if(status)
                i2d_rbt_node_deit(&object);
            else
                *result = object;
        }
    }

    return status;
}

static void i2d_rbt_node_deit(i2d_rbt_node ** result) {
    i2d_rbt_node * object;

    object = *result;
    i2d_free(object);
    *result = NULL;
}

static int i2d_rbt_node_insert(i2d_rbt * tree, i2d_rbt_node * x) {
    i2d_rbt_node * p;    /* parent */
    i2d_rbt_node * i;    /* iterator */
    i2d_rbt_node * s;    /* parent's sibling */

    /* binary search for x's parent */
    p = NULL;
    i = tree->root;
    while(i) {
        p = i;
        i = (0 > tree->compare(x->key, i->key)) ? i->l : i->r;
    }

    /* link x with x's parent */
    x->p = p;
    if(is_nil(p)) {
        tree->root = x;
    } else {
        if(0 > tree->compare(x->key, p->key)) {
            p->l = x;
        } else {
            p->r = x;
        }
    }

    while(is_red(p)) {
        if(is_lchild(p)) {
            s = p->p->r;
            if(is_red(s)) {
                p->c = black;
                s->c = black;
                x = p->p;
                p = x->p;
                x->c = red;
            } else {
                if(is_rchild(x)) {
                    x = x->p;
                    left_rotate(tree, x);
                    p = x->p;
                }

                p->c = black;
                p->p->c = red;
                right_rotate(tree, p->p);
            }
        } else {
            s = p->p->l;
            if(is_red(s)) {
                p->c = black;
                s->c = black;
                x = p->p;
                p = x->p;
                x->c = red;
            } else {
                if(is_lchild(x)) {
                    x = x->p;
                    right_rotate(tree, x);
                    p = x->p;
                }

                p->c = black;
                p->p->c = red;
                left_rotate(tree, p->p);
            }
        }
    }

    tree->root->c = black;
    return I2D_OK;
}

static int i2d_rbt_node_delete(i2d_rbt * tree, i2d_rbt_node * x) {
    int c;
    i2d_rbt_node * y;    /* x's replacement */
    i2d_rbt_node * z;    /* x or y's successor */
    i2d_rbt_node * p;    /* z's parent */
    i2d_rbt_node * s;    /* z's sibling */
    i2d_rbt_node * r = tree->root;

    if(is_nil(x->l)) {
        z = x->r;
        p = x->p;
        c = x->c;
        change_parent(tree, x, z);
    } else if(is_nil(x->r)) {
        z = x->l;
        p = x->p;
        c = x->c;
        change_parent(tree, x, z);
    } else {
        /* get x's successor */
        y = x->r;
        while(y->l)
            y = y->l;
        c = y->c;

        if(y->p != x) {
            /* get y's successor to replace y */
            z = y->r;
            p = y->p;
            change_parent(tree, y, y->r);

            /* y inherits x's right sub tree */
            y->r = x->r;
            y->r->p = y;
        } else {
            /* y is x's successor and immediate right sub tree */
            z = y->r;
            p = y;
        }

        /* y inherits x's parent, left sub tree, and color */
        change_parent(tree, x, y);
        y->l = x->l;
        y->l->p = y;
        y->c = x->c;
    }

    if(c == black) {
        while(p && is_black(z)) {
            if(z == p->l) {
                s = p->r;

                if(is_red(s)) {
                    s->c = black;
                    p->c = red;
                    left_rotate(tree, p);
                    s = p->r;
                }

                if(is_nil(s)) {
                    z = p;
                    p = z->p;
                } else {
                    if(is_black(s->l) && is_black(s->r)) {
                        s->c = red;
                        z = p;
                        p = z->p;
                    } else {
                        if(is_black(s->r)) {
                            s->c = red;
                            s->l->c = black;
                            right_rotate(tree, s);
                            s = p->r;
                        }

                        s->c = p->c;
                        s->r->c = black;
                        p->c = black;
                        left_rotate(tree, p);
                        z = r;
                    }
                }
            } else {
                s = p->l;

                if(is_red(s)) {
                    s->c = black;
                    p->c = red;
                    right_rotate(tree, p);
                    s = p->l;
                }

                if(is_nil(s)) {
                    z = p;
                    p = z->p;
                } else {
                    if(is_black(s->l) && is_black(s->r)) {
                        s->c = red;
                        z = p;
                        p = z->p;
                    } else {
                        if(is_black(s->l)) {
                            s->c = red;
                            s->r->c = black;
                            left_rotate(tree, s);
                            s = p->l;
                        }

                        s->c = p->c;
                        s->l->c = black;
                        p->c = black;
                        right_rotate(tree, p);
                        z = r;
                    }
                }
            }
        }
        if(z)
            z->c = black;
    }

    (x)->l = NULL;
    (x)->r = NULL;
    return I2D_OK;
}

static int i2d_rbt_node_search(i2d_rbt * tree, i2d_rbt_node ** x, void * key) {
    int result;
    i2d_rbt_node * i;

    i = tree->root;
    while(i != NULL) {
        result = tree->compare(key, i->key);
        if(!result) {
            *x = i;
            return I2D_OK;
        } else if(0 > result) {
            i = i->l;
        } else {
            i = i->r;
        }
    }

    return I2D_FAIL;
}
