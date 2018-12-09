## BROCARD Thimotée GERARDIN Xavier PERIGNON Alexis

# Présentation du Projet
<div style='text-align: justify;'>
Le but de ce projet est de développer un framework de tests à partir de nos compétences en programmation système.
Le langage utilisé est donc le C.

Le projet est découpé en deux parties et un bonus.
La première partie (Partie A) consiste à l'implémentation de la structure qui contiendra nos tests ainsi que diverses options, les fonctions d'initialisation/allocation et de libération de la structure et les fonctions d'ajouts de tests.
La deuxième partie (Partie B) consiste elle à l'implémentation de la fonction permettant de lancer l’exécution de tous les tests enregistrés dans la structure.
Le bonus nous demandant des modifications de la partie B pour executer les tests en parallèle ou sans aucun ```fork```.


<div style="page-break-after: always;"></div>

# PARTIE A : ajout des tests

## Structure de test, allocation et libération de mémoire

Pour l'implémentation de la structure de test nous avons choisis en lisant les paramètres de la fonction d'initialisation fournie et en ajoutant des champs qu'on a jugé nécessaires.

  
```cpp
struct testfw_t
{
  char* program; 
  int timeout;
  char* logfile; 
  char* cmd; 
  bool silent; 
  bool verbose;
  
  struct test_t** tests;
  unsigned int nbTest; 
  unsigned int lenTests; 
};
```
En outre : 
 - Le champ ```program``` contient le nom de l’exécutable.
 - Le champ ```timeout``` contient le temps maximal accepté (en seconde) pour passer un test.
 - Le champ ```logfile``` contient le chemin vers un fichier ou les résultats des tests seront redirigés à la place de la sortie standard. 
 - Le champ ```cmd``` contient la commande qui sera exécuté.
 - Le champ ```silent``` sert à supprimer tout l'affichage (quand ```True```).
 - Le champ ```verbose``` sert à l'affichage d'information supplémentaire sur le fonctionnement du framework.
- Le champ ```tests``` est un tableau (dynamique) contenant les tests ajoutés.
- Le champ ```nbTest``` est le nombre de tests présents dans le tableau.
- Le champ ```lenTests``` est la taille actuelle du tableau afin de pouvoir le realloc quand sa capacité n'est plus suffisante.

/!\ ```logfile``` et ```timeout``` sont deux options exclusives on ne doit pas mettre les deux en même temps sous peine d'avoir un comportement indéfini. /!\
/!\ ```silent```et ```verbose``` sont "concurrent" si les deux sont mis a ```True```, ```silent``` supprime l'affichage et domine ```verbose```. /!\

---
Pour l'allocation de la mémoire et l'initialisation de la structure de tests (```testfw_init```)nous avons dû allouer manuellement : la structure en elle même, le tableau étant donné qu'on à choisi un tableau dynamique (```tests```) afin de ne pas avoir de capacité "finie" et les "cases" du tableau contenant la structure```test_t``` qui nous est fournie, on a ensuite juste à ajouter les paramètre dans les champs de notre structure.

Pour la libération de la mémoire (```testfw_free```) on doit libérer les cases de ```tests``` une à une pour éviter les fuites mémoires tout en ne dépassant pas le nombre de tests (```nbTest```) pour éviter les erreurs, il suffit ensuite de free le tableau puis la structure en dernier.

## Fonction d'ajout de tests

Avant de pouvoir implémenter les fonctions d'ajout de tests nous avons dû coder deux fonctions simples :
- ```testfw_length``` qui renvoie juste le nombre de test qu'on peut récupérer dans notre structure avec ```fw->nbTest```
- ```testfw_get``` qui renvoie un pointeur vers un test enregistré grâce à son indice mis en paramètre, il suffit d'aller le récupérer dans le tableau de la structure avec ```fw->tests[k]``` avec k l'indice du test voulu.


L'ajout de tests se fait a partir de trois fonctions différentes : 

 - ```testfw_register_func``` prend en paramètre ```suite``` (un préfixe au nom de la fonction, qui va servir à son "identification"), ```name``` (qui est le nom de la fonction) et un pointeur sur une fonction, elle va tout simplement l'enregistrer à la fin de notre tableau ```tests``` de la structure (et realloc de la place en cas de tableau saturé).
 - ```testfw_register_symb``` prend en paramètre ```suite``` et ```name``` , elle va charger dynamiquement l'exécutable (qui est notre champ ```program``` de la structure) avec ```dlopen``` puis chercher la fonction dont le nom est créé par la concaténation de ```suite_name``` avec ```dlsym```, elle appelle ensuite la fonction précédente qui va enregistrer la fonction chargé dynamiquement dans notre tableau.
 - ```test_fw_register_suite``` prend en paramètre ```suite```, elle va chercher dynamiquement dans l'exécutable toute les fonctions commençant par ```suite```, pour cela on met le résultat de la commande ```"nm --defined-only program | cut -d ' ' -f 3 | grep \"suite_\"``` (avec ```program``` le nom de notre exécutable dans la structure et ```suite``` la suite prise en paramètre) dans un file qu'on va parser ligne par ligne, chaque ligne va nous permettre d'appeler la fonction précédente.

On remarque que chaque fonction est dépendante de la précédente (```test_fw_register_suite``` a besoin de ```testfw_register_symb``` qui a besoin de ```testfw_register_func```) le moindre problème se répercute donc sur une autre fonction. Notre groupe a eu un problème avec ```test_fw_register_suite``` qui semblait bien enregistrer la fonction dans la boucle mais une fois sortie, seul la dernière fonction était enregistrer (et dans toute les case de notre tableau ```tests```) ce problème était causé par le fait qu'on mettait tout simplement le pointeur du nom du test directement dans le champ sans en faire de copie, donc à la fin de la boucle toute les cases pointaient vers le même nom causant ainsi notre problème. Nous avons résolu notre problème en créant un pointeur sur une chaîne de caractère ou on à copié le nom et en mettant ce nouveau pointeur à la place.  

