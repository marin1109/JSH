#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "parser.h"
#include "command.h"

char *last_path;

struct job *jobs[MAX_JOBS];
int jobs_nb_last = 0;
int jobs_nb = 0;

pid_t job_to_remove = 0;

void exit_jsh(int val) {
    exit(val);
}

char *pwd_jsh() {
    char *pwd = malloc(MAX_PATH_LENGTH * sizeof(char));
    if (pwd == NULL) {
        return NULL;
    }

    if (getcwd(pwd, MAX_PATH_LENGTH) == NULL) {
        return NULL;
    }

    return pwd;
}

int cd(const char *pathname) {
    char *pwd = pwd_jsh();
    char *new_last_path = malloc(sizeof(char) * (strlen(pwd) + 1));
    strcpy(new_last_path, pwd);
    free(pwd);

    if (pathname == NULL) {
        char *home = getenv("HOME");
        if (home == NULL || chdir(home) == -1) {
            free(new_last_path);
            return 1;
        }
        free(last_path);
        last_path = new_last_path;
        return 0;
    }

    if (strcmp(pathname, "-") == 0) {
        if (last_path != NULL) {
            if (chdir(last_path) == -1) {
                free(new_last_path);
                return 1;
            }
            free(last_path);
            last_path = new_last_path;
        }
        return 0;
    }

    if (chdir(pathname) == -1) {
        free(new_last_path);
        return 1;
    } else {
        free(last_path);
        last_path = new_last_path;
    }

    return 0;
}

int redirection(int *last_return, char *file, int mode, int option) {
    int fd_file;
    if (mode) {
        fd_file = open(file, option, 0664);
    } else {
        fd_file = open(file, option);
    }

    if (fd_file == -1) {
        if (errno == ENOENT) {
            fprintf(stderr, "%s: No Such File or Directory\n", file);
            *last_return = 1;
            return -1;
        }
        if (errno == EEXIST) {
            fprintf(stderr, "%s: File already exist\n", file);
            *last_return = 1;
            return -1;
        } else {
            fprintf(stdout, "Error open file\n");
            *last_return = 1;
            return -1;
        }
    }

    return fd_file;
}

void add_job(int pid, char *name, struct job *child_processus, int nb_processus) {
    if (jobs_nb_last == MAX_JOBS) {
        fprintf(stderr, "Too many jobs\n");
        return;
    }
    setpgid(pid, pid);
    jobs[jobs_nb_last] = malloc(sizeof(struct job));
    jobs[jobs_nb_last]->id = getpgid(pid);
    jobs[jobs_nb_last]->state = "Running";
    // jobs[jobs_nb_last]->all_processus = child_processus;
    // jobs[jobs_nb_last]->nb_processus = nb_processus;
    jobs[jobs_nb_last]->name = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(jobs[jobs_nb_last]->name, name);
    if (*(name + strlen(name) - 1) == '&') {
        jobs[jobs_nb_last]->foreground = 0;
        *(jobs[jobs_nb_last]->name + strlen(name) - 2) = '\0';
        fprintf(stderr, "[%d] %d  %s  %s\n", jobs_nb_last + 1, jobs[jobs_nb_last]->id, jobs[jobs_nb_last]->state, jobs[jobs_nb_last]->name);
    } else
        jobs[jobs_nb_last]->foreground = 1;
    
    ++jobs_nb_last;
    ++jobs_nb;
}

