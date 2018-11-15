#include <stdlib.h>
#include <stdbool.h>
#include "testfw.h"
#include "sample.h"
#include <stdio.h>

#define TIMEOUT 2
#define LOGFILE "test.log"
#define SILENT false
#define VERBOSE false
#define COMMAND NULL

int main(int argc, char *argv[])
{
    struct testfw_t *fw = testfw_init(argv[0], TIMEOUT, LOGFILE, COMMAND, SILENT, VERBOSE);
    testfw_register_func(fw, "test", "failure", test_failure);
    testfw_register_symb(fw, "othertest", "success");
    testfw_register_suite(fw, "test");

    struct test_t *test = testfw_get(fw, 0);
    printf("get du 1er test : %s_%s\n", test->suite, test->name);
    test = testfw_get(fw, 1);
    printf("get du 2eme test : %s_%s\n", test->suite, test->name);
    testfw_run_all(fw, argc - 1, argv + 1, TESTFW_FORKS);
    testfw_free(fw);
    return EXIT_SUCCESS;
}