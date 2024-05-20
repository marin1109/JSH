#include "build.h"
#include "parser.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

char *build_prompt() {
    char *prompt = malloc(sizeof(char) * MAX_PROMPT_LENGTH);
    if (!prompt) {
        fprintf(stderr, "Error allocating memory\n");
        exit(1);
    }

    char *pwd = pwd_jsh();
    int nb_jobs = get_nb_jobs();

    snprintf(prompt, MAX_PROMPT_LENGTH, "%s[%d]%s%s%s%s$ ",
             PROMPT_GREEN, nb_jobs, PROMPT_CYAN,
             (strlen(pwd) > 26) ? "..." : "",
             (strlen(pwd) > 26) ? pwd + strlen(pwd) - 22 : pwd,
             PROMPT_RESET);

    free(pwd);
    return prompt;
}

void build_cd(struct argv_t *arg){
    if (arg->len == 1) {
        last_command_return = cd(NULL);
        if (last_command_return == 1) {
            fprintf(stderr, "No such file or directory\n");
        }
    } else {
        last_command_return = cd(arg->data[1]);
        if (last_command_return == 1) {
            fprintf(stderr, "bash: cd: %s: No such file or directory\n", arg->data[0]);
        }
    }
}

void build_pwd() {
    pwd = pwd_jsh();
    fprintf(stdout, "%s\n", pwd);
    last_command_return = (pwd == NULL) ? 1 : 0;
    free(pwd);
}

void build_exit(struct argv_t *arg) {
    if (!get_nb_jobs()) {
        if (index_redirec) {
            for(int i = 0 ; i < arg->nb_fifo ; ++i){
                free(arg->all_fifo[i]);
            }
            free(arg->all_fifo);
            free(arg->data);
            free(arg);
            free(line);
            // if(arg->nb_fifo < 2)
                free(l);
            exit_jsh(0);
        }
        if (arg->len == 1) {
            for(int i = 0 ; i < arg->nb_fifo ; ++i){
                free(arg->all_fifo[i]);
            }
            free(arg->all_fifo);
            free(arg->data);
            free(arg);
            free(line);
            // if(arg->nb_fifo < 2)
                free(l);
            exit_jsh(last_command_return);
        } else if (arg->len == 2) {
            int val_exit = atoi(arg->data[1]);
            for(int i = 0 ; i < arg->nb_fifo ; ++i){
                free(arg->all_fifo[i]);
            }
            free(arg->all_fifo);
            free(arg->data);
            free(arg);
            free(line);
            // if(arg->nb_fifo < 2)
                free(l);
            exit_jsh(val_exit);
        } else {
            fprintf(stderr, "exit has at most two arguments\n");
        }
    } else {
        fprintf(stderr, "There are still jobs running\n");
        last_command_return = 1;
    }
}

void build_jobs() {
    remove_jobs(0, -1);
    print_jobs();
}

void build_kill(struct argv_t *arg) {
    int sig = SIGTERM;
    if (arg->data[1][0] == '-') {
        int sig_nb = strtol(arg->data[1] + 1, NULL, 10);
        if (sig_nb < 65 && sig_nb > 0) {
            sig = atoi(arg->data[1] + 1);
        } else {
            fprintf(stderr, "-bash: kill: %s: invalid signal specification", arg->data[1] + 1);
        }
    } else if (arg->data[1][0] == '%') {
        if (kill_job(strtol(arg->data[1] + 1, NULL, 10), sig) == -1) {
            fprintf(stderr, "-bash: kill: %s: no such job\n", arg->data[1]);
        }
    } else {
        if (strtol(arg->data[1], NULL, 10) == 0 && strcmp(arg->data[1], "0") != 0) {
            fprintf(stderr, "-bash: kill: %s: arguments must be process or job IDs\n", arg->data[1]);
        }
        if (kill(strtol(arg->data[1], NULL, 10), sig) == -1) {
            fprintf(stderr, "-bash: kill: (%s) - No such process\n", arg->data[1]);
        }
    }
    for (int i = 2; i < arg->len; ++i) {
        if (arg->data[i][0] == '%') {
            if (kill_job(strtol(arg->data[i] + 1, NULL, 10), sig) == -1) {
                fprintf(stderr, "-bash: kill: %s: no such job\n", arg->data[i]);
            }
        } else if (kill(strtol(arg->data[i], NULL, 10), sig) == -1) {
            fprintf(stderr, "-bash: kill: (%s) - No such process\n", arg->data[i]);
        }
    }
}

