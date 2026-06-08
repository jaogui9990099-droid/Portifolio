#ifndef FORGE_TEST_UNITY_H
#define FORGE_TEST_UNITY_H

#include <stdio.h>
#include <stdlib.h>

static int _forge_test_pass = 0;
static int _forge_test_fail = 0;
static const char* _forge_test_name = "";

#define TEST_BEGIN(name) do { _forge_test_name = (name); } while (0)

#define ASSERT_TRUE(expr) \
    do { \
        if (!(expr)) { \
            fprintf(stderr, "  FAIL [%s] line %d: %s\n", _forge_test_name, __LINE__, #expr); \
            _forge_test_fail++; \
        } else { \
            _forge_test_pass++; \
        } \
    } while (0)

#define ASSERT_EQ(a, b)   ASSERT_TRUE((a) == (b))
#define ASSERT_NE(a, b)   ASSERT_TRUE((a) != (b))
#define ASSERT_NULL(p)    ASSERT_TRUE((p) == NULL)
#define ASSERT_NOTNULL(p) ASSERT_TRUE((p) != NULL)

#define TEST_REPORT() \
    do { \
        int total = _forge_test_pass + _forge_test_fail; \
        printf("\n%d/%d tests passed", _forge_test_pass, total); \
        if (_forge_test_fail == 0) printf(" -- OK\n"); \
        else { printf(" -- %d FAILED\n", _forge_test_fail); exit(1); } \
    } while (0)

#endif