void remove_jobs(int need_to_print, pid_t p) {
    if (p == -1){
        for (int i = 0; i < jobs_nb_last; ++i) {
            int status = 0;
            if (jobs[i] != NULL) {
                if (waitpid(jobs[i]->id, &status, WNOHANG | WUNTRACED | WCONTINUED) > 0) {
                    if (WIFEXITED(status) || WIFSIGNALED(status) || WIFSTOPPED(status) || WIFCONTINUED(status)) {
                        if (WIFEXITED(status))
                            jobs[i]->state = "Done   ";
                        else if (WIFSTOPPED(status))
                            jobs[i]->state = "Stopped";
                        else if (WIFCONTINUED(status))
                            jobs[i]->state = "Running";
                        else
                            jobs[i]->state = "Killed ";
                        if (need_to_print) {
                            fprintf(stderr, "[%d] %d  %s  %s\n", i + 1, jobs[i]->id, jobs[i]->state, jobs[i]->name);
                        }
                    }
                }
            }
        }
    }

    int end = 1;

    if (need_to_print || p != -1) {
        int condition = 0;

        for (int i = jobs_nb_last - 1; i >= 0; --i) {
            if (jobs[i] != NULL) {
                if (need_to_print)
                    condition = strcmp(jobs[i]->state, "Done   ") == 0 || strcmp(jobs[i]->state, "Killed ") == 0;
                else {
                    condition = jobs[i]->foreground;
                }
                if (condition) {
                    free(jobs[i]->name);
                    free(jobs[i]);
                    jobs[i] = NULL;
                    if (end)
                        --jobs_nb_last;
                    //--jobs_nb;
                    if (jobs_nb > 0){
                        --jobs_nb;
                    }

                } else
                    end = 0;
            }
        }
    }
}

void turn_to_background(int pid) {
    for (int i = 0; i < jobs_nb_last; ++i) {
        if (jobs[i] != NULL) {
            if (jobs[i]->foreground == 1) {
                jobs[i]->foreground = 0;
                jobs[i]->state = "Stopped";
                fprintf(stderr, "[%d] %d  %s  %s\n", i + 1, jobs[i]->id, jobs[i]->state, jobs[i]->name);
            }
        }
    }
}

void print_jobs() {
    for (int i = 0; i < jobs_nb_last; ++i) {
        if (jobs[i] != NULL) {
            fprintf(stdout, "[%d] %d  %s  %s\n", i + 1, jobs[i]->id, jobs[i]->state, jobs[i]->name);
        }
    }
    int end = 1;

    for (int i = jobs_nb_last - 1; i >= 0; --i) {
        if (jobs[i] != NULL && (strcmp(jobs[i]->state, "Done   ") == 0 || strcmp(jobs[i]->state, "Killed ") == 0)) {
            free(jobs[i]->name);
            free(jobs[i]);
            jobs[i] = NULL;
            if (end)
                --jobs_nb_last;
            if (jobs_nb > 0){
                --jobs_nb;
            }
        } else
            end = 0;
    }
}

int get_nb_jobs() {
    return jobs_nb;
}

int kill_job(int n, int sig) {
    if (jobs[n - 1] == NULL)
        return -1;
    if (kill(-(jobs[n - 1]->id), sig) == -1)
        return -1;
    else {
        return 0;
    }
}

void fg(int num_job){
    if (num_job <= jobs_nb_last){
        if (jobs[num_job - 1] != NULL){
            jobs[num_job - 1]->foreground = 1;
            tcsetpgrp(STDIN_FILENO, jobs[num_job - 1]->id);
            tcsetpgrp(STDOUT_FILENO,jobs[num_job - 1]->id);
            int status;
            kill(-jobs[num_job - 1]->id, SIGCONT);
            if (jobs_nb > 0){
                --jobs_nb;
            }
            if (waitpid(jobs[num_job - 1]->id, &status, WUNTRACED) != -1){
                if (WIFEXITED(status)){
                    free(jobs[num_job - 1]->name);
                    jobs[num_job - 1] = NULL;
                    return;
                }
                if (WIFSTOPPED(status)){
                    jobs[num_job - 1]->foreground = 0;
                    jobs[num_job - 1]->state = "Stopped";
                    fprintf(stderr, "[%d] %d  %s  %s\n", num_job, jobs[num_job - 1]->id, jobs[num_job - 1]->state, jobs[num_job - 1]->name);
                    ++jobs_nb;
                    return;
                }
            }
        }
    } 
    else
        fprintf(stderr, "bg %d : the value of job is incorrect\n", num_job);
}

