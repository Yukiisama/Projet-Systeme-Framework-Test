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
   if (fw == NULL || fw->tests ==NULL){
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
   fw->tests[fw->nbTest]->suite = suite;
   fw->tests[fw->nbTest]->name = name;
   fw->tests[fw->nbTest]->func = func;
   fw->nbTest +=1;
   //printf(" %s | %s \n", suite , name);
   return fw->tests[fw->nbTest-1];
}

struct test_t *testfw_register_symb(struct testfw_t *fw, char *suite, char *name)
{
    if (fw == NULL || fw->tests ==NULL){
       perror("invalid struc");
       exit(EXIT_FAILURE);
    }
    char suitename[200];
    sprintf(suitename,"%s_%s",suite,name);
    //printf("%s\n",suitename); // for debugging

    void * handle = dlopen(fw->program,RTLD_LAZY);
    testfw_func_t func;
    * (void **)(&func) = dlsym(handle,suitename);
    //dlclose(handle);

    return testfw_register_func(fw,suite,name,func);



}

int testfw_register_suite(struct testfw_t *fw, char *suite)
{
    if ( fw == NULL || fw->tests == NULL){
        perror("invalid struct");
        exit(EXIT_FAILURE);
    }

    char command[100];

    sprintf(command,"nm --defined-only %s | cut -d ' ' -f 3 | grep \"^%s\"",fw->program , suite);
    fprintf(stderr, "%s\n",command);
    FILE * f = popen(command, "r");
    
    char  tab[200]; int i = 0;
    //TODO
    while(fgets(tab,200,f)!=NULL){

        char *tab2 = strchr (tab, '_')+1;
        tab2[strlen(tab2)-1]='\0';
        //strtok (tab2, '\n');
      //  printf("%s \n" ,tab2);
        testfw_register_symb(fw, suite, tab2);
        struct test_t *lol = testfw_get(fw, i);
        printf("%s_%s\n", lol->suite, lol->name);
        i++;
        }
    
    pclose(f);
    return i;
}

/* ********** RUN TEST ********** */

int testfw_run_all(struct testfw_t *fw, int argc, char *argv[], enum testfw_mode_t mode)
{
    return 0;
}
