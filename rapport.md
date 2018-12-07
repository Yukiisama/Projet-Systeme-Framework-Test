
# Présentation du Projet

Le but de ce projet est de développer un framework de tests à partir de nos compétences en programmation système.
Le langage utilisé est donc le C.

Le projet est découpé en deux parties.
La première partie (Partie A) consiste à l'implémentation de la structure qui contiendra nos tests ainsi que diverses options, les fonctions d'initialisation/allocation et de libération de la structure et les fonctions d'ajouts de tests.
La deuxième partie (Partie B) consiste elle à l'implémentation de la fonction permettant de lancer l’exécution de tous les tests enregistrés dans la structure.


# Partie A : ajout des tests

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
- Le champ ```lenTest``` est la taille actuelle du tableau afin de pouvoir le realloc quand sa capacité n'est plus suffisante.

/!\ ```logfile``` et ```timeout``` sont deux options exclusives on ne doit pas mettre les deux en même temps sous peine d'avoir un comportement indéfini. /!\
/!\ ```silent```et ```verbose``` sont "concurrent" si les deux sont mis a ```True```, ```silent``` supprime l'affichage et domine ```verbose```. /!\

---
Pour l'allocation de la mémoire et l'initialisation de la structure de tests (```testfw_init```)nous avons dû allouer manuellement : la structure en elle même, le tableau étant donné qu'on à choisi un tableau dynamique (```tests```) afin de ne pas avoir de capacité "finie" et les "cases" du tableau contenant la structure```test_t``` qui nous est fournie, on a ensuite juste à ajouter les paramètre dans les champs de notre structure.

Pour la libération de la mémoire (```testfw_free```) on doit libérer les cases de ```tests``` une à une pour éviter les fuites mémoires tout en ne dépassant pas le nombre de tests (```nbTest```) pour éviter les erreurs, il suffit ensuite de free le tableau puis la structure en dernier.

## Fonction d'ajout de tests

Avant de pouvoir implémenter les fonctions d'ajout de tests nous avons du coder deux fonctions simples :
- ```testfw_length``` qui renvoie juste le nombre de test qu'on peut récupérer dans notre structure avec ```fw->nbTest```
- ```testfw_get``` qui renvoie un pointeur vers un test enregistré grâce à son indice mis en paramètre, il suffit d'aller le récupérer dans le tableau de la structure avec ```fw->tests[k]``` avec k l'indice du test voulu.


L'ajout de tests se fait a partir de trois fonctions différentes : 

 - ```testfw_register_func``` prend en paramètre ```suite``` (un préfixe au nom de la fonction, qui va servir à son "identification"), ```name``` (qui est le nom de la fonction) et un pointeur sur une fonction, elle va tout simplement l'enregistrer à la fin de notre tableau ```tests``` de la structure (et realloc de la place en cas de tableau saturé).
 - ```testfw_register_symb``` prend en paramètre ```suite``` et ```name``` , elle va charger dynamiquement l'exécutable (qui est notre champ ```program``` de la structure) avec ```dlopen``` puis chercher la fonction dont le nom est créé par la concaténation de ```suite_name``` avec ```dlsym```, elle appelle ensuite la fonction précédente qui va enregistrer la fonction chargé dynamiquement dans notre tableau.
 - ```test_fw_register_suite``` prend en paramètre ```suite```, elle va chercher dynamiquement dans l'exécutable toute les fonctions commençant par ```suite```, pour cela on met le résultat de la commande ```"nm --defined-only program | cut -d ' ' -f 3 | grep \"suite_\"``` (avec ```program``` le nom de notre exécutable dans la structure et ```suite``` la suite prise en paramètre) dans un file qu'on va parser ligne par ligne, chaque ligne va nous permettre d'appeler la fonction précédente.

On remarque que chaque fonction est dépendante de la précédente (```test_fw_register_suite``` a besoin de ```testfw_register_symb``` qui a besoin de ```testfw_register_func```) le moindre problème se répercute donc sur une autre fonction. Notre groupe a eu un problème avec ```test_fw_register_suite``` qui semblait bien enregistrer la fonction dans la boucle mais une fois sortie, seul la dernière fonction était enregistrer (et dans toute les case de notre tableau ```tests```) ce problème était causé par le fait qu'on mettait tout simplement le pointeur du nom du test directement dans le champ sans en faire de copie, donc à la fin de la boucle toute les cases pointaient vers le même nom causant ainsi notre problème. Nous avons résolu notre problème en créant un pointeur sur une chaîne de caractère ou on à copié le nom et en mettant ce nouveau pointeur à la place.  

# Partie B : exécution des tests

La partie B traite de l’exécution des tests enregistrés avec les fonctions précédentes.\
Par soucis de lisibilité du code nous avons découpé la fonction qui lance tout les tests en 3 fonctions.\
```redirect_logfile``` qui va s'occuper de mettre en place la redirection dans le fichier voulu (c'est à dire dans le cas où le champ ```logfile``` de notre structure est initialisé)\
```redirect_cmd```qui va s'occuper de la redirection dans la commande voulu, cette fonction effectue une sauvegarde des sorties standard (```STDOUT``` et ```STDERR```) afin de pouvoir les récupérer une fois la redirection terminée.
Ces redirections se font a l'aide des appels système ```dup2```et la sauvegarde avec ```dup```.\
```launch_test```qui va lancer le test en accédant à la case de ```tests``` correspondant à l'indice passé en paramètre, en outre il prend également
