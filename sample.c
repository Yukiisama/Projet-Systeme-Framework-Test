#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "sample.h"

int test_success(int argc, char *argv[])
{
    return EXIT_SUCCESS;
}

int test_failure(int argc, char *argv[])
{
    return EXIT_FAILURE;
}

int test_segfault(int argc, char *argv[])
{
    *((int *)0) = 0; // segfault!
    return EXIT_SUCCESS;
}

int test_alarm(int argc, char *argv[])
{
    alarm(1);
    pause(); // wait signal
    return EXIT_SUCCESS;
}

int test_assert(int argc, char *argv[])
{
    assert(1 == 2);
    return EXIT_SUCCESS;
}

int test_sleep(int argc, char *argv[])
{
    sleep(4); // in seconds
    return EXIT_SUCCESS;
}

int test_infiniteloop(int argc, char *argv[])
{
    while (1)
        ;
    return EXIT_SUCCESS;
}

int test_args(int argc, char *argv[])
{
    printf("argc: %d, ", argc);
    printf("argv: ");
    for (int i = 0; i < argc; i++)
        printf("%s ", argv[i]);
    printf("\n");
    return EXIT_SUCCESS;
}

int othertest_success(int argc, char *argv[])
{
    return EXIT_SUCCESS;
}

int othertest_failure(int argc, char *argv[])
{
    return EXIT_FAILURE;
}

int test_hello(int argc, char *argv[])
{
    int n = 10;
    if (argc == 1)
        n = atoi(argv[0]);
    for (int i = 0; i < n; i++)
        printf("hello world!\n");
    return EXIT_SUCCESS;
}

int test_goodbye(int argc, char *argv[])
{
    int n = 10;
    if (argc == 1)
        n = atoi(argv[0]);
    for (int i = 0; i < n; i++)
        printf("goodbye!!\n");
    return EXIT_SUCCESS;
}