<div style="page-break-after: always;"></div>

# PARTIE B : exécution des tests

La partie B traite de l’exécution des tests enregistrés avec les fonctions précédentes.\
Par soucis de lisibilité du code nous avons découpé la fonction qui lance tout les tests en 4 fonctions.\
```redirect_logfile``` qui va s'occuper de mettre en place la redirection dans le fichier voulu (c'est à dire dans le cas où le champ ```logfile``` de notre structure est initialisé).\

```redirect_cmd```qui va s'occuper de la redirection dans la commande voulu, cette fonction effectue une sauvegarde des sorties standard (```STDOUT``` et ```STDERR```) afin de pouvoir les récupérer une fois la redirection terminée.
Ces redirections se font a l'aide des appels système ```dup2```et la sauvegarde avec ```dup```.

```launch_test```qui va lancer le test en accédant à la case de ```tests``` correspondant à l'indice passé en paramètre, en outre il prend en paramètre ```argc``` et ```argv``` qui correspondent aux nombre d'argument et au tableau d'argument de la fonction à tester (à ne pas confondre avec ```argc``` et ```argv``` du ```main```, cette fonction setup également le ```timeout```pour la fonction qui va être lancer.

```launch_suite_test```qui est la fonctions qui s'occupe de lancer une "séquence" de tests, elle prend en paramètre ```argc```, ```argv```, ```start``` et ```end```, ces deux derniers correspondent aux indices du premier et du dernier test de la séquence à lancer. Cette fonction est surtout utile pour le mode FORKP qui va lancer les tests en parallele. Cette fonction commence par effectuer un ```sigaction``` qui va traiter le signal d'alarme du timeout, ensuite on vérifie si une redirection vers le logfile est demandée. Dans la boucle principale on lance les tests de allant de l'indice ```start``` a ```end``` à l'aide de la fonction précédente et nous recupérrons le temps d'éxecution a l'aide d'appels des fonctions ```gettimeofday``` avant et après le test. On récupère le signal de fin et le code retour qui vont nous indiqué si le test à réussi ou non, nous donnant également plus de précision (```TIMEOUT```, ```KILLED```, ```SUCCESS``` etc...), on affiche ensuite les résultats des tests sous la forme suivante ```[TermState] run test "suite.name" in time ms``` avec. Cette fonction renvoie le nombre de tests loupé dans la séquence lancée.

Le mode séquentiel de ```testfw_run_all``` se contente d'appeler la fonction ```launch_suite_test``` avec comme indice de debut 0 et comme indice de fin ```nbTest```afin de lancer tout les tests ajouté dans la structure.

Le principal problème que nous avons rencontré dans cette partie était lié à la redirection sur la commande externe. A l'origine, avant de résoudre notre problème la redirection sur commande empéchait CTest connaitre l'éxécution des tests mettant tout d'office a ```[FAILURE]``` avec comme message ```Failed  Required regular expression not found.Regex```.
Il s'agissait d'un problème de compréhension du sujet, donc pour le résoudre nous avons demander des précisions à des camarades et à des professeurs.

# BONUS : 

La première question du bonus était de faire le tourner les tests en parallèles. Pour cela nous avons juste rajouter a notre fonction ```testfw_run_all``` le cas où le parallèle était demandé (```FORKP```). Ce cas engendre autant de fork qu'il y à de tests, chaque processus fils effectue donc un test, il suffit donc à chaque tour de boucle (allant de ```0``` a ```nbTest```) de faire un ```fork``` puis d'appeler une fois notre fonction ```launch_suite_test``` dans le processus crée avec comme valeur de début de séquence la valeur de notre boucle, et celle de fin la valeur suivante.

La seconde question du bonus était de faire un mode ou aucun ```fork```n'était fait (mode ```NOFORK```) ;  Le mode ```NOFORK``` que nous avons implementé exécute les tests enregistrés jusqu'au premier échec. Il n'utilise aucun ```fork``` et utilise les mécanismes ```sigaction``` pour masquer les signaux importants ainsi que ```siglongjump``` et ```sigsetjump``` pour contourner les ```segfaults```, ```alarms``` etc... qui auraient tué notre programme. Nous avons compris du sujet que le mode devait s'arrêter au premier échec comme préciser dans le fichier Readme.md : ```In the nofork mode [...] the first test that fails will interrupt all the following```

<div style="page-break-after: always;"></div>

# ANNEXE : 

## Grille d'auto évaluation

|FONCTIONNALITÉS                                        |DIFFICULTÉ  |SCORE (/10)|
|:------------------------------------------------------|:-----------|:---------:|
|**Partie A :**                                         |            |           |
|- struct testfw_t                                      |+           |10         |
|- testfw_init / tesfw_free / testfw_get / testfw_length|+           |10         |
|- testfw_register_func                                 |+           |10         |
|- testfw_register_symb                                 |++          |10         |
|- testfw_register_suite                                |++          |10         |
|**Partie B :**                                         |            |           |
|- testfw_run_all en mode FORKS (sans les options)      |++          |10         |
|- option timeout                                       |++          |10         |
|- option redirection vers logfile                      |++          |10         |
|- option redirection vers commande externe             |+++         |10         |
|**Bonus :**                                            |            |           |
|- testfw_run_all en mode FORKP (avec les options)      |+++         |10         |
|- testfw_run_all en mode NOFORK (avec les options)     |+++         |10         |


</div>

