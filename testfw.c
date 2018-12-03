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

#include "testfw.h"

#define DEFAULT_SUITE_NAME "defaultSuiteName"
#define DEFAULT_TEST_NAME  "defaultTestName"
#define DEFAULT_NB_TESTS 30

/* ********** STRUCTURES ********** */

struct testfw_t
{
    char * program; // filename executable
    int timeout;
    char *logfile;  // rediger dans un fichier
    char *cmd;      // rediriger dans une commande
    bool silent;    // automatiser , tests par défaut ?
    bool verbose;   // affichage détailler dans la console (ie debug)

    struct test_t** tests;
    unsigned int nbTest; // contient le nombre de tests enregistré
    unsigned int lenTests; // contient la taille du tableau tests
};

/* ********** FRAMEWORK ********** */
struct testfw_t *testfw_init(char *program, int timeout, char *logfile, char *cmd, bool silent, bool verbose)
{
   timeout = ( timeout < 0 ) ? 0 : timeout;
   if(program == NULL) {
       perror("program file incorrect");
       exit(TESTFW_EXIT_FAILURE);
   }

    struct testfw_t * new = (struct testfw_t *) malloc(sizeof(struct testfw_t));
    if (new == NULL) {
        perror("malloc error");
        exit(TESTFW_EXIT_FAILURE);
    }

    new->tests = (struct test_t **) malloc(sizeof(struct test_t *) * DEFAULT_NB_TESTS);
    if (new->tests == NULL) {
        perror("malloc error");
        exit(TESTFW_EXIT_FAILURE);
    }
    
    for ( int i = 0; i < DEFAULT_NB_TESTS; i++) {
        new->tests[i] = (struct test_t *) malloc(sizeof(struct test_t));
        if (new->tests[i] == NULL) {
            perror("malloc error");
            exit(TESTFW_EXIT_FAILURE);
        }
    }
    
    new->program = program;
    new->timeout = timeout;
    new->logfile = logfile;
    new->cmd     = cmd;
    new->silent  = silent;
    new->verbose = verbose;

    new->nbTest  = 0;
    new->lenTests = DEFAULT_NB_TESTS;

    return new;
}