void build_interogation() {
    fprintf(stdout, "%d\n", last_command_return);
    last_command_return = 0;
}

void execute_command(struct argv_t *arg){
    int is_after_redir = 0;
    int nb_redir = -1;
    int first_redir = -1;
    int fd_file = -2;
    int redir_error = 0;

    for (int i = 1; i < arg->len; ++i) {
        if (is_str_redirection(arg->data[i])) {
            is_after_redir = 1;
            nb_redir = which_redirection_str_is(arg->data[i]);
            if (first_redir == -1)
                first_redir = i;
        } else if (is_after_redir == 1) {
            if (nb_redir == 1) {
                fd_file = redirection(&last_command_return, arg->data[i], 0, O_RDONLY);
                dup2(fd_file, 0);
            } else if (nb_redir == 2 || nb_redir == 5) {
                int option = O_WRONLY | O_EXCL | O_CREAT;
                fd_file = redirection(&last_command_return, arg->data[i], 1, option);
                if (nb_redir == 2)
                    dup2(fd_file, 1);
                else
                    dup2(fd_file, 2);
            } else if (nb_redir == 3 || nb_redir == 6) {
                int option = O_WRONLY | O_CREAT | O_TRUNC;
                fd_file = redirection(&last_command_return, arg->data[i], 1, option);
                if (nb_redir == 3)
                    dup2(fd_file, 1);
                else
                    dup2(fd_file, 2);
            } else if (nb_redir == 4 || nb_redir == 7) {
                int option = O_WRONLY | O_CREAT | O_APPEND;
                fd_file = redirection(&last_command_return, arg->data[i], 1, option);
                if (nb_redir == 4)
                    dup2(fd_file, 1);
                else
                    dup2(fd_file, 2);
            }
            if (fd_file == -1)
                redir_error = 1;
        }
    }

    if (first_redir != -1)
        arg->data[first_redir] = NULL;

    if (redir_error == 0 && (arg->data[0][0] == '.' || arg->data[0][0] == '/')) {
        int r = execv(arg->data[0], arg->data);
        if (r == -1) {
            if (arg->esp == 0)
                fprintf(stderr, "Unknown command\n");
            else
                remove_jobs(0, -1);
        }
    } else if (redir_error == 0) {
        int r = execvp(arg->data[0], arg->data);
        if (r == -1) {
            if (arg->esp == 0)
                fprintf(stderr, "Unknown command\n");
            else
                remove_jobs(0, -1);
        }
    }
}

void build_external(struct argv_t *arg) {

    pid_t pids = fork();
    int status = 0;

    switch (pids) {
        case 0: {
            activate_sig();

            execute_command(arg);
            exit(1);
        }
        default: {
            add_job(pids, l, NULL, 0);
            if (arg->esp == 0) {
                tcsetpgrp(STDIN_FILENO, pids);
                tcsetpgrp(STDOUT_FILENO, pids);
                if (waitpid(pids, &status, WUNTRACED) != -1) {
                    if (!WIFSTOPPED(status)) {
                        remove_jobs(0, 1);
                    } else {
                        turn_to_background(pids);
                    }
                    tcsetpgrp(STDIN_FILENO, getpid());
                    tcsetpgrp(STDOUT_FILENO, getpid());
                }
            }
            


            if (WIFEXITED(status)) {
                last_command_return = WEXITSTATUS(status);
            } else {
                last_command_return = 1;
            }
            break;
        }
    }
}

void build_clean(struct argv_t *arg, int nb_fifo) {
    for(int i = 0 ; i < nb_fifo ; ++i){
        free(arg->all_fifo[i]);
    }
    free(arg->all_fifo);
    free(arg->data);
    free(arg);
    free(line);
    if(nb_fifo < 2)
        free(l);
}

