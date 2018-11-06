#define _GNU_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "testfw.h"

/* ********** STRUCTURES ********** */

struct testfw_t
{
    char * program; // filename executable 
    int timeout; 
    char *logfile; // rediger dans un fichier
    char *cmd; // rediriger dans une commande
    bool silent; // automatiser , tests par défaut ?
    bool verbose; // affichage détailler dans la console ( ie debug)

};

/* ********** FRAMEWORK ********** */
struct testfw_t *testfw_init(char *program, int timeout, char *logfile, char *cmd, bool silent, bool verbose)
{
   timeout = (timeout <0) ? 0:timeout;
   if(program==NULL){
       perror("program file incorrect");
       exit(TESTFW_EXIT_FAILURE);
   }

   
    struct testfw_t * new = (struct testfw_t *) malloc (sizeof(struct testfw_t));

    //Check new struct is correct
    if (new == NULL){
        perror("malloc error");
        exit(TESTFW_EXIT_FAILURE);
    }

    new->program = program;
    new->timeout = timeout;
    new->logfile = logfile;
    new->cmd = cmd;
    new->silent = silent;
    new->verbose = verbose;

    return new;


}

void testfw_free(struct testfw_t *fw)
{
    if(fw != NULL){
        free(fw);
        fw=NULL;
    }
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
