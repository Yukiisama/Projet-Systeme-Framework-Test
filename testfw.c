#define _GNU_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "testfw.h"
#include <dlfcn.h>

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
    bool verbose;   // affichage détailler dans la console ( ie debug)

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

    //TODO: voir si il ne faut pas recopier le contenu des char* plutot que de recopier le pointeur
    //TODO: voir si il faut pas remplacer le struct test_t ** par struct test_t *


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

    char* suitecpy = (char *) malloc(strlen(suite) * sizeof(char) + 1);
    char* namecpy = (char *) malloc(strlen(name) * sizeof(char) + 1);

    strcpy(suitecpy, suite);
    strcpy(namecpy, name);

    fw->tests[fw->nbTest]->suite = suitecpy;
    fw->tests[fw->nbTest]->name = namecpy;
    fw->tests[fw->nbTest]->func = func;
    fw->nbTest += 1;
    //printf(" %s | %s \n", suite , name);
    return fw->tests[fw->nbTest-1];
}

struct test_t *testfw_register_symb(struct testfw_t *fw, char *suite, char *name)
{
    if (fw == NULL || fw->tests ==NULL){
       perror("invalid struc");
       exit(TESTFW_EXIT_FAILURE);
    }
    char suitename[128];
    int n = snprintf(suitename, 128, "%s_%s", suite, name);

    if (n >= sizeof(suitename)) {
        fprintf(stderr, "suitename too large for buffer\n");
        exit(TESTFW_EXIT_FAILURE);
    }

    void * handle = dlopen(fw->program,RTLD_LAZY);
    testfw_func_t func;
    * (void **)(&func) = dlsym(handle,suitename);
    if (handle)
        dlclose(handle);

    return testfw_register_func(fw,suite,name,func);
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
    int n = snprintf(command, size, "nm --defined-only %s | cut -d ' ' -f 3 | grep \"^%s\"", fw->program, suite);
    
    if(n >= sizeof(command)) {
        fprintf(stderr, "command too long for buffer\n");
        exit(TESTFW_EXIT_FAILURE);
    }

    FILE * file = popen(command, "r");
    
    for(i = 0; fgets(buf, size, file) != NULL; i++) {
        tok = strtok(buf, "_"); // on récupère le test
        name = strtok(NULL, "_"); // on récupère le name
        if ( (ptr = strchr(name, '\n')) != NULL ) *ptr = '\0';
        testfw_register_symb(fw, tok, name);
    }

    pclose(file);

    return i;
}

/* ********** RUN TEST ********** */

int testfw_run_all(struct testfw_t *fw, int argc, char *argv[], enum testfw_mode_t mode)
{
    return 0;
}
