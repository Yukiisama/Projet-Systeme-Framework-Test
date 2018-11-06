# Projet de Programmation Système

Il s'agit de développer un framework de tests, qui met en oeuvre vos compétences en programmation système. Ce framework est détaillé dans le fichier [README.md](README.md). Il s'appuie sur une API décrite dans le fichier [testfw.h](testfw.h). Votre travail consiste à implémenter cette API en complétant le fichier [testfw.c](testfw.c) qui vous est fourni. Le programme [sample_main.c](sample_main.c) montre un exemple simple d'utilisation de l'API avec quelques tests définis dans le fichier [sample.c](sample.c). En outre, le programme [testfw_main.c](testfw_main.c) illustre l'utilisation du framework de tests de manière exhaustive. Voici l'aide de cette commande :

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

Pour compiler et tester l'ensemble de votre projet, un fichier [CMakeLists.txt](CMakeLists.txt) vous est fourni.

```bash
$ mkdir build
$ cd build
$ cmake ..            # génération du Makefile
$ make                # compilation de projet
$ make test           # lancement des tests
```

Le projet est découpé en deux parties et fera l'objet d'un rendu intermédiaire.

## Partie A

* Lisez attentivement le fichier [README.md](README.md) et le fichier [testfw.h](testfw.h).
* Commençez par implementer la structure *testfw_t*, ainsi que les fonctions *testfw_init()* et *testfw_free()* qui permettent respectivement d'allouer cette structure et de la libérer. Cette structure contiendra les options générales du framework, ainsi que la liste des tests à exécuter. Ajoutez les fonctions *testfw_length()* et *testfw_get()*, qui permettent d'accéder à la liste des tests.
* Un test est défini par la structure *test_t*, qui contient simplement le nom du test, le nom de la suite de test et un pointeur sur la fonction de test, qui est de type *testfw_func_t* avec la même signature que la fonction *main()*. * * Implémentez la fonction *testfw_register_func()* qui permet dajouter un test dans le framework à partir d'une fonction C passé comme pointeur (ex. [sample_main.c](sample_main.c)).
* Pour enregistrer dynamiquement un test à partir d'un nom passé comme chaîne de caractères (i.e. nom de symbole), vous devez implementer la fonction *testfw_register_symb()*, qui utilisera quelques fonctions de la bibliothèques *dl* (*dlopen()*, *dlsym()*, *dlclose()*) afin de rechercher dans un executable le symbole d'un fonction de la forme "\<suite\>_\<name\>". On suppose donc qu'à l'édition de lien le code de cette fonction de test est connu.
* Pour implémenter la fonction *testfw_register_suite()*, qui découvre dynamiquement dans l'exécutable \<program\> toutes les fonctions tests préfixées par le nom de la suite (i.e. de la forme "\<suite\>_*"), le plus simple est d'utiliser la commande externe shell suivante (via un appel aux fonctions *popen()* et *pclose()*) :

```bash
$ nm --defined-only <program> | cut -d ' ' -f 3 | grep "^<suite>"
```

* Pour valider votre projet à ce niveau, vous devez pouvoir le compiler correctement le projet avec CMake et utiliser l'action -l du programme principal fourni dans la bibiothèque *testfw_main*. Par exemple :

```bash
$ ./sample -l
test.alarm
test.args
test.assert
test.failure
test.goodbye
test.hello
test.infiniteloop
test.segfault
test.sleep
test.success
```

## Partie B

* Implementez maintenant la fonction *testfw_run_all()*, qui permet de lancer l'exécution de tous les tests enregistrés. Cette fonction retourne le nombre de tests qui échouent ou zéro en cas de succès. En outre, elle affiche sur sa sortie standard (sauf en mode silent) le résultat des tests avec le format suivant, illustré par quelques tests fournis dans sample.c :

```text
[SUCCESS] run test "test.hello" in 0.47 ms (status 0)
[TIMEOUT] run test "test.infiniteloop" in 2000.19 ms (status 124)
[KILLED] run test "test.segfault" in 0.14 ms (signal "Segmentation fault")
[FAILURE] run test "test.failure" in 0.40 ms (status 1)
```

* Attention de bien respecter ce format de sortie ! Pour mesurer le temps d'exécution d'un test, on utilisera la fonction gettimeofday(). Pour afficher le nom d'un signal à partir de son numéro, vous pouvez utiliser *strsignal()*.
* Plusieurs modes d'éxécution sont possibles, mais on ne demandera d'implémenter que le mode TESTFW_FORKS, qui exécute tous les tests séquentiellement, c'est-à-dire les uns après les autres dans un processus fils (fork). En outre, on prendra en compte les options générales du framework :
  * *timeout* : la commande est interrompue si elle dépasse une limite de temps (et elle renvoie le status 124) ; 
  * *logfile* : la sortie standard du test (et sa sortie d'erreur) sont redirigées dans un fichier ;
  * *cmd* : la sortie standard du test (et sa sortie d'erreur) sont redirigés sur l'entrée standard d'une commande shell grâce aux fonctions popen/pclose (cf. man).
* A ce niveau, vous pouvez valider (partiellement) votre projet en exécutant les tests fourni avec CMake. Par exemple :

```bash
$ make test
Running tests...
      Start  1: test.success
 1/13 Test  #1: test.success .....................   Passed    0.00 sec
      Start  2: test.failure
 2/13 Test  #2: test.failure .....................   Passed    0.00 sec
      Start  3: test.segfault
 3/13 Test  #3: test.segfault ....................   Passed    0.00 sec
      Start  4: test.assert
 4/13 Test  #4: test.assert ......................   Passed    0.00 sec
      Start  5: test.sleep
 5/13 Test  #5: test.sleep .......................   Passed    2.00 sec
      Start  6: test.alarm
 6/13 Test  #6: test.alarm .......................   Passed    1.00 sec
      Start  7: test.args
 7/13 Test  #7: test.args ........................   Passed    0.00 sec
      Start  8: test.infiniteloop
 8/13 Test  #8: test.infiniteloop ................   Passed    2.00 sec
      Start  9: sample_all
 9/13 Test  #9: sample_all .......................   Passed    5.01 sec
      Start 10: sample_main
10/13 Test #10: sample_main ......................   Passed    0.01 sec
      Start 11: hello
11/13 Test #11: hello ............................   Passed    0.01 sec
      Start 12: hello_grep
12/13 Test #12: hello_grep .......................   Passed    0.00 sec
      Start 13: hello_diff
13/13 Test #13: hello_diff .......................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 13

Total Test time (real) =  10.05 sec
```

## Bonus

* En bonus, vous pouvez implémenter le mode TESTFW_FORKP qui exécute tous les tests en parallèle, c'est-à-dire concurrement dans plusieurs processus fils que l'on attendra tous à la fin... Une solution simple consiste à appeler le code de TESTFW_FORKS dans un nouveau processus fils, c'est-à-dire à faire un fork de fork !
* En super-bonus, vous pouvez implémenter le mode TESTFW_NOFORK, qui exécute les tests sans aucun fork...

