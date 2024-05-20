#ifndef BUILD_H
#define BUILD_H

#include "command.h"
#include "parser.h"

// Couleurs pour l'invite de commande
#define PROMPT_GREEN "\001\033[32m\002"
#define PROMPT_CYAN "\001\033[36m\002"
#define PROMPT_RESET "\001\033[00m\002"
#define MAX_PROMPT_LENGTH 100

extern int last_command_return;
extern char *pwd;
extern int index_redirec;
extern char *line;
extern char *l;

// Fonction pour construire l'invite de commande
char *build_prompt();

// Fonction pour gérer la commande 'cd'
void build_cd(struct argv_t *arg);

// Fonction pour gérer la commande 'pwd'
void build_pwd();

// Fonction pour gérer la commande 'exit'
void build_exit(struct argv_t *arg);

// Fonction pour gérer la commande 'jobs'
void build_jobs();

// Fonction pour gérer la commande 'kill'
void build_kill(struct argv_t *arg);

// Fonction pour gérer la commande 'fg'
void build_fg(struct argv_t * arg);

// Fonction pour gérer la commande 'bg'
void build_bg(struct argv_t * arg);

// Fonction pour gérer la commande '?'
void build_interogation();

// Fonction pour gérer les commandes externes
void build_external(struct argv_t *arg);

// Fonction pour gérer le nettoyage de la mémoire
void build_clean(struct argv_t *arg, int);

// Fonction pour exécuter une commande pour pipe
void execute_command(struct argv_t *);

// Fonction pour gérer les pipes
void build_pipe(struct argv_t *, int n_pipes);

struct argv_t *build_substitution(char **, int *, int, char **);

#endif 