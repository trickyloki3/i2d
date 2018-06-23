#include "i2d_util.h"
#include "i2d_range.h"
#include "i2d_logic.h"
#include "i2d_rbt.h"
#include "i2d_opt.h"
#include "i2d_item.h"
#include "i2d_skill.h"
#include "i2d_constant.h"
#include "i2d_db.h"
#include "i2d_script.h"

static void i2d_format_test(void);
static void i2d_lexer_test(void);
static void i2d_range_not_test(void);
static void i2d_logic_test(void);
static void i2d_logic_or_test(i2d_logic *, i2d_logic *, i2d_logic *);
static void i2d_logic_and_test(i2d_logic *, i2d_logic *, i2d_logic *);
static void i2d_logic_not_test(i2d_logic *, i2d_logic *, i2d_logic *);

int main(int argc, char * argv[]) {
    i2d_format_test();
    i2d_range_not_test();
    i2d_lexer_test();
    i2d_logic_test();
    return 0;
}

static void i2d_format_test(void) {
    i2d_buffer buffer;
    i2d_string_stack stack;
    i2d_format format;

    assert(!i2d_buffer_create(&buffer, I2D_SIZE));
    assert(!i2d_string_stack_create(&stack, 16));
    assert(!i2d_string_stack_push(&stack, "Hello", 5));
    assert(!i2d_string_stack_push(&stack, "World", 5));
    assert(!i2d_format_create(&format, "{0} {1}!", 8));
    assert(!i2d_format_write(&format, &stack, &buffer));
    i2d_format_destroy(&format);
    i2d_string_stack_destroy(&stack);
    i2d_buffer_destroy(&buffer);
}

static void i2d_lexer_test(void) {
    i2d_lexer * lexer = NULL;
    i2d_string script;
    i2d_token * token = NULL;
    i2d_token * tokens = NULL;

    int i;
    int j;
    enum i2d_token_type sequence[] = { I2D_LITERAL, I2D_CURLY_OPEN, I2D_CURLY_CLOSE, I2D_PARENTHESIS_OPEN, I2D_PARENTHESIS_CLOSE, I2D_COMMA, I2D_SEMICOLON, I2D_LITERAL, I2D_LITERAL, I2D_LITERAL, I2D_LITERAL, I2D_TEMPORARY_CHARACTER, I2D_PERMANENT_GLOBAL, I2D_TEMPORARY_GLOBAL, I2D_TEMPORARY_NPC, I2D_TEMPORARY_SCOPE, I2D_TEMPORARY_INSTANCE, I2D_PERMANENT_ACCOUNT_LOCAL, I2D_PERMANENT_ACCOUNT_GLOBAL, I2D_ADD, I2D_SUBTRACT, I2D_MULTIPLY, I2D_DIVIDE, I2D_MODULUS, I2D_ADD_ASSIGN, I2D_SUBTRACT_ASSIGN, I2D_MULTIPLY_ASSIGN, I2D_DIVIDE_ASSIGN, I2D_MODULUS_ASSIGN, I2D_GREATER, I2D_LESS, I2D_NOT, I2D_EQUAL, I2D_GREATER_EQUAL, I2D_LESS_EQUAL, I2D_NOT_EQUAL, I2D_RIGHT_SHIFT, I2D_LEFT_SHIFT, I2D_BIT_AND, I2D_BIT_OR, I2D_BIT_XOR, I2D_BIT_NOT, I2D_RIGHT_SHIFT_ASSIGN, I2D_LEFT_SHIFT_ASSIGN, I2D_BIT_AND_ASSIGN, I2D_BIT_OR_ASSIGN, I2D_BIT_XOR_ASSIGN, I2D_AND, I2D_OR, I2D_CONDITIONAL, I2D_COLON, I2D_UNIQUE_NAME, I2D_ASSIGN };

    assert(!i2d_lexer_init(&lexer));
    assert(!i2d_string_create(&script, "//\n\"QUOTE\"/*123*/{}(),; _var1 var2 1234 0x11 @ $ $@ . .@ ' # ## + - * / % += -= *= /= %= > < ! == >= <= != >> <<  & | ^ ~ >>= <<= &= |= ^= && || ? : :: =", 147));
    for(j = 0; j < 2; j++) {
        assert(!i2d_lexer_tokenize(lexer, &script, &tokens));
        i = 0;
        token = tokens->next;
        while(token != tokens) {
            assert(token->type == sequence[i++]);
            token = token->next;
        }
        i2d_lexer_reset(lexer, &tokens);
    }
    i2d_string_destroy(&script);
    i2d_lexer_deit(&lexer);

    assert(!i2d_lexer_init(&lexer));
    assert(!i2d_string_create(&script, "@var $var $@var .var .@var 'var #var ##var @var$ $var$ $@var$ .var$ .@var$ 'var$ #var$ ##var$", 93));
    assert(!i2d_lexer_tokenize(lexer, &script, &tokens));
    token = tokens->next;
    while(token != tokens) {
        assert(token->type == I2D_LITERAL);
        token = token->next;
    }
    i2d_lexer_reset(lexer, &tokens);
    i2d_string_destroy(&script);
    i2d_lexer_deit(&lexer);
}