void testfw_free(struct testfw_t *fw)
{
    if (fw != NULL) {
        for (int i = 0; i < fw->lenTests; i++) {
            if (i < fw->nbTest) {
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
    return ( fw != NULL ) ? fw->nbTest : -1;
}

struct test_t *testfw_get(struct testfw_t *fw, int k)
{
    if ( fw != NULL && fw->tests != NULL ) {
        return ( k > fw->nbTest ) ? fw->tests[fw->nbTest - 1] : fw->tests[k];
    }
    return NULL;
}

/* ********** REGISTER TEST ********** */

struct test_t *testfw_register_func(struct testfw_t *fw, char *suite, char *name, testfw_func_t func)
{
    if (fw == NULL || fw->tests == NULL){
       perror("invalid struc");
       exit(TESTFW_EXIT_FAILURE);
   }

    if (suite == NULL)
        suite = DEFAULT_SUITE_NAME;
    if (name == NULL)
        name = DEFAULT_TEST_NAME;


    if ( fw->nbTest >= fw->lenTests ){
        fw->lenTests *= 2;
        fw->tests = (struct test_t **) realloc(fw->tests,fw->lenTests);
    }

    char* suitecpy = (char *) malloc((strlen(suite) + 1) * sizeof(char));
    char* namecpy = (char *) malloc((strlen(name) + 1)* sizeof(char));

    strcpy(suitecpy, suite);
    strcpy(namecpy, name);

    fw->tests[fw->nbTest]->suite = suitecpy;
    fw->tests[fw->nbTest]->name = namecpy;
    fw->tests[fw->nbTest]->func = func;
    fw->nbTest += 1;
    
    return fw->tests[fw->nbTest-1];
}

struct test_t *testfw_register_symb(struct testfw_t *fw, char *suite, char *name)
{
    if (fw == NULL || fw->tests ==NULL){
       perror("invalid struc");
       exit(TESTFW_EXIT_FAILURE);
    }
    int size = 128;
    char suitename[size];
    int n = snprintf(suitename, size, "%s_%s", suite, name);

    if (n >= sizeof(suitename)) {
        fprintf(stderr, "suitename too large for buffer\n");
        exit(TESTFW_EXIT_FAILURE);
    }
    testfw_func_t func;
    void * handle;

    handle = dlopen(fw->program, RTLD_LAZY);
    * (void **)(&func) = dlsym(handle, suitename);
    /*if (handle)
        dlclose(handle);
    */
    return testfw_register_func(fw, suite, name, func);
}

int testfw_register_suite(struct testfw_t *fw, char *suite)
{
    if ( fw == NULL || fw->tests == NULL){
        perror("invalid struct");
        exit(TESTFW_EXIT_FAILURE);
    }

    int size = 256, i;
    char buf[size], command[size];
    char *tok, *name, *ptr;
    int n = snprintf(command, size, "nm --defined-only %s | cut -d ' ' -f 3 | grep \"^%s_\"", fw->program, suite);
    
    if (n >= sizeof(command)) {
        fprintf(stderr, "command too long for buffer\n");
        exit(TESTFW_EXIT_FAILURE);
    }

    FILE * file = popen(command, "r");
    
    for (i = 0; fgets(buf, size, file) != NULL; i++) {
        tok = strtok(buf, "_"); // on récupère le test
        name = strtok(NULL, "_"); // on récupère le name
        if ( (ptr = strchr(name, '\n')) != NULL ) *ptr = '\0';
        testfw_register_symb(fw, tok, name);
    }

    pclose(file);

    return i;
}

/* ********** RUN TEST ********** */

pid_t fils;
void alarm_handler (int signal){
    exit(124);
}

int testfw_run_all(struct testfw_t *fw, int argc, char *argv[], enum testfw_mode_t mode)
{
    pid_t pid;
    if (fw == NULL) {
        perror("Null pointer ");
        exit(TESTFW_EXIT_FAILURE);
    }
    if (mode != TESTFW_FORKS) {
        perror("Mode not implemented yet! ");
        exit(TESTFW_EXIT_FAILURE);
    }
    if (fw->logfile != NULL) {
        int fd = open(fw->logfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("Can't open/create logfile ");
            exit(TESTFW_EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        close(fd);
    }
    if (fw->cmd != NULL) {
        FILE * file = popen(fw->cmd, "r");
        if (file == NULL) {
            perror("Can't execute command ");
            exit(TESTFW_EXIT_FAILURE);
        }

        // faut faire un truc mais jsp quoi j'ai pas compris

        pclose(file);
    }
    struct timeval start, end;
    //pid_t pid;
    int status, termSig, termState, nbFail = 0;
    char *strTermState, strTermSig[64];
    struct sigaction s;
    s.sa_handler = alarm_handler;
    s.sa_flags = 0;
    sigaction(SIGALRM,&s,NULL);
    
    for (int i = 0; i < fw->nbTest; i++) 
    {
        gettimeofday(&start, NULL);
        pid = fork();
        if (pid == 0) {
            if (fw->timeout != 0) alarm(fw->timeout);
            close(STDERR_FILENO);
            close(STDOUT_FILENO);
            fils = getpid();
            int ret = fw->tests[i]->func(argc, argv);
            exit(ret);
        }
        wait(&status);
        gettimeofday(&end, NULL);
        termSig   = WTERMSIG(status); // sigint qui a terminé le prog
        termState = WEXITSTATUS(status); // code retour du proc fils

        if (termState != 0 || termSig != 0) nbFail++;

        //TODO: reécrire cette partie pour la rendre plus jolie
        if (termSig != 0 && termSig != 1) {
            strTermState = "KILLED";
            snprintf(strTermSig, 64, "signal \"%s\"", strsignal(termSig));
        } else {
            if (termState == 124) {
                if ((end.tv_sec - start.tv_sec) >= fw->timeout) {
                    strTermState = "TIMEOUT";
                    snprintf(strTermSig, 64, "status %d", termState);
                } else {
                    strTermState = "KILLED";
                    snprintf(strTermSig, 64, "signal \"%s\"", strsignal(SIGALRM));
                }
            } else {
                strTermState = (termState == 0) ? "SUCCESS" : "FAILURE"; 
                snprintf(strTermSig, 64, "status %d", termState);
            }
        }
        float elapsed = ((end.tv_sec - start.tv_sec) * 1000.0) + ((end.tv_usec - start.tv_usec) / 1000.0);
        if (fw->verbose)
            printf("[%s] run test \"%s.%s\" in %lf ms (%s)\n", strTermState, fw->tests[i]->suite, fw->tests[i]->name, elapsed, strTermSig);
    }

    return nbFail;
}
