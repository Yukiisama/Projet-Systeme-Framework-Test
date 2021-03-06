#define _GNU_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <setjmp.h>

#include "testfw.h"

#define DEFAULT_SUITE_NAME "defaultSuiteName"
#define DEFAULT_TEST_NAME "defaultTestName"
#define DEFAULT_NB_TESTS 30

/* ********** STRUCTURES ********** */

struct testfw_t
{
    char *program; // filename executable
    int timeout;   // max time to execute a test
    char *logfile; // redirect into logfile
    char *cmd;     // redirect into a cmd (popen used)
    bool silent;   // delete whole display information
    bool verbose;  // Use this for debug purpose , display more informations

    struct test_t **tests; // struct which contain all tests
    unsigned int nbTest;   // nb tests available
    unsigned int lenTests; // size of test tab
};

/* ********** FRAMEWORK ********** */

struct testfw_t *testfw_init(char *program, int timeout, char *logfile, char *cmd, bool silent, bool verbose)
{
    timeout = (timeout < 0) ? 0 : timeout;
    if (program == NULL)
    {
        perror("program file incorrect");
        exit(TESTFW_EXIT_FAILURE);
    }

    struct testfw_t *new = (struct testfw_t *)malloc(sizeof(struct testfw_t));
    if (new == NULL)
    {
        perror("malloc error");
        exit(TESTFW_EXIT_FAILURE);
    }

    new->tests = (struct test_t **)malloc(sizeof(struct test_t *) * DEFAULT_NB_TESTS);
    if (new->tests == NULL)
    {
        perror("malloc error");
        exit(TESTFW_EXIT_FAILURE);
    }

    for (int i = 0; i < DEFAULT_NB_TESTS; i++)
    {
        new->tests[i] = (struct test_t *)malloc(sizeof(struct test_t));
        if (new->tests[i] == NULL)
        {
            perror("malloc error");
            exit(TESTFW_EXIT_FAILURE);
        }
    }
    //Assign struct testfw_t
    new->program = program;
    new->timeout = timeout;
    new->logfile = logfile;
    new->cmd = cmd;
    new->silent = silent;
    new->verbose = verbose;

    new->nbTest = 0;
    new->lenTests = DEFAULT_NB_TESTS;

    return new;
}

void testfw_free(struct testfw_t *fw)
{
    if (fw != NULL)
    {
        for (int i = 0; i < fw->lenTests; i++)
        {
            if (i < fw->nbTest)
            {
                free(fw->tests[i]->name);
                free(fw->tests[i]->suite);
            }
            free(fw->tests[i]);
        }
        free(fw->tests);
    }
    free(fw);
}

int testfw_length(struct testfw_t *fw)
{
    return (fw != NULL) ? fw->nbTest : -1;
}

struct test_t *testfw_get(struct testfw_t *fw, int k)
{
    if (fw != NULL && fw->tests != NULL)
    {
        return (k > fw->nbTest) ? fw->tests[fw->nbTest - 1] : fw->tests[k];
    }
    return NULL;
}

/* ********** REGISTER TEST ********** */

struct test_t *testfw_register_func(struct testfw_t *fw, char *suite, char *name, testfw_func_t func)
{
    if (fw == NULL || fw->tests == NULL)
    {
        perror("invalid struc");
        exit(TESTFW_EXIT_FAILURE);
    }

    if (suite == NULL)
        suite = DEFAULT_SUITE_NAME;
    if (name == NULL)
        name = DEFAULT_TEST_NAME;

    if (fw->nbTest >= fw->lenTests)
    {
        fw->lenTests *= 2;
        fw->tests = (struct test_t **)realloc(fw->tests, fw->lenTests);
    }

    char *suitecpy = (char *)malloc((strlen(suite) + 1) * sizeof(char));
    char *namecpy = (char *)malloc((strlen(name) + 1) * sizeof(char));

    strcpy(suitecpy, suite);
    strcpy(namecpy, name);

    fw->tests[fw->nbTest]->suite = suitecpy;
    fw->tests[fw->nbTest]->name = namecpy;
    fw->tests[fw->nbTest]->func = func;
    fw->nbTest += 1;

    return fw->tests[fw->nbTest - 1];
}

struct test_t *testfw_register_symb(struct testfw_t *fw, char *suite, char *name)
{
    if (fw == NULL || fw->tests == NULL)
    {
        perror("invalid struc");
        exit(TESTFW_EXIT_FAILURE);
    }
    int size = 128;
    char suitename[size];
    int n = snprintf(suitename, size, "%s_%s", suite, name);

