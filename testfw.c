#define _GNU_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "testfw.h"

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
    if ( fw != NULL ) {
        for (int i = 0; i < fw->lenTests; i++ ) {
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
