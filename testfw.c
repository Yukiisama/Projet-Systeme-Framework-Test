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
    char *logfile; // rediger dans un fichier
    char *cmd; // rediriger dans une commande
    bool silent; // automatiser , tests par défaut ?
    bool verbose; // affichage détailler dans la console ( ie debug)
    char * full_program; // contien le nom complet du programme
    struct test_t** tests;
    unsigned int nbTest; // contient le nombre de tests enregistré
    unsigned int lenTests; // contient la taille du tableau tests
};

/* ********** FRAMEWORK ********** */
struct testfw_t *testfw_init(char *program, int timeout, char *logfile, char *cmd, bool silent, bool verbose)
{
   timeout = ( timeout < 0 ) ? 0 : timeout;
   if(program==NULL){
       perror("program file incorrect");
       exit(TESTFW_EXIT_FAILURE);
   }

    struct testfw_t * new = (struct testfw_t *) malloc(sizeof(struct testfw_t));

    //Check new struct is correct
    if (new == NULL){
        perror("malloc error");
        exit(TESTFW_EXIT_FAILURE);
    }

    new->tests = (struct test_t **) malloc(sizeof(struct test_t *) * DEFAULT_NB_TESTS);
    if ( new->tests == NULL ) {
        perror("malloc error");
        exit(TESTFW_EXIT_FAILURE);
    }
    for ( int i = 0; i < DEFAULT_NB_TESTS; i++) {
        new->tests[i] = (struct test_t *) malloc(sizeof(struct test_t));
        if ( new->tests[i] == NULL ) {
            perror("malloc error");
            exit(TESTFW_EXIT_FAILURE);
        }
    }
    
    int len = strlen(program) - 2;
    char* program_name = malloc(sizeof(char) * len);
    for(int i = 0; i < len; i++) {
        program_name[i] = program[i + 2];
    }

    new->program = program_name;
    new->full_program = program;
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
    if ( fw != NULL ) {
        for (int i = 0; i < fw->lenTests; i++ ) {
            free(fw->tests[i]->name);
            free(fw->tests[i]->suite);
            free(fw->tests[i]);
        }
        free(fw->tests);
        free(fw->program);
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
       exit(EXIT_FAILURE);
   }

    if (suite == NULL)
        suite = DEFAULT_SUITE_NAME;
    if (name == NULL)
        name = DEFAULT_TEST_NAME;


    if ( fw->nbTest >= fw->lenTests ){
        fw->lenTests *= 2;
        fw->tests = (struct test_t **) realloc(fw->tests,fw->lenTests);
    }

    char* suitecpy = malloc(strlen(suite)*sizeof(char));
    char* namecpy = malloc(strlen(name)*sizeof(char));

    strcpy(suitecpy, suite);
    strcpy(namecpy, name);

    namecpy[strlen(namecpy) - 1] = '\0';

    fw->tests[fw->nbTest]->suite = suitecpy;
    fw->tests[fw->nbTest]->name = namecpy;
    fw->tests[fw->nbTest]->func = func;
    fw->nbTest += 1;
    //printf(" %s | %s \n", suite , name);
    return fw->tests[fw->nbTest-1];
}

struct test_t *testfw_register_symb(struct testfw_t *fw, char *suite, char *name)
{
    if (fw == NULL || fw->tests == NULL){
       perror("invalid struc");
       exit(EXIT_FAILURE);
    }

    void * handle = dlopen(fw->full_program, RTLD_LAZY);
    testfw_func_t func; 
    char suitename[strlen(suite) + strlen(name)];
    sprintf(suitename,"%s_%s",suite,name);
    int* tmp = (int*) dlsym(handle,suitename);
    func = (testfw_func_t) &tmp;

    if (handle) 
        dlclose(handle);

    return testfw_register_func(fw,suite,name,func);
}

int testfw_register_suite(struct testfw_t *fw, char *suite)
{
    if ( fw == NULL || fw->tests == NULL){
        perror("invalid struct");
        exit(EXIT_FAILURE);
    }

    int size = 512, i = 0;
    char buf[size];
    char *tok, *name;
    int commandLen;
    
    commandLen = strlen("nm --defined-only  | cut -d ' ' -f 3 | grep \"^\"");
    commandLen += strlen(suite);
    commandLen += strlen(fw->program);

    char command[commandLen];
    sprintf(command, "nm --defined-only %s | cut -d ' ' -f 3 | grep \"^%s\"", fw->program, suite);

    FILE * file = popen(command, "r");
    
    for(;fgets(buf, size, file) != NULL; i++) {
        tok = strtok(buf, "_"); // on récupère le test
        name = strtok(NULL, "_"); // on récupère le name
        testfw_register_symb(fw, tok, name);
    }

    return i;
}

/* ********** RUN TEST ********** */

int testfw_run_all(struct testfw_t *fw, int argc, char *argv[], enum testfw_mode_t mode)
{
    return 0;
}
