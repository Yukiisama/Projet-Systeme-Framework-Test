// Simple Test Framework (testfw)
#define _GNU_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>

#include "testfw.h"

#define DEFAULT_MODE TESTFW_FORKS
#define DEFAULT_SUITE "test"
#define DEFAULT_TIMEOUT 2

enum action_t
{
    EXECUTE,
    LIST
};

/* ********** USAGE ********** */

void usage(int argc, char *argv[])
{
    printf("Simple Test Framework (version %d.%d)\n", TESTFW_VERSION_MAJOR, TESTFW_VERSION_MINOR);
    printf("Usage: %s [options] [actions] [-- <testargs> ...]\n", argv[0]);
    printf("Register Options:\n");
    printf("  -r <suite.name>: register a function \"suite_name()\" as a test\n");
    printf("  -R <suite>: register all functions \"suite_*()\" as a test suite\n");
    printf("Actions:\n");
    printf("  -x: execute all registered tests (default action)\n");
    printf("  -l: list all registered tests\n");
    printf("Execution Options:\n");
    printf("  -m <mode>: set execution mode: \"forks\"|\"forkp\"|\"nofork\" [default \"forks\"]\n");
    printf("  -d <file>: compare test output with an expected file (using diff)\n");
    printf("  -g <pattern>: search for a pattern in test output (using grep)\n");
    printf("Other Options:\n");
    printf("  -o <logfile>: redirect test output to a log file\n");
    printf("  -O: redirect test stdout & stderr to /dev/null\n");
    printf("  -t <timeout>: set time limits for each test (in sec.) [default %d]\n", DEFAULT_TIMEOUT);
    printf("  -T: no timeout\n");
    printf("  -c: return the total number of test failures\n");
    printf("  -s: silent mode (framework only)\n");
    printf("  -S: full silent mode (both framework and test output)\n");
    printf("  -v: verbose mode\n");
    printf("  -h: print this help message\n");
    exit(EXIT_FAILURE);
}

/* ********** MAIN ********** */

int main(int argc, char *argv[])
{
    int opt;

    char *logfile = NULL;                   // defaul logfile (no log)
    char *cmd = NULL;                       // default external command (no command)
    int timeout = DEFAULT_TIMEOUT;          // timeout (in sec.)
    bool count = false;                     // return nb failures
    bool silent = false;                    // silent mode
    bool verbose = false;                   // verbose mode
    enum testfw_mode_t mode = DEFAULT_MODE; // default mode
    enum action_t action = EXECUTE;         // default action
    char *suite = DEFAULT_SUITE;            // default suite
    char *name = NULL;

    while ((opt = getopt(argc, argv, "g:d:vr:R:t:Tm:sSco:Olxh?")) != -1)
    {
        switch (opt)
        {
        // register tests
        case 'r':
        {
            char *sep = strchr(optarg, '.');
            if (!sep)
            {
                fprintf(stderr, "Error: invalid test name \"%s\"!\n", optarg);
                exit(EXIT_FAILURE);
            }
            *sep = 0;
            suite = optarg;
            name = sep + 1;
            break;
        }
        case 'R':
            suite = optarg;
            name = NULL;
            break;
        // actions
        case 'x':
            action = EXECUTE;
            break;
        case 'l':
            action = LIST;
            break;
        // options
        case 's':
            silent = true;
            break;
        case 'c':
            count = true;
            break;
        case 'o':
            assert(cmd == NULL && logfile == NULL);
            logfile = optarg;
            break;
        case 'O':
            assert(cmd == NULL && logfile == NULL);
            logfile = "/dev/null";
            break;
        case 'S':
            assert(cmd == NULL && logfile == NULL);
            silent = true;
            logfile = "/dev/null";
            break;
        case 't':
            timeout = atoi(optarg);
            break;
        case 'T':
            timeout = 0; // no timeout
            break;
        case 'm':
            if (strcmp(optarg, "forks") == 0)
                mode = TESTFW_FORKS;
            else if (strcmp(optarg, "forkp") == 0)
                mode = TESTFW_FORKP;
            else if (strcmp(optarg, "nofork") == 0)
                mode = TESTFW_NOFORK;
            else
            {
                fprintf(stderr, "Error: invalid execution mode \"%s\"!\n", optarg);
                exit(EXIT_FAILURE);
            }
            break;
        case 'd':
            assert(cmd == NULL && logfile == NULL);
            asprintf(&cmd, "diff %s -", optarg);
            break;
        case 'g':
            assert(cmd == NULL && logfile == NULL);
            asprintf(&cmd, "grep %s", optarg);
            break;
        case 'v':
            verbose = true;
            break;
        case '?':
        case 'h':
        default:
            usage(argc, argv);
            break;
        }
    }

    /* external command */

    int testargc = argc - optind;
    char **testargv = argv + optind;
    struct testfw_t *fw = testfw_init(argv[0], timeout, logfile, cmd, silent, verbose);

    /* register tests */
    if (suite && name)
        testfw_register_symb(fw, suite, name);
    else if (suite)
        testfw_register_suite(fw, suite);

    int length = testfw_length(fw);
    if (length == 0)
    {
        fprintf(stderr, "Error: no tests are registred in suite \"%s\"!\n", suite);
        return EXIT_FAILURE;
    }

    /* actions */
    int nfailures = 0;
    if (action == LIST)
    {
        for (int k = 0; k < length; k++)
        {
            struct test_t *test = testfw_get(fw, k);
            printf("%s.%s\n", test->suite, test->name);
        }
    }
    else if (action == EXECUTE)
    {
        nfailures = testfw_run_all(fw, testargc, testargv, mode);
    }
    else
        usage(argc, argv);

    /* final diagnostic */
    if (action == EXECUTE && !silent)
        printf("=> %.f%% tests passed, %d tests failed out of %d\n", (length - nfailures) * 100.0 / length, nfailures, length);

    /* free tests */
    testfw_free(fw);

    if (count || mode == TESTFW_NOFORK)
        return nfailures;

    free(cmd);

    return EXIT_SUCCESS;
}
