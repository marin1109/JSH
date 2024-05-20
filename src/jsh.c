#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include "parser.h"
#include "command.h"
#include "build.h"

char *pwd;
int last_command_return = 0;
int index_redirec;
char *line;
char *l;

int main(int argc, char *argv[], char *envp[]) {
    signaux();

    struct argv_t *arg;
    rl_outstream = stderr;

    while (1) {
        
        remove_jobs(1, -1);
        
        char *prompt = build_prompt();

        line = readline(prompt);

        free(prompt);

        if (line == NULL) {
            exit_jsh(last_command_return);
        }

        l = malloc(sizeof(char) * (strlen(line) + 1));
        strcpy(l, line);
        add_history(l);


        arg = split(line);


        int new_len = arg->len;

        struct argv_t *new_arg = build_substitution(arg->data, &new_len, 0, arg->all_fifo);
        new_arg->esp = 0;


        new_arg->len = new_len;
        new_arg->esp = arg->esp;
 
        int n_pipes = count_pipes(new_arg->data, new_arg->len);


        if (is_input_well_formed(arg) == 0){
            fprintf(stderr, "error syntax\n");
        }
        else{
            if (n_pipes > 0) {
                build_pipe(new_arg, n_pipes);
            }
            else{
                if (new_len != 0) {
                    index_redirec = is_redirection(new_arg->data, new_len);
                    if (strcmp(new_arg->data[0], "cd") == 0) {
                        build_cd(new_arg);
                    } else if (strcmp(new_arg->data[0], "pwd") == 0 && !index_redirec) {
                        build_pwd();
                    } else if (strcmp(new_arg->data[0], "exit") == 0) {
                        build_exit(new_arg);
                    } else if (strcmp(new_arg->data[0], "jobs") == 0) {
                        build_jobs(new_arg);
                    } else if (strcmp(new_arg->data[0], "kill") == 0 && strcmp(new_arg->data[1], "-l") != 0) {
                        build_kill(new_arg);
                    } else if (strcmp(new_arg->data[0], "?") == 0) {
                        build_interogation();
                    }
                    else if (strcmp(new_arg->data[0], "fg") == 0)
                    {
                        build_fg(new_arg);
                    }
                    else if (strcmp(new_arg->data[0], "bg") == 0)
                    {
                        build_bg(new_arg);
                    }
                    else
                    {
                        
                        build_external(new_arg);
                    }
                }
            }
        }
        // if(!arg->esp){
            for(int i = 0 ; i < new_arg->nb_fifo ; ++i){
                remove(new_arg->all_fifo[i]);
            }
        // }
        
        build_clean(arg, new_arg->nb_fifo);

        free(new_arg);

    }
    return 0;
}
