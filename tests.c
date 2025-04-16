#include <glib.h>
#include "interpreter.h"
#include <setjmp.h> 
#include <cmocka.h>

GHashTable *variables;
GHashTable *functions;

static void test_integer(void **state){

    (void) state;

	variables = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free);
	functions = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, freeFunctionDeclaration);

	struct Context *context = malloc(sizeof(struct Context));
	context->parent = NULL;
	context->variables = variables;
	context->functions = functions;

    char* s = "1";
    struct Node *n = parse(s);
    struct EvalNode result;
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, 1);

    s = "-1";
    n = parse(s);
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, -1);

    s = "+1";
    n = parse(s);
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, 1);

    s = "1+1";
    n = parse(s);
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, 2);

    s = "(1+1)";
    n = parse(s);
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, 2);

    s = "3+3+3+5*5*2+1+2+2";
    n = parse(s);
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, 64);

    s = "5*5*2+2+1+1+2*2*2*2";
    n = parse(s);
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, 70);

    s = "5*(5+6*(9-7)/3)";
    n = parse(s);
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, 45);

    s = "123+300";
    n = parse(s);
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, 423);

    s = "5   *      (     5     +    6    * ( 9   - 7 ) / 3 )";
    n = parse(s);
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, 45);

    s = "123      +         300     ";
    n = parse(s);
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, 423);

    s = "2>1";
    n = parse(s);
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, 1);

    s = "2<1";
    n = parse(s);
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, 0);

    s = "(2+2)>3";
    n = parse(s);
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, 1);

    s = "2+2>3";
    n = parse(s);
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, 1);

    s = "2*2>3";
    n = parse(s);
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, 1);

    s = "Integer a = 567";
    n = parse(s);
    eval(n, context);
    s = "a";
    n = parse(s);
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, 567);

    s = "Integer abc = 567";
    n = parse(s);
    eval(n, context);
    s = "abc=5";
    n = parse(s);
    eval(n, context);
    s = "abc";
    n = parse(s);
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, 5);

    g_hash_table_destroy(variables);
    g_hash_table_destroy(functions);

    free(context);
}

static void test_decimal(void **state){

    (void) state;

	variables = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free);
	functions = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, freeFunctionDeclaration);

	struct Context *context = malloc(sizeof(struct Context));
	context->parent = NULL;
	context->variables = variables;
	context->functions = functions;

    char* s = "1.0";
    struct Node *n = parse(s);
    struct EvalNode result;
    result = eval(n, context);

    assert_float_equal(result.value.decimalValue, 1.0, 0.1);

    s = "-1.0";
    n = parse(s);
    result = eval(n, context);

    assert_float_equal(result.value.decimalValue, -1.0, 0.1);

    s = "+1.0";
    n = parse(s);
    result = eval(n, context);

    assert_float_equal(result.value.decimalValue, 1.0, 0.1);

    s = "1.0+1.0";
    n = parse(s);
    result = eval(n, context);

    assert_float_equal(result.value.decimalValue, 1.0+1.0, 0.1);

    s = "(1.0+1.0)";
    n = parse(s);
    result = eval(n, context);

    assert_float_equal(result.value.decimalValue, (1.0+1.0), 0.1);

    s = "3.0+3.0+3.0+5.0*5.0*2.0+1.0+2.0+2.0";
    n = parse(s);
    result = eval(n, context);

    assert_float_equal(result.value.decimalValue, 3.0+3.0+3.0+5.0*5.0*2.0+1.0+2.0+2.0, 0.1);

    s = "5.0*5.0*2.0+2.0+1.0+1.0+2.0*2.0*2.0*2.0";
    n = parse(s);
    result = eval(n, context);

    assert_float_equal(result.value.decimalValue, 5.0*5.0*2.0+2.0+1.0+1.0+2.0*2.0*2.0*2.0, 0.1);

    s = "5.0*(5.0+6.0*(9.0-7.0)/3.0)";
    n = parse(s);
    result = eval(n, context);

    assert_float_equal(result.value.decimalValue, 5.0*(5.0+6.0*(9.0-7.0)/3.0), 0.1);

    s = "123.0+300.0";
    n = parse(s);
    result = eval(n, context);

    assert_float_equal(result.value.decimalValue, 123.0+300.0, 0.1);

    s = "5.0  *      (     5.0     +    6.0    * ( 9.0   - 7.0 ) / 3.0 )";
    n = parse(s);
    result = eval(n, context);

    assert_float_equal(result.value.decimalValue, 5.0  *      (     5.0     +    6.0    * ( 9.0   - 7.0 ) / 3.0 ), 0.1);

    s = "123.0      +         300.0     ";
    n = parse(s);
    result = eval(n, context);

    assert_float_equal(result.value.decimalValue, 123.0      +         300.0     , 0.1);

    s = "2.0>1.0";
    n = parse(s);
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, 1);

    s = "2.0<1.0";
    n = parse(s);
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, 0);

    s = "(2.0+2.0)>3.0";
    n = parse(s);
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, 1);

    s = "2.0+2.0>3.0";
    n = parse(s);
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, 1);

    s = "2.0*2.0>3.0";
    n = parse(s);
    result = eval(n, context);

    assert_int_equal(result.value.integerValue, 1);

    s = "Decimal a = 567.0";
    n = parse(s);
    eval(n, context);
    s = "a";
    n = parse(s);
    result = eval(n, context);

    assert_float_equal(result.value.decimalValue, 567.0, 0.1);

    s = "Decimal abc = 567.0";
    n = parse(s);
    eval(n, context);
    s = "abc=5.0";
    n = parse(s);
    eval(n, context);
    s = "abc";
    n = parse(s);
    result = eval(n, context);

    assert_float_equal(result.value.decimalValue, 5.0, 0.1);

    g_hash_table_destroy(variables);
    g_hash_table_destroy(functions);

    free(context);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_integer),
        cmocka_unit_test(test_decimal),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}