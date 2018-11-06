// Simple Test Framework (testfw)

#ifndef TESTFW_H
#define TESTFW_H

#include <stdbool.h>

/* ********** TEST FRAMEWORK API ********** */

#define TESTFW_VERSION_MAJOR 0
#define TESTFW_VERSION_MINOR 3
#define TESTFW_EXIT_SUCCESS EXIT_SUCCESS
#define TESTFW_EXIT_FAILURE EXIT_FAILURE
#define TESTFW_EXIT_TIMEOUT 124

/**
 * @brief execution modes
 */
enum testfw_mode_t
{
    TESTFW_FORKS, /**< sequential test execution with process fork */
    TESTFW_FORKP, /**< parallel test execution with process fork */
    TESTFW_NOFORK /**< sequential test execution without process fork */
};

/**
 * @brief test function type
 */
typedef int (*testfw_func_t)(int argc, char *argv[]);

/**
 * @brief test structure
 */
struct test_t
{
    char *suite;        /**< suite name */
    char *name;         /**< test name */
    testfw_func_t func; /**< test function */
};

/**
 * @brief test framework structure (forward decalaration)
 */
struct testfw_t;

/**
 * @brief initialize test framework
 *
 * @param program the filename of this executable
 * @param timeout the time limits (in sec.) for each test, else 0.
 * @param logfile the file in which to redirect all test outputs (standard & error), else NULL
 * @param cmd a shell command in which to redirect all test outputs (standard & erro),else NULL
 * @param silent if true, the test framework runs in silent mode
 * @param verbose if true, the test framework runs in verbose mode
 * @return a pointer on a new test framework structure
 */
struct testfw_t *testfw_init(char *program, int timeout, char *logfile, char *cmd, bool silent, bool verbose);

/**
 * @brief finalize the test framework and free all memory
 *
 * @param fw the test framework to be freed
 */
void testfw_free(struct testfw_t *fw);

/**
 * @brief get number of registered tests
 *
 * @param fw the test framework
 * @return the number of registered tests
 */
int testfw_length(struct testfw_t *fw);

/**
 * @brief get a registered test
 *
 * @param fw the test framework
 * @param k index of the test to get (k >=0)
 * @return a pointer on the k-th registered test
 */
struct test_t *testfw_get(struct testfw_t *fw, int k);

/**
 * @brief register a single test function
 *
 * @param fw the test framework
 * @param suite a suite name in which to register this test
 * @param name a test name
 * @param func a test function
 * @return a pointer to the structure, that registers this test
 */
struct test_t *testfw_register_func(struct testfw_t *fw, char *suite, char *name, testfw_func_t func);

/**
 * @brief register a single test function named "<suite>_<name>""
 *
 * @param fw the test framework
 * @param suite a suite name in which to register this test
 * @param name a test name
 * @return a pointer to the structure, that registers this test
 */
struct test_t *testfw_register_symb(struct testfw_t *fw, char *suite, char *name);

/**
 * @brief register all test functions named "<suite>_*"
 *
 * @param fw the test framework
 * @param suite a suite name in which to register these tests
 * @return the number of new registered tests
 */
int testfw_register_suite(struct testfw_t *fw, char *suite);

/**
 * @brief run all registered tests
 *
 * @param fw the test framework
 * @param argc the number of arguments passed to each test function
 * @param argv the array of arguments passed to each test function
 * @param mode the execution mode in which to run each test function
 * @return the number of tests that fail
 */
int testfw_run_all(struct testfw_t *fw, int argc, char *argv[], enum testfw_mode_t mode);

#endif
