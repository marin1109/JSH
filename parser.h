#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// Structure représentant une commande et ses informations supplémentaires.
struct argv_t {
    char **data;
    int len;
    int esp;
    int nb_fifo;
    char **all_fifo;
};

// Divise une ligne de commande en mots et stocke les informations dans une structure argv_t.
// Gère également les caractères spéciaux comme '&'.
struct argv_t *split(char *);

// Compte et retourne le nombre de mots dans une ligne.
int nb_words(char *);

// Vérifie si une chaîne donnée correspond à un opérateur de redirection.
// Retourne 1 si c'est le cas, sinon 0.
int is_str_redirection(char *);

// Examine les arguments pour déterminer s'ils contiennent une redirection.
// Retourne l'index de la première redirection trouvée, sinon 0.
int is_redirection(char **, int);

// Retourne un numéro associé à une chaîne de redirection spécifique.
int which_redirection_str_is(char *);

// Identifie le type de redirection dans les arguments donnés.
// Retourne un numéro représentant le type de redirection.
int which_redirection(struct argv_t *);

// Compte et retourne le nombre total de redirections dans une ligne de commande.
int nb_direction(struct argv_t *);

int is_input_well_formed(struct argv_t *);

// Crée et retourne une nouvelle structure argv_t pour une commande, en excluant les opérateurs de redirection.
struct argv_t * data_cmd(struct argv_t *, int);

// Vérifie si la commande contient un processus de substitution.
int is_process_substitution(char **, int, int *, int *,int *, int *);

// Compte et retourne le nombre de pipes dans une ligne de commande.
int count_pipes(char **, int);

// Retourne une chaîne de caractères représentant la commande à exécuter.
char *get_cmd_pipe(char **, int);

// Divise une ligne de commande en commandes séparées par des pipes.
char **split_pipe(char **, int, int);

// Divise une ligne de commande en commandes séparées par des processus de substitution.
char **split_substitution(struct argv_t *);

char **split_without_first_substitution(char **, int *, int, int, char *);

#endif 