void do_fg(struct argv_t * arg){
    if (arg->len == 2){
        if (arg->data[1][0] == '%')
        {
            int num_job = atoi(arg->data[1] + 1);
            fg(num_job);
        }
        else
            fprintf(stderr, "%s %%job is the right syntax\n", arg->data[0]);
    }
    else
        fprintf(stderr, "%s have one arguments\n", arg->data[0]);
}

void do_bg(struct argv_t * arg)
{
    if (arg->len == 2){
        if (arg->data[1][0] == '%')
        {
            int num_job = atoi(arg->data[1] + 1);
            if (jobs[num_job - 1] != NULL){
                kill(jobs[num_job - 1]->id, SIGCONT);
                jobs[num_job - 1]->state = "Running";
            }
        }
        else
        {
            fprintf(stderr, "%s %%job is the right syntax\n", arg->data[0]);
        }
    }
    else
    {
        fprintf(stderr, "%s have one arguments\n", arg->data[0]);
    }
}

 void signaux()
{
    struct sigaction actINTbash, actTERMbash, actTSTPbash, actTTINbash, actQUITbash, actTTOUbash;

    memset(&actINTbash, 0, sizeof(actINTbash));
    memset(&actTERMbash, 0, sizeof(actTERMbash));
    memset(&actTSTPbash, 0, sizeof(actTSTPbash));
    memset(&actTTINbash, 0, sizeof(actTTINbash));
    memset(&actQUITbash, 0, sizeof(actQUITbash));
    memset(&actTTOUbash, 0, sizeof(actTTOUbash));

    actINTbash.sa_handler = SIG_IGN;
    actTERMbash.sa_handler = SIG_IGN;
    actTSTPbash.sa_handler = SIG_IGN;
    actTTINbash.sa_handler = SIG_IGN;
    actQUITbash.sa_handler = SIG_IGN;
    actTTOUbash.sa_handler = SIG_IGN;

    sigaction(SIGINT, &actINTbash, NULL);
    sigaction(SIGTERM, &actTERMbash, NULL);
    sigaction(SIGTSTP, &actTSTPbash, NULL);
    sigaction(SIGTTIN, &actTTINbash, NULL);
    sigaction(SIGQUIT, &actQUITbash, NULL);
    sigaction(SIGTTOU, &actTTOUbash, NULL);
}

void activate_sig() {
    struct sigaction actINTbash, actTERMbash, actTSTPbash, actTTINbash, actQUITbash, actTTOUbash, actKILLbash;

    memset(&actINTbash, 0, sizeof(actINTbash));
    memset(&actTERMbash, 0, sizeof(actTERMbash));
    memset(&actTSTPbash, 0, sizeof(actTSTPbash));
    memset(&actTTINbash, 0, sizeof(actTTINbash));
    memset(&actQUITbash, 0, sizeof(actQUITbash));
    memset(&actTTOUbash, 0, sizeof(actTTOUbash));
    memset(&actKILLbash, 0, sizeof(actKILLbash));

    actINTbash.sa_handler = SIG_DFL;
    actTERMbash.sa_handler = SIG_DFL;
    actTSTPbash.sa_handler = SIG_DFL;
    actTTINbash.sa_handler = SIG_DFL;
    actQUITbash.sa_handler = SIG_DFL;
    actTTOUbash.sa_handler = SIG_DFL;
    actKILLbash.sa_handler = SIG_DFL;

    sigaction(SIGINT, &actINTbash, NULL);
    sigaction(SIGTERM, &actTERMbash, NULL);
    sigaction(SIGTSTP, &actTSTPbash, NULL);
    sigaction(SIGTTIN, &actTTINbash, NULL);
    sigaction(SIGQUIT, &actQUITbash, NULL);
    sigaction(SIGTTOU, &actTTOUbash, NULL);
    sigaction(SIGKILL, &actKILLbash, NULL);
}
