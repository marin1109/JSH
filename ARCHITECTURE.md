# Architecture du Projet "jsh"

## Introduction

Le projet "jsh" consiste en la réalisation d'un interpréteur de commandes interactif, un shell, avec la gestion des tâches (job control). Ce fichier explique l'architecture logicielle, les structures de données et les algorithmes implémentés dans le cadre de ce projet.

## Structure du Code Source

Le code source du projet est organisé de manière à regrouper les fonctionnalités liées à la gestion des jobs, des commandes externes et internes, des redirections, etc.

- `jsh.c`: Contient la boucle principale d'interaction avec l'utilisateur et la gestion générale du shell.
- `build.c`: Contient les fonctions de gestions les commandes saisies par l'utilisateur.
- `command.c`: Gère l'exécution des commandes internes et des jobs.
- `parser.c`: Gère l'analyse de la ligne de commande et la détection des redirections.

## Structures de Données

Les structures de données clés utilisées dans le projet incluent :
- `Job`: Représente un job avec son numéro, son état, son nom et s'il est en avant-plan ou en arrière-plan.
- La Structure `Job` est la suivante : 
```c
struct job {
    int id;
    char *state;
    char *name;
    int foreground;
};
```

`jsh` peut avoir au maximun 512 jobs à la fois.

- `argv_t`: Représente une liste d'arguments, dont on dispose des informations sur sa taille et ses éléments et si la commande est doit être lancée en avant-plan ou en arrière-plan.
- La structure ```argv_t``` est : 
```c
struct argv_t {
    char **data;
    int len;
    int esp;
};
```

## Algorithmes

### Job Control
L'algorithme de job control gère la création, la surveillance et la manipulation des jobs. Il permet de basculer entre l'avant-plan et l'arrière-plan, d'afficher les jobs en cours d'exécution et de les supprimer.

### Exécution des Commandes
L'algorithme d'exécution des commandes gère le lancement des commandes externes et internes. Il permet de lancer des commandes en avant-plan ou en arrière-plan, de gérer les commandes internes et de lancer des commandes externes en créant des processus fils.

### Redirections
L'algorithme de gestion des redirections analyse la ligne de commande pour détecter les opérations de redirection (<, >, >|, >>, 2>, 2>|, 2>>) et redirige les flux standard en conséquence.

## Conclusion

Ce fichier a présenté l'architecture logicielle, les structures de données et les algorithmes implémentés dans le cadre du projet "jsh". Le code source est organisé de manière à regrouper les fonctionnalités liées à la gestion des jobs, des commandes externes et internes et des redirections.
