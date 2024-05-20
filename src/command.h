#ifndef COMMAND_H
#define COMMAND_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "parser.h"

#define EXIT_VAL 0
#define MAX_PATH_LENGTH 4096
#define MAX_JOBS 512

// Structure représentant un job dans jsh
struct job {
    int id;
    char *state;
    char *name;
    int foreground;
    struct job *all_processus;
    int nb_processus;
};

// Quitte le shell avec une valeur donnée
void exit_jsh(int val);

// Retourne le chemin du répertoire courant
char *pwd_jsh();

// Change le répertoire courant
int cd(const char *pathname);

// Gère la redirection des entrées/sorties
int redirection(int *last_return, char *file, int mode, int option);

// Ajoute un job à la liste des jobs
void add_job(int pid, char *name, struct job *, int);

// Retire un job de la liste des jobs
void remove_jobs(int need_to_print, pid_t pid);

// Affiche la liste des jobs
void print_jobs();

// Retourne le nombre de jobs
int get_nb_jobs();

// Initialise les signaux
void signaux();

// Active les signaux pour un processus
void activate_sig();

// Arrête un job
int kill_job(int n, int sig);

// Met un job en arrière plan
void turn_to_background(int pid);

void do_fg(struct argv_t *);

void do_fg(struct argv_t *);

#endif
