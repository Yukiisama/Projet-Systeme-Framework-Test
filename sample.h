#ifndef test_H
#define test_H

/**
 * @brief return success
 */
int test_success(int argc, char *argv[]);

/**
 * @brief return failure
 */
int test_failure(int argc, char *argv[]);

/**
 * @brief test segmentation fault signal
 */
int test_segfault(int argc, char *argv[]);

/**
 * @brief test alarm signal
 */
int test_alarm(int argc, char *argv[]);

/**
 * @brief test assert
 */
int test_assert(int argc, char *argv[]);

/**
 * @brief test sleep (timeout error)
 */
int test_sleep(int argc, char *argv[]);

/*
 * @brief test argv aruments
 */
int test_args(int argc, char *argv[]);

/**
 * @brief return success
 */
int othertest_success(int argc, char *argv[]);

/**
 * @brief return failure
 */
int othertest_failure(int argc, char *argv[]);

#endif