#include <stdlib.h>
#include <stdbool.h>
#include "testfw.h"
#include "sample.h"

#define TIMEOUT 2
#define LOGFILE "test.log"
#define SILENT false
#define VERBOSE false
#define COMMAND NULL

int main(int argc, char *argv[])
{
    struct testfw_t *fw = testfw_init(argv[0], TIMEOUT, LOGFILE, COMMAND, SILENT, VERBOSE);
    testfw_register_func(fw, "test", "success", test_success);
    testfw_register_symb(fw, "test", "failure");
    testfw_register_suite(fw, "othertest");
    testfw_run_all(fw, argc - 1, argv + 1, TESTFW_FORKS);
    testfw_free(fw);
    return EXIT_SUCCESS;
}