void build_fg(struct argv_t * arg){
    do_fg(arg);
    last_command_return = 0;
    tcsetpgrp(STDIN_FILENO, getpid());
    tcsetpgrp(STDOUT_FILENO, getpid());
}

void build_bg(struct argv_t * arg){
    do_bg(arg);
    last_command_return = 0;
}

void build_pipe_aux(char **cmds, int n_pipes) {
    int pipefds[2 * n_pipes];

    for (int i = 0; i < n_pipes; i++) {
        if (pipe(pipefds + i*2) < 0) {
            perror("Erreur de création de pipe");
            exit(EXIT_FAILURE);
        }
    }


    int pid;
    for (int i = 0; i < n_pipes + 1; i++) {
        pid = fork();
        if (pid == 0) {
            if (i != 0) {
                dup2(pipefds[(i - 1) * 2], 0);
            }
            if (i != n_pipes) {
                dup2(pipefds[i * 2 + 1], 1);
            }
            for (int j = 0; j < 2 * n_pipes; j++) {
                close(pipefds[j]);
            }
            struct argv_t * arg = split(cmds[i]);
            execute_command(arg);
        } else if (pid < 0) {
            perror("Erreur fork");
            exit(EXIT_FAILURE);
        }else{
            // child_processus[i].id = pid;
            // child_processus[i].state = "Running";
        }
    }

    for (int i = 0; i < 2 * n_pipes; i++) {
        close(pipefds[i]);
    }

    for (int i = 0; i < n_pipes + 1; i++) {
        wait(NULL);
    }
}

void build_pipe(struct argv_t * arg, int n_pipes){
    pid_t pids = fork();
    int status = 0;

    // struct job *child_processus = malloc(sizeof(struct job) * n_pipes);
    switch (pids) {
        case 0: {
            activate_sig();
            char **cmd_pipe = split_pipe(arg->data, arg->len, n_pipes);
            
            // build_pipe_aux(cmd_pipe,n_pipes,child_processus);
            build_pipe_aux(cmd_pipe,n_pipes);
            for(int i = 0; i <= n_pipes; ++i){
                free(cmd_pipe[i]);
            }
            free(cmd_pipe);
            exit(1);
        }
        default: {
            // add_job(pids, l, child_processus, n_pipes);
            add_job(pids, l, NULL, n_pipes);
            if (arg->esp == 0) {
                tcsetpgrp(STDIN_FILENO, pids);
                tcsetpgrp(STDOUT_FILENO, pids);
                if (waitpid(pids, &status, WUNTRACED) != -1) {
                    if (!WIFSTOPPED(status)) {
                        remove_jobs(0, 1);
                    } else {
                        turn_to_background(pids);
                    }
                    tcsetpgrp(STDIN_FILENO, getpid());
                    tcsetpgrp(STDOUT_FILENO, getpid());
                }
            }

            break;
        }
    }
}

// void build_pipe(struct argv_t * arg, int n_pipes){
//     pid_t pids = fork();
//     int status = 0;

//     struct job *child_processus = malloc(sizeof(struct job) * n_pipes);

//     activate_sig();
//     //execute_command(arg);
//     char **cmd_pipe = split_pipe(arg->data, arg->len, n_pipes);
//     build_pipe_aux(cmd_pipe,n_pipes,child_processus);
//     for(int i = 0; i <= n_pipes; ++i){
//         free(cmd_pipe[i]);
//     }
//     free(cmd_pipe);
//     signaux();


//     add_job(pids, l, child_processus, n_pipes);
//     if (arg->esp == 0) {
//         tcsetpgrp(STDIN_FILENO, pids);
//         tcsetpgrp(STDOUT_FILENO, pids);
//         if (waitpid(pids, &status, WUNTRACED) != -1) {
//             if (!WIFSTOPPED(status)) {
//                 remove_jobs(0, 1);
//             } else {
//                 turn_to_background(pids);
//             }
//             tcsetpgrp(STDIN_FILENO, getpid());
//             tcsetpgrp(STDOUT_FILENO, getpid());
//         }
//     }

//     // if (WIFEXITED(status)) {
//     //     last_command_return = WEXITSTATUS(status);
//     // } else {
//     //     last_command_return = 1;
//     // }
// }

