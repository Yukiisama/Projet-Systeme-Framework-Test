# A Simple Test Framework (TestFW)

A simple test framework for language C, inspired by [Check](https://libcheck.github.io/check/) & [Google Test](https://github.com/google/googletest) & [CTest](https://gitlab.kitware.com/cmake/community/wikis/doc/ctest/Testing-With-CTest).

The main features are:

* test framework for C/C++ projects
* a simple framework written in C, with few files to include in your project
* framework developed on Linux system based on POSIX standard, as far as possible
* easy way to integrate your tests, just by adding some *test_\*()* functions
* support test with *argv* arguments
* all tests are executed sequentially (one by one)
* different execution modes: sequential fork, parallel fork or no fork

## Compilation

Our test framework is made up of two libraries:

* *libtestfw.a*: all the basic routines to discover, register and run tests (see API in [testfw.h](testfw.h)).
* *libtestfw_main.a*: a *main()* routine to launch tests easily (optionnal).

You can compile it by hand quite easily.

```bash
gcc -std=c99 -Wall -g   -c -o testfw.o testfw.c
ar rcs libtestfw.a testfw.o
gcc -std=c99 -Wall -g   -c -o testfw_main.o testfw_main.c
ar rcs libtestfw_main.a testfw_main.o
```

Or if you prefer, you can use the CMake build system ([CMakeLists.txt](CMakeLists.txt)).

```bash
mkdir build ; cd build
cmake .. && make
```

## Writing a First Test

Adding a test *hello* in a suite *test* (the default one) is really simple, you just need to write a function *test_hello()*
with the following signature ([hello.c](hello.c)).

```c
#include <stdio.h>
#include <stdlib.h>

int test_hello(int argc, char* argv[])
{
    printf("hello world\n");
    return EXIT_SUCCESS;
}
```

A success test should allways return EXIT_SUCCESS. All other cases are considered as a failure. More precisely, running a test returns one of the following status:

* SUCCESS: return EXIT_SUCCESS or 0 (normal exit)
* FAILURE: return EXIT_FAILURE or 1 (or any value different of EXIT_SUCCESS)
* KILLED: killed by any signal (SIGSEGV, SIGABRT, ...)
* TIMEOUT: after a time limit, return an exit status of 124 (following the convention used in *timeout* command)

Compile it and run it.

```bash
$ gcc -std=c99 -Wall -g -c hello.c
$ gcc hello.o -o hello -rdynamic -ltestfw_main -ltestfw -ldl -L.
$ ./hello
hello world
[SUCCESS] run test "test.hello" in 0.52 ms (status 0, wstatus 0)
=> 100% tests passed, 0 tests failed out of 1
```

And that's all!

## Running Tests

Let's consider the code [sample.c](sample.c). To run all this tests, you need first to compile it and then to link it against our both libraries.

```bash
gcc -std=c99 -Wall -g -c sample.c
gcc sample.o -o sample -rdynamic -ltestfw_main -ltestfw -ldl -L.
```

The '-rdynamic' option is required to load all symbols in the dynamic symbol table (ELF linker).

### Usage

Then, launching the main routine provide you some helpful commands to run your tests. Usage:

```text
Simple Test Framework (version 0.2)
Usage: ./sample [options] [actions] [-- <testargs> ...]
Register Options:
  -r <suite.name>: register a function "suite_name()" as a test
  -R <suite>: register all functions "suite_*()" as a test suite
Actions:
  -x: execute all registered tests (default action)
  -l: list all registered tests
Execution Options:
  -m <mode>: set execution mode: "forks"|"forkp"|"nofork" [default "forks"]
  -d <file>: compare test output with an expected file (using diff)
  -g <pattern>: search for a pattern in test output (using grep)
Other Options:
  -o <logfile>: redirect test output to a log file
  -O: redirect test stdout & stderr to /dev/null
  -t <timeout>: set time limits for each test (in sec.) [default 2]
  -T: no timeout
  -c: return the total number of test failures
  -s: silent mode (framework only)
  -S: full silent mode (both framework and test output)
  -v: verbose mode
  -h: print this help message
```

### List registred tests

List all available tests in the default suite (named *test*):

```bash
$ ./sample -l
test.alarm
test.args
test.assert
test.failure
test.infiniteloop
test.segfault
test.sleep
test.success
```

To use another suite, use '-r/-R' options:

```bash
$ ./sample -R othertest -l
othertest.failure
othertest.success
```

### Run a test suite

Run your tests with some options (timeout = 2 seconds, log file = /dev/null).  By default, these tests are launched sequentially (one by one) in a forked process (mode *forks*).

```bash
$ ./sample -t 2 -O -x
[KILLED] run test "test.alarm" in 1000.56 ms (signal "Alarm clock")
[SUCCESS] run test "test.args" in 0.53 ms (status 0)
[KILLED] run test "test.assert" in 0.49 ms (signal "Aborted")
[FAILURE] run test "test.failure" in 0.40 ms (status 1)
[SUCCESS] run test "test.goodbye" in 0.52 ms (status 0)
[SUCCESS] run test "test.hello" in 0.47 ms (status 0)
[TIMEOUT] run test "test.infiniteloop" in 2000.19 ms (status 124)
[KILLED] run test "test.segfault" in 0.14 ms (signal "Segmentation fault")
[TIMEOUT] run test "test.sleep" in 2000.33 ms (status 124)
[SUCCESS] run test "test.success" in 0.42 ms (status 0)
=> 40% tests passed, 6 tests failed out of 10
```

If you prefer to run all tests in parallel (i.e. in concurrent processes), you can use the *forkp* mode. It will probably run faster, at the risk that the test outputs will be interleaved.

```bash
$ ./sample -O -t 2 -m forkp
[SUCCESS] run test "test.args" in 0.17 ms (status 0)
[FAILURE] run test "test.failure" in 0.27 ms (status 1)
[SUCCESS] run test "test.goodbye" in 0.30 ms (status 0)
[SUCCESS] run test "test.success" in 0.18 ms (status 0)
[KILLED] run test "test.assert" in 0.31 ms (signal "Aborted")
[SUCCESS] run test "test.hello" in 0.31 ms (status 0)
[KILLED] run test "test.segfault" in 0.46 ms (signal "Segmentation fault")
[KILLED] run test "test.alarm" in 1000.29 ms (signal "Alarm clock")
[TIMEOUT] run test "test.infiniteloop" in 2000.23 ms (status 124)
[TIMEOUT] run test "test.sleep" in 2000.15 ms (status 124)
=> 40% tests passed, 6 tests failed out of 10
```

### Run a single test

Let's run a *single test* instead of a *test suite* as follow:

```bash
$ ./sample -m forks -r test.failure -x
[FAILURE] run test "test.failure" in 0.43 ms (status 1)
=> 0% tests passed, 1 tests failed out of 1
$ echo $?
0
```

As already explained, the *forks* mode starts each test separately in a forked process. The failure of a test will not affect the execution of the following tests.

Now, let's run a single test in *nofork* mode:

```bash
$ ./sample -m nofork  -r test.failure -x
[FAILURE] run test "test.failure" in 0.01 ms (status 1)
=> 0% tests passed, 1 tests failed out of 1
$ echo $?
1
```

### Using TestFW with CMake

In the *nofork* mode, each test is runned *directly* as a function call (without fork). As a consequence, the first test that fails will interrupt all the following.  It is especially useful when running all tests one by one within another test framework as CTest. See [CMakeLists.txt](CMakeLists.txt).

And running tests.

```bash
cmake . && make && make test
```

You can also pass arguments à la *argv* s follows.

```bash
$ ./sample -r test.args -- a b c
argc: 3, argv: a b c
[SUCCESS] run test "test.args" in 0.45 ms (status 0, wstatus 0)
=> 100% tests passed, 0 tests failed out of 1
```

## Apply an external command to test output

The TestFW API allows the execution of an external command (e.g. diff, grep) using the classic Unix *pipe* mechanism. The *testfw_main* library provides to useful options (-g and -d) based on this mechanism. In this case, the return status will be the status of the test it self if it fails, else the status of the external command applied.

Let's consider the [hello.c](hello.c) sample, that just prints "hello world".

```bash
$ ./hello
hello world
[SUCCESS] run test "test.hello" in 0.53 ms (status 0)
=> 100% tests passed, 0 tests failed out of 1
```

Let's search for some patterns in the test output (using *grep* command).

```bash
$ ./hello -g "world"
hello world
[SUCCESS] run test "test.hello" in 0.53 ms (status 0)
=> 100% tests passed, 0 tests failed out of 1

$  ./hello -g "helo"
[FAILURE] run test "test.hello" in 0.57 ms (status 1)
=> 0% tests passed, 1 tests failed out of 1
```

If you want to compare line by line your test output with an expected output, you can use the -d option (based on *diff* command).

```bash
$ echo "hello world" > hello.expected
$ ./hello -d hello.expected
[SUCCESS] run test "test.hello" in 0.57 ms (status 0)
=> 100% tests passed, 0 tests failed out of 1

$ ./hello -d goodbye.expected
1c1
< goodbye
---
> hello world
[FAILURE] run test "test.hello" in 0.17 ms (status 1)
=> 0% tests passed, 1 tests failed out of 1
```

## Writing your own Main Routine

A *main()* routine is already provided for convenience in the *libtestfw_main.a* library, but it could be useful in certain case to write your own *main()* routine based on the [testfw.h](testfw.h) API. See [sample_main.c](sample_main.c).

```c
#include <stdlib.h>
#include <stdbool.h>
#include "testfw.h"
#include "sample.h"

#define TIMEOUT 2
#define LOGFILE "test.log"
#define SILENT false

int main(int argc, char *argv[])
{
    struct testfw_t *fw = testfw_init(argv[0], TIMEOUT, LOGFILE, SILENT);
    testfw_register_func(fw, "test", "success", test_success);
    testfw_register_symb(fw, "test", "failure");
    testfw_register_suite(fw, "othertest");
    testfw_run_all(fw, argc - 1, argv + 1, TESTFW_FORK);
    testfw_free(fw);
    return EXIT_SUCCESS;
}
```

Compiling and running this test will produce the following results.

```bash
$ gcc -std=c99 -rdynamic -Wall sample.c sample_main.c -o sample_main -ltestfw -ldl -L.
$ ./sample_main
[SUCCESS] run test "test.success" in 0.24 ms (status 0)
[FAILURE] run test "test.failure" in 0.29 ms (status 1)
[FAILURE] run test "othertest.failure" in 0.09 ms (status 1)
[SUCCESS] run test "othertest.success" in 0.17 ms (status 0)
=> 50% tests passed, 2 tests failed out of 4
```

---

aurelien.esnard@u-bordeaux.fr