static void i2d_range_not_test(void) {
    i2d_range range1;
    i2d_range range2;
    i2d_range range3;
    i2d_range range4;
    i2d_range range5;
    i2d_range range6;

    i2d_zero(range1);
    i2d_zero(range2);
    i2d_zero(range3);
    i2d_zero(range4);
    i2d_zero(range5);
    i2d_zero(range6);

    i2d_range_create_add(&range1, 0, 15);
    i2d_range_create_add(&range2, 2, 3);
    i2d_range_create_add(&range3, 5, 10);
    i2d_range_and(&range4, &range1, &range2);
    i2d_range_or(&range5, &range4, &range3);
    i2d_range_not(&range6, &range5);

    i2d_range_print(&range1, "range1");
    i2d_range_print(&range2, "range2");
    i2d_range_print(&range3, "range3");
    i2d_range_print(&range4, "range4");
    i2d_range_print(&range5, "range5");
    i2d_range_print(&range6, "range6");

    i2d_range_destroy(&range6);
    i2d_range_destroy(&range5);
    i2d_range_destroy(&range4);
    i2d_range_destroy(&range3);
    i2d_range_destroy(&range2);
    i2d_range_destroy(&range1);
}

static void i2d_logic_test(void) {
    i2d_string getrefine;
    i2d_string readparam;
    i2d_range getrefine_range;
    i2d_range readparam_range;
    i2d_logic * getrefine_logic = NULL;
    i2d_logic * readparam_logic = NULL;
    i2d_logic * and_logic = NULL;
    i2d_logic * or_logic = NULL;

    i2d_string_create(&getrefine, "getrefine", 9);
    i2d_string_create(&readparam, "readparam", 9);
    i2d_range_create(&getrefine_range);
    i2d_range_create(&readparam_range);
    i2d_range_add(&getrefine_range, 0, 15);
    i2d_range_add(&readparam_range, 1, 99);
    i2d_logic_init(&getrefine_logic, &getrefine, &getrefine_range);
    i2d_logic_init(&readparam_logic, &readparam, &readparam_range);
    i2d_logic_var(&and_logic, getrefine_logic, readparam_logic, and);
    i2d_logic_var(&or_logic, getrefine_logic, readparam_logic, or);
    i2d_logic_or_test(readparam_logic, and_logic, or_logic);
    i2d_logic_and_test(readparam_logic, and_logic, or_logic);
    i2d_logic_not_test(readparam_logic, and_logic, or_logic);
    i2d_logic_deit(&or_logic);
    i2d_logic_deit(&and_logic);
    i2d_logic_deit(&readparam_logic);
    i2d_logic_deit(&getrefine_logic);
    i2d_range_destroy(&readparam_range);
    i2d_range_destroy(&getrefine_range);
    i2d_string_destroy(&readparam);
    i2d_string_destroy(&getrefine);
}