struct argv_t *build_substitution(char **data, int *len, int fifo_nb, char **all_fifo) {
    int start = 0;
    int end = 0;
    int start_space = 0;
    int end_space = 0;
    if(is_process_substitution(data, *len, &start, &start_space, &end, &end_space) == 1){

        char fifo_name[18];
        strcpy(fifo_name, "/tmp/substition");
        char a[2];
        a[0] = fifo_nb + '0';
        a[1] = '\0';
        
        strcat(fifo_name, a);

        if(mkfifo(fifo_name, 0664) == -1){
            perror("error create mkfifo");
            exit(1);
        }

        int r = fork();
        switch (r) {
            case -1: {
                perror("Erreur fork");
                exit(EXIT_FAILURE);
            }
            case 0: {

                int fd = open(fifo_name, O_WRONLY);
                if(fd == -1){
                    perror("erreur open");
                    exit(1);
                }
                dup2(fd, 1);
                close(fd);


                char **new_data;
                int new_len;
                if(start == end){
                    new_len = 1;
                    new_data = malloc(sizeof(char *));
                    new_data[0] = malloc(sizeof(char) * (strlen(data[start])-2));
                    strncpy(new_data[0], data[start]+2, strlen(data[start])-3);
                }
                else{
                    new_len = end - start + 1 - start_space - end_space;

                    new_data = malloc(sizeof(char *) * new_len);
                    if(!start_space){
                        new_data[0] = malloc(sizeof(char) * (strlen(data[start])-1));
                        strncpy(new_data[0], data[start]+2, strlen(data[start])-2);
                    }

                    for(int k = 1 ; k < new_len + 1 ; ++k){
                        new_data[k-1] = malloc(sizeof(char) * (strlen(data[start + k])+1));
                        strcpy(new_data[k-1], data[start + k]);
                    }
                }

                int n_pipes = count_pipes(new_data, new_len);

                struct argv_t *arg = malloc(sizeof(struct argv_t));
                arg->data = new_data;
                arg->len = new_len;
                arg->esp = 0;
                if (is_input_well_formed(arg) == 0){
                    return arg;
                }
                if (n_pipes > 0) {
                    build_pipe(arg, n_pipes);

                }
                else{
                    if (new_len != 0) {
                        struct argv_t *arg = malloc(sizeof(struct argv_t));
                        arg->data = new_data;
                        arg->len = new_len;
                        arg->esp = 0;
                        index_redirec = is_redirection(new_data, new_len);
                        if (strcmp(new_data[0], "cd") == 0) {
                            build_cd(arg);
                        } else if (strcmp(new_data[0], "pwd") == 0 && !index_redirec) {
                            build_pwd();
                        } else if (strcmp(new_data[0], "exit") == 0) {
                            build_exit(arg);
                        } else if (strcmp(new_data[0], "jobs") == 0) {
                            build_jobs(arg);
                        } else if (strcmp(new_data[0], "kill") == 0 && strcmp(new_data[1], "-l") != 0) {
                            build_kill(arg);
                        } else if (strcmp(new_data[0], "?") == 0) {
                            build_interogation();
                        }
                        else if (strcmp(new_data[0], "fg") == 0)
                        {
                            do_fg(arg);
                            tcsetpgrp(STDIN_FILENO, getpid());
                            tcsetpgrp(STDOUT_FILENO, getpid());
                        }
                        else if (strcmp(new_data[0], "bg") == 0)
                        {
                            do_bg(arg);
                        }
                        else
                        {
                            execute_command(arg);
                        }
                        free(arg);
                    }
                    else{
                        perror("Erreur new_len = 0");
                        exit(EXIT_FAILURE);
                    }
                }
                exit(0);
            }
            default: {
                
                all_fifo[fifo_nb] = malloc(18);
                strcpy(all_fifo[fifo_nb], fifo_name);
                return build_substitution(split_without_first_substitution(data, len, start, end, fifo_name), len, fifo_nb + 1, all_fifo);
                
            }
        }
        
    }
    else{
        struct argv_t *arg = malloc(sizeof(struct argv_t));
        arg->data = data;
        arg->len = *len;
        arg->all_fifo = all_fifo;
        arg->nb_fifo = fifo_nb;
        return arg;
    }

}