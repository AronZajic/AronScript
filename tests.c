#include <glib.h>
#include "interpreter.h"
#include <setjmp.h> 
#include <cmocka.h>

GHashTable *variables;
GHashTable *functions;

static void test_add_basic(void **state){

    (void) state;

    variables = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free);
    functions = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free);

    char* s = "1";
    struct Node *n = parse(s);
    int result;
    result = eval(n, variables, functions).value;

    assert_int_equal(result, 1);

    s = "-1";
    n = parse(s);
    result = eval(n, variables, functions).value;

    assert_int_equal(result, -1);

    s = "+1";
    n = parse(s);
    result = eval(n, variables, functions).value;

    assert_int_equal(result, 1);

    s = "1+1";
    n = parse(s);
    result = eval(n, variables, functions).value;

    assert_int_equal(result, 2);

    s = "(1+1)";
    n = parse(s);
    result = eval(n, variables, functions).value;

    assert_int_equal(result, 2);

    s = "3+3+3+5*5*2+1+2+2";
    n = parse(s);
    result = eval(n, variables, functions).value;

    assert_int_equal(result, 64);

    s = "5*5*2+2+1+1+2*2*2*2";
    n = parse(s);
    result = eval(n, variables, functions).value;

    assert_int_equal(result, 70);

    s = "5*(5+6*(9-7)/3)";
    n = parse(s);
    result = eval(n, variables, functions).value;

    assert_int_equal(result, 45);

    s = "123+300";
    n = parse(s);
    result = eval(n, variables, functions).value;

    assert_int_equal(result, 423);

    s = "5   *      (     5     +    6    * ( 9   - 7 ) / 3 )";
    n = parse(s);
    result = eval(n, variables, functions).value;

    assert_int_equal(result, 45);

    s = "123      +         300     ";
    n = parse(s);
    result = eval(n, variables, functions).value;

    assert_int_equal(result, 423);

    s = "2>1";
    n = parse(s);
    result = eval(n, variables, functions).value;

    assert_int_equal(result, 1);

    s = "2<1";
    n = parse(s);
    result = eval(n, variables, functions).value;

    assert_int_equal(result, 0);

    s = "(2+2)>3";
    n = parse(s);
    result = eval(n, variables, functions).value;

    assert_int_equal(result, 1);

    s = "2+2>3";
    n = parse(s);
    result = eval(n, variables, functions).value;

    assert_int_equal(result, 1);

    s = "2*2>3";
    n = parse(s);
    result = eval(n, variables, functions).value;

    assert_int_equal(result, 1);

    s = "Integer a = 567";
    n = parse(s);
    eval(n, variables, functions);
    s = "a";
    n = parse(s);
    result = eval(n, variables, functions).value;

    assert_int_equal(result, 567);

    s = "Integer abc = 567";
    n = parse(s);
    eval(n, variables, functions);
    s = "abc=5";
    n = parse(s);
    eval(n, variables, functions);
    s = "abc";
    n = parse(s);
    result = eval(n, variables, functions).value;

    assert_int_equal(result, 5);

    g_hash_table_destroy(variables);
    g_hash_table_destroy(functions);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_add_basic),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}