    if (n >= sizeof(suitename))
    {
        fprintf(stderr, "suitename too large for buffer\n");
        exit(TESTFW_EXIT_FAILURE);
    }
    testfw_func_t func;
    void *handle;
    /* loads the dynamic library file, if filename is NULL, then the returned handle is for the main program.
    RTLD_LAZY  Only resolve symbols as the code that references them is executed */
    handle = dlopen(NULL, RTLD_LAZY);
    *(void **)(&func) = dlsym(handle, suitename);
    if (handle)
        dlclose(handle);

    return testfw_register_func(fw, suite, name, func);
}

int testfw_register_suite(struct testfw_t *fw, char *suite)
{
    if (fw == NULL || fw->tests == NULL)
    {
        perror("invalid struct");
        exit(TESTFW_EXIT_FAILURE);
    }

    int size = 256, i;
    char buf[size], command[size];
    char *tok, *name, *ptr;
    int n = snprintf(command, size, "nm --defined-only %s | cut -d ' ' -f 3 | grep \"^%s_\"", fw->program, suite);

    if (n >= sizeof(command))
    {
        fprintf(stderr, "command too long for buffer\n");
        exit(TESTFW_EXIT_FAILURE);
    }

    FILE *file = popen(command, "r");

    for (i = 0; fgets(buf, size, file) != NULL; i++)
    {
        tok = strtok(buf, "_");   // We get the test
        name = strtok(NULL, "_"); // We get the name
        if ((ptr = strchr(name, '\n')) != NULL)
            *ptr = '\0';
        testfw_register_symb(fw, tok, name);
    }

    pclose(file);

    return i;
}

/* ********** RUN TEST ********** */
/**
 * @brief This handler will be used for timeout purpose
 *
 * @param Signal on which we specify the action ( SIG_ALRM)
 */
void alarm_handler(int signal)
{
    exit(TESTFW_EXIT_TIMEOUT);
}

/**
 * @brief Redirect all test outputs (standard & error)
 *
 * @param Test framework structure ( contain the logfile name)
 */
void redirect_logfile(struct testfw_t *fw)
{
    int fd = open(fw->logfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("Can't open/create logfile ");
        exit(TESTFW_EXIT_FAILURE);
    }
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    close(fd);
}

/**
 * @brief Redirect all test outputs (standard & error) in shell command 
 *
 * @param Test framework structure ( contain the command)
 * @param Alias of standard output 
 * @param Alias of standard error
 */
