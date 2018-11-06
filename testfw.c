#define _GNU_SOURCE
#include <stdbool.h>
#include <stdio.h>

#include "testfw.h"

/* ********** STRUCTURES ********** */

struct testfw_t
{
    /* ... */
};

/* ********** FRAMEWORK ********** */

struct testfw_t *testfw_init(char *program, int timeout, char *logfile, char *cmd, bool silent, bool verbose)
{
    return NULL;
}

void testfw_free(struct testfw_t *fw)
{
}

int testfw_length(struct testfw_t *fw)
{
    return 0;
}

struct test_t *testfw_get(struct testfw_t *fw, int k)
{
    return NULL;
}

/* ********** REGISTER TEST ********** */

struct test_t *testfw_register_func(struct testfw_t *fw, char *suite, char *name, testfw_func_t func)
{
    return NULL;
}

struct test_t *testfw_register_symb(struct testfw_t *fw, char *suite, char *name)
{
    return NULL;
}

int testfw_register_suite(struct testfw_t *fw, char *suite)
{
    return 0;
}

/* ********** RUN TEST ********** */

int testfw_run_all(struct testfw_t *fw, int argc, char *argv[], enum testfw_mode_t mode)
{
    return 0;
}