void i2d_logic_or_test(i2d_logic * var_logic, i2d_logic * and_logic, i2d_logic * or_logic) {
    i2d_logic * or_var_var = NULL;
    i2d_logic * or_var_and = NULL;
    i2d_logic * or_var_or = NULL;
    i2d_logic * or_and_var = NULL;
    i2d_logic * or_and_and = NULL;
    i2d_logic * or_and_or = NULL;
    i2d_logic * or_or_var = NULL;
    i2d_logic * or_or_and = NULL;
    i2d_logic * or_or_or = NULL;
    i2d_logic_or(&or_var_var, var_logic, var_logic);
    i2d_logic_or(&or_var_and, var_logic, and_logic);
    i2d_logic_or(&or_var_or, var_logic, or_logic);
    i2d_logic_or(&or_and_var, and_logic, var_logic);
    i2d_logic_or(&or_and_and, and_logic, and_logic);
    i2d_logic_or(&or_and_or, and_logic, or_logic);
    i2d_logic_or(&or_or_var, or_logic, var_logic);
    i2d_logic_or(&or_or_and, or_logic, and_logic);
    i2d_logic_or(&or_or_or, or_logic, or_logic);
    fprintf(stderr, "or test results\n");
    i2d_logic_print(or_var_var, 0);
    i2d_logic_print(or_var_and, 0);
    i2d_logic_print(or_var_or, 0);
    i2d_logic_print(or_and_var, 0);
    i2d_logic_print(or_and_and, 0);
    i2d_logic_print(or_and_or, 0);
    i2d_logic_print(or_or_var, 0);
    i2d_logic_print(or_or_and, 0);
    i2d_logic_print(or_or_or, 0);
    i2d_logic_deit(&or_or_or);
    i2d_logic_deit(&or_or_and);
    i2d_logic_deit(&or_or_var);
    i2d_logic_deit(&or_and_or);
    i2d_logic_deit(&or_and_and);
    i2d_logic_deit(&or_and_var);
    i2d_logic_deit(&or_var_or);
    i2d_logic_deit(&or_var_and);
    i2d_logic_deit(&or_var_var);
}

static void i2d_logic_and_test(i2d_logic * var_logic, i2d_logic * and_logic, i2d_logic * or_logic) {
    i2d_logic * and_var_var = NULL;
    i2d_logic * and_var_and = NULL;
    i2d_logic * and_var_or = NULL;
    i2d_logic * and_and_var = NULL;
    i2d_logic * and_and_and = NULL;
    i2d_logic * and_and_or = NULL;
    i2d_logic * and_or_var = NULL;
    i2d_logic * and_or_and = NULL;
    i2d_logic * and_or_or = NULL;
    i2d_logic_and(&and_var_var, var_logic, var_logic);
    i2d_logic_and(&and_var_and, var_logic, and_logic);
    i2d_logic_and(&and_var_or, var_logic, or_logic);
    i2d_logic_and(&and_and_var, and_logic, var_logic);
    i2d_logic_and(&and_and_and, and_logic, and_logic);
    i2d_logic_and(&and_and_or, and_logic, or_logic);
    i2d_logic_and(&and_or_var, or_logic, var_logic);
    i2d_logic_and(&and_or_and, or_logic, and_logic);
    i2d_logic_and(&and_or_or, or_logic, or_logic);
    fprintf(stderr, "and test results\n");
    i2d_logic_print(and_var_var, 0);
    i2d_logic_print(and_var_and, 0);
    i2d_logic_print(and_var_or, 0);
    i2d_logic_print(and_and_var, 0);
    i2d_logic_print(and_and_and, 0);
    i2d_logic_print(and_and_or, 0);
    i2d_logic_print(and_or_var, 0);
    i2d_logic_print(and_or_and, 0);
    i2d_logic_print(and_or_or, 0);
    i2d_logic_deit(&and_or_or);
    i2d_logic_deit(&and_or_and);
    i2d_logic_deit(&and_or_var);
    i2d_logic_deit(&and_and_or);
    i2d_logic_deit(&and_and_and);
    i2d_logic_deit(&and_and_var);
    i2d_logic_deit(&and_var_or);
    i2d_logic_deit(&and_var_and);
    i2d_logic_deit(&and_var_var);
}

static void i2d_logic_not_test(i2d_logic * var_logic, i2d_logic * and_logic, i2d_logic * or_logic) {
    i2d_logic * not_var = NULL;
    i2d_logic * not_and = NULL;
    i2d_logic * not_or = NULL;
    i2d_logic_not(&not_var, var_logic);
    i2d_logic_not(&not_and, and_logic);
    i2d_logic_not(&not_or, or_logic);
    fprintf(stderr, "not test results\n");
    i2d_logic_print(not_var, 0);
    i2d_logic_print(not_and, 0);
    i2d_logic_print(not_or, 0);
    i2d_logic_deit(&not_or);
    i2d_logic_deit(&not_and);
    i2d_logic_deit(&not_var);
}