FILE *redirect_cmd(struct testfw_t *fw, int *std_save, int *err_save)
{
    FILE *file = popen(fw->cmd, "w");
    if (file == NULL)
    {
        perror("Can't execute command ");
        exit(TESTFW_EXIT_FAILURE);
    }
    *std_save = dup(STDOUT_FILENO);
    *err_save = dup(STDERR_FILENO);
    int fd = fileno(file);

    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
    if (dup2(fd, STDERR_FILENO) == -1)
    {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
    return file;
}

/**
 * @brief Launch the i-th test and setup timeout , verbose and silent mode if specify 
 *
 * @param Test framework structure ( contain verbose ,silent , timeout)
 * @param The i-th test to be run
 * @param Main int argc
 * @param Main char * argv[]
 */
int launch_test(struct testfw_t *fw, int i, int argc, char *argv[])
{
    if (fw == NULL)
    {
        perror("Null struct in launch_test ");
        exit(TESTFW_EXIT_FAILURE);
    }
    if (fw->verbose)
        printf("[DEBUG] Lancement du test : %s.%s avec timeout = %d, silent = %d\n",
               fw->tests[i]->suite,
               fw->tests[i]->name,
               fw->timeout,
               fw->silent);

    if (fw->silent)
    {
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }
    if (fw->timeout != 0)
        alarm(fw->timeout);

    return fw->tests[i]->func(argc, argv);
}
/**
 * @brief Launch the tests starting from start to end indice
 *
 * @param Test framework structure 
 * @param Indice of the first test to be run (execute tests start to end )
 * @param Indice of the last test to be run (execute tests start to end )
 * @param Main int argc
 * @param Main char * argv[]
 */
int launch_suite_test(struct testfw_t *fw, int start, int end, int argc, char *argv[])
{
    //Variable Declaration
    struct timeval time_start, time_end; //time struct to mesurate time
    pid_t pid;
    int status, termSig, termState, nbFail = 0, std_save, err_save;
    char *strTermState, strTermSig[64];
    struct sigaction s;
    FILE *file; //used if cmd != NULL
    float time;
    //Timeout purpose
    s.sa_handler = alarm_handler;
    s.sa_flags = 0;
    sigemptyset(&s.sa_mask);
    sigaction(SIGALRM, &s, NULL);

    if (fw->logfile != NULL)
    {
        redirect_logfile(fw);
    }
    /*Execute each tests from start to end using a fork child proccess each time you
    encounter a test .*/
    for (int i = start; i < end; i++)
    {
        if (fw->cmd != NULL)
        {
            file = redirect_cmd(fw, &std_save, &err_save);
        }
        gettimeofday(&time_start, NULL);
        pid = fork();
        if (pid == 0)
        {
            exit(launch_test(fw, i, argc, argv));
        }
        wait(&status);
        gettimeofday(&time_end, NULL);   // time_end is the ending time of the i-th test
        termSig = WTERMSIG(status);      // sigint which terminate the prog
        termState = WEXITSTATUS(status); // return code of the child process

        if (fw->cmd != NULL)
        {
            dup2(std_save, STDOUT_FILENO);
            dup2(err_save, STDERR_FILENO);
        }
        //if one test failed
        if (termState != TESTFW_EXIT_SUCCESS || termSig != TESTFW_EXIT_SUCCESS)
            nbFail++;
        //if test has been killed
        if (termSig != TESTFW_EXIT_SUCCESS && termSig != TESTFW_EXIT_FAILURE)
        {
            strTermState = "KILLED";
            snprintf(strTermSig, 64, "signal \"%s\"", strsignal(termSig));
        }
        else
        {
            if (termState == TESTFW_EXIT_TIMEOUT)
            {
                if ((time_end.tv_sec - time_start.tv_sec) >= fw->timeout)
                {
                    strTermState = "TIMEOUT";
                    snprintf(strTermSig, 64, "status %d", termState);
                }
                else
                {
                    strTermState = "KILLED";
                    snprintf(strTermSig, 64, "signal \"%s\"", strsignal(SIGALRM));
                }
            }
            else if (fw->cmd == NULL)
            {
                strTermState = (termState == TESTFW_EXIT_SUCCESS) ? "SUCCESS" : "FAILURE";
                snprintf(strTermSig, 64, "status %d", termState);
            }
            else
            {
                int ret = pclose(file);
                strTermState = (!ret) ? "SUCCESS" : "FAILURE";
                snprintf(strTermSig, 64, "status %d", WEXITSTATUS(ret));
            }
        }
        //time elapsed since test has been launched
        time = ((time_end.tv_sec - time_start.tv_sec) * 1000.0) + ((time_end.tv_usec - time_start.tv_usec) / 1000.0);
        printf("[%s] run test \"%s.%s\" in %.2lf ms (%s)\n",
               strTermState,
               fw->tests[i]->suite,
               fw->tests[i]->name,
               time,
               strTermSig);
        fflush(stdout);
        fflush(stderr);
    }
    return nbFail;
}

/**************************VARIABLES WE NEED TO BE GLOBAL ******************************/
jmp_buf buff;
int termSig, termState;
int state_timeout = 0;
int state_kill = 0;
int state_segv = 0;
/********************************************************/
/*
 * @brief This handler will be used defining new actions avoiding segfault etc to terminate well the test.
 * Use also for timeout purpose
 * @param Signal on which we specify the action ( SIG_ALRM)
 */
void handler_nofork(int signal)
{
    switch (signal)
    {

    case 6:
        termState = TESTFW_EXIT_FAILURE;
        state_kill = 1;
        siglongjmp(buff, 1);
        break;
    case 7:
        termState = TESTFW_EXIT_FAILURE;
        siglongjmp(buff, 1);
        break;
    case 11:
        termState = TESTFW_EXIT_FAILURE;
        state_segv = 1;
        siglongjmp(buff, 1);
        break;
    case 14:
        termState = TESTFW_EXIT_TIMEOUT;
        state_timeout = 1;
        siglongjmp(buff, 1);
        break;
    default:
        termState = TESTFW_EXIT_FAILURE;
        break;
    }
}

int testfw_run_all(struct testfw_t *fw, int argc, char *argv[], enum testfw_mode_t mode)
{
    if (fw == NULL)
    {
        perror("Null pointer ");
        exit(TESTFW_EXIT_FAILURE);
    }
    struct timeval time_start, time_end; //time struct to mesurate time
    double time;
    int result, std_save, err_save;
    ;
    char *strTermState, strTermSig[64];
    int total_fail = 0, status;
    pid_t pid;
    struct sigaction s_nofork;
    FILE *file; //used if cmd != NULL

    switch (mode)
    {
    // Each test process sequentially
    default:
        total_fail = launch_suite_test(fw, 0, fw->nbTest, argc, argv);
        break;
    // Parallel execution
    case TESTFW_FORKP:
        for (int i = 0; i < fw->nbTest; i++)
        {
            pid = fork();
            if (pid == 0)
            {
                exit(launch_suite_test(fw, i, i + 1, argc, argv));
            }
        }
        for (int i = 0; i < fw->nbTest; i++)
        {
            wait(&status);
            if (WEXITSTATUS(status) != TESTFW_EXIT_SUCCESS)
                total_fail += 1;
        }
        break;
    case TESTFW_NOFORK:

        s_nofork.sa_flags = SA_RESTART;
        s_nofork.sa_handler = handler_nofork;
        sigemptyset(&s_nofork.sa_mask);
        if (fw->logfile != NULL)
        {
            redirect_logfile(fw);
        }
        //Masqued all signals
        for (int i = 0; i < NSIG; i++)
        {
            sigaction(i, &s_nofork, NULL);
        }
        // run each test until a test failed
        for (int i = 0; i < fw->nbTest; i++)
        {
            //init time start
            gettimeofday(&time_start, NULL);

            if (fw->cmd != NULL)
            {
				
                file = redirect_cmd(fw, &std_save, &err_save);
            }
            //redirect
            if (fw->cmd != NULL)
            {
                dup2(std_save, STDOUT_FILENO);
                dup2(err_save, STDERR_FILENO);
            }
            // sigsetjmp == 0 until siglongjmp call , i.e when a test failed
            if (sigsetjmp(buff, 1) == 0)
                result = launch_test(fw, i, argc, argv);
            //We already execute the test and conclude it was a fail , we print final informations and leave there
            else
            {
                result = 1;
            }
            switch (result)
            {
            case 0:
                termSig = TESTFW_EXIT_SUCCESS;
                termState = TESTFW_EXIT_SUCCESS;
                break;
            default:
                termSig = TESTFW_EXIT_FAILURE;
                if (state_timeout)
                    termState = TESTFW_EXIT_TIMEOUT;
                else if (state_kill)
                    termSig = 6;
                else if (state_segv)
                    termSig = 11;
                else
                {
                    termSig = TESTFW_EXIT_FAILURE;
                    termState = TESTFW_EXIT_FAILURE;
                }
                break;
            }
            
            gettimeofday(&time_end, NULL); // time_end is the ending time of the i-th test
            //if one test failed

            if (termState != TESTFW_EXIT_SUCCESS || termSig != TESTFW_EXIT_SUCCESS)
                total_fail++;
            //if test has been killed
            if (termSig != TESTFW_EXIT_SUCCESS && termSig != TESTFW_EXIT_FAILURE)
            {
                strTermState = "KILLED";
                snprintf(strTermSig, 64, "signal \"%s\"", strsignal(termSig));
            }
            else
            {
                if (termState == TESTFW_EXIT_TIMEOUT)
                {
                    if ((time_end.tv_sec - time_start.tv_sec) >= fw->timeout)
                    {
                        strTermState = "TIMEOUT";
                        snprintf(strTermSig, 64, "status %d", termState);
                    }
                    else
                    {
                        strTermState = "KILLED";
                        snprintf(strTermSig, 64, "signal \"%s \"", strsignal(SIGALRM));
                    }
                }
                //time elapsed since test has been launched
                else if (fw->cmd == NULL)
                {
                    strTermState = (termState == TESTFW_EXIT_SUCCESS) ? "SUCCESS" : "FAILURE";
                    snprintf(strTermSig, 64, "status %d", termState);
                }
                else
				{
                int ret = pclose(file);
                strTermState = (!ret) ? "SUCCESS" : "FAILURE";
                snprintf(strTermSig, 64, "status %d", WEXITSTATUS(ret));
				}
            }

            //time elapsed since test has been launched
            time = ((time_end.tv_sec - time_start.tv_sec) * 1000.0) + ((time_end.tv_usec - time_start.tv_usec) / 1000.0);
            printf("[%s] run test \"%s.%s\" in %.2lf ms (%s)\n",
                   strTermState,
                   fw->tests[i]->suite,
                   fw->tests[i]->name,
                   time,
                   strTermSig);
            fflush(stdout);
            fflush(stderr);

            if (result == 1 && (termState != TESTFW_EXIT_SUCCESS || termSig != TESTFW_EXIT_SUCCESS))
            {
                //In the nofork mode, each test is runned directly as a function call (without fork).
                // As a consequence, the first test that fails will interrupt all the following.
                return total_fail;
            }
        }

        break;
    }

    return total_fail;
}
