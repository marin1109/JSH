#include "parser.h"

struct argv_t *split(char *line) {
    if (line == NULL)
        return NULL;

    struct argv_t *tab_data = malloc(sizeof(struct argv_t));
    if (tab_data == NULL) {
        fprintf(stderr, "error with malloc for struct argv_t\n");
        exit(1);
    }
    tab_data->esp = 0;
    tab_data->nb_fifo = 0;
    int nb_word = nb_words(line);

    char **data;
    int index = 0;
    if (nb_word != 0) {
        data = malloc(sizeof(char *) * (nb_word + 1));

        if (data == NULL) {
            fprintf(stderr, "error with malloc in split\n");
            exit(1);
        }

        char *word = strtok(line, " ");

        while (word != NULL) {
            if (strcmp(word, "&") == 0) {
                ++tab_data->esp;
                word = strtok(NULL, " ");
                --nb_word;
            } else {
                data[index] = word;
                word = strtok(NULL, " ");
                ++index;
            }
        }

        data[nb_word] = NULL;
        tab_data->data = data;
        tab_data->len = nb_word;
        tab_data->all_fifo = malloc(sizeof(char *) * nb_word);
    } else {
        tab_data->len = 0;
        tab_data->data = NULL;
    }

    return tab_data;
}

int nb_words(char *line) {
    if (line == NULL)
        return 0;

    size_t len_line = strlen(line);
    int nb_word = 0;
    int flag = 1;
    for (int i = 0; i < len_line; ++i) {
        if (*(line + i) != ' ' && flag) {
            ++nb_word;
            flag = 0;
        }
        if (*(line + i) == ' ') {
            flag = 1;
        }
    }
    return nb_word;
}

int is_str_redirection(char *str) {
    if (strcmp(str, "<") == 0 || strcmp(str, ">") == 0 || strcmp(str, ">|") == 0 || strcmp(str, ">>") == 0 || strcmp(str, "2>") == 0 || strcmp(str, "2>|") == 0 || strcmp(str, "2>>") == 0) {
        return 1;
    }
    return 0;
}

int is_redirection(char **data, int len) {
    if (len >= 3) {
        for (int i = 1; i < len; ++i) {
            if (is_str_redirection(data[i])) {
                return i;
            }
        }
    }
    return 0;
}

int which_redirection_str_is(char *str) {
    if (str == NULL)
        return 0;

    if (strcmp(str, "<") == 0)
        return 1;
    if (strcmp(str, ">") == 0)
        return 2;
    if (strcmp(str, ">|") == 0)
        return 3;
    if (strcmp(str, ">>") == 0)
        return 4;
    if (strcmp(str, "2>") == 0)
        return 5;
    if (strcmp(str, "2>|") == 0)
        return 6;
    if (strcmp(str, "2>>") == 0)
        return 7;
    else
        return 0;
}

int which_redirection(struct argv_t *arg) {
    if (arg == NULL)
        return 0;

    if (arg->len >= 3) {
        for (int i = 1; i < arg->len; ++i) {
            int r = which_redirection_str_is(arg->data[i]);
            if (r > 0) {
                return r;
            }
        }
    }
    return 0;
}

int nb_direction(struct argv_t *arg) {
    if (arg == NULL)
        return 0;

    int nb = 0;
    for (int i = 1; i < arg->len; ++i) {
        if (is_str_redirection(arg->data[i])) {
            ++nb;
        }
    }
    return nb;
}

int is_input_well_formed(struct argv_t * arg){
    for(int i = 0; i <= arg->len - 1; ++i){
        if (is_str_redirection(arg->data[i]) || strcmp(arg->data[i], "|") == 0){
            if (i != 0 && (i != arg->len - 1)){
                if (is_str_redirection(arg->data[i - 1]) || is_str_redirection(arg->data[i + 1]) || (strcmp(arg->data[i - 1], "|") == 0) || (strcmp(arg->data[i + 1], "|") == 0)){
                    return 0;
                }
            }
            else{
                return 0;
            }
        }
    }
    return 1;
}


struct argv_t *data_cmd(struct argv_t *arg, int redir) {
    if (arg == NULL)
        return NULL;

    struct argv_t *arg_cmd = malloc(sizeof(struct argv_t));
    char **data_cmd = malloc(sizeof(char *) * (redir + 1));
    for (int i = 0; i < redir; ++i) {
        data_cmd[i] = arg->data[i];
    }
    data_cmd[redir] = NULL;

    arg_cmd->data = data_cmd;
    arg_cmd->len = redir;

    return arg_cmd;
}

int is_process_substitution(char **data, int len, int *start, int *start_space, int *end, int *end_space) {
    if (data == NULL)
        return 0;

    for (int i = 0; i < len; i++) {
        char *current_str = data[i];
        if (current_str[0] == '<' && current_str[1] == '(') {
            if(strlen(current_str) == 2){
                *start_space = 1;
            }
            else{
                *start_space = 0;
            }
            *start = i;
            if (current_str[strlen(current_str) - 1] == ')') {
                *end = i;
                *end_space = 0;
                return 1;
            }
            for (int j = i + 1; j < len; j++) {
                char *next_str = data[j];
                if (next_str[strlen(next_str) - 1] == ')') {
                    if(strlen(current_str) == 1 ||strlen(current_str) == 2){
                        *end_space = 1;
                    }
                    else{
                        *end_space = 0;
                    }
                    *end = j;
                    return 1;
                }
            }
        }
    }
    return 0;
}

int count_pipes(char **data, int len) {
    if (data == NULL)  return 0;
    if(len == 1)  
        return 0;
    

    int pipe_count = 0;
    bool pipe = false;

    for(int i = 1; i < len-1; i++) {
        if(pipe && strcmp(data[i], "|") == 0) {
            return -1;
        }
        if (strcmp(data[i], "|") == 0) {
            pipe_count++;
            pipe = true;
        }
        else {
            pipe = false;
        }
    }

    return pipe_count;
}

char * get_cmd_pipe(char ** args, int len_char) {
    if (args == NULL || len_char == 0) return NULL;

    char *cmd = malloc(sizeof(char*));
    if (cmd == NULL) {
        fprintf(stderr, "error with malloc in get_cmd_pipe\n");
        exit(1);
    }
    cmd = realloc(cmd, sizeof(char) * (strlen(args[0]) + 1));
    strcpy(cmd, args[0]);
    for (int i = 1; i < len_char; ++i) {
        if(strcmp(args[i], "|") == 0) {
            break;
        }
        cmd = realloc(cmd, sizeof(char) * (strlen(cmd) + strlen(args[i]) + 2));
        strcat(cmd, " ");
        strcat(cmd, args[i]);
    }
    return cmd;
}

char **split_pipe(char **data, int len, int nb_pipes) {
    int pipe_count = count_pipes(data, len);
    if (pipe_count == -1) {
        return NULL;
    }

    char **commands = malloc((pipe_count + 1) * sizeof(char*));
    if (!commands) return NULL;

    int cmd_start = 0;
    int cmd_count = 0;

    for (int i = 0; i < len; ++i) {
        if (strcmp(data[i], "|") == 0 || i == len - 1) {
            int len_char = i - cmd_start + (i == len - 1 ? 1 : 0);
            commands[cmd_count++] = get_cmd_pipe(data + cmd_start, len_char);
            cmd_start = i + 1;
        }
    }

    return commands;
}

char **split_substitution(struct argv_t *args) {
    if (args == NULL || args->data == NULL || args->len <= 0) return NULL;

    char **commands = malloc(sizeof(char*) * 2);
    if (!commands) return NULL;

    commands[0] = calloc(1, sizeof(char));
    commands[1] = calloc(1, sizeof(char));

    for (int i = 0; i < args->len; ++i) {
        if (args->data[i][0] == '<' && args->data[i][1] == '(') {
            for (int k = i+1; k < args->len; k++) {
                if(args->data[k][strlen(args->data[k]) - 1] == ')') break;
                
                commands[1] = realloc(commands[1], strlen(commands[1]) + strlen(args->data[k]) + 2);
                strcat(commands[1], args->data[k]);
                strcat(commands[1], " ");
            }
            break; 
        } else {
            commands[0] = realloc(commands[0], strlen(commands[0]) + strlen(args->data[i]) + 2);
            strcat(commands[0], args->data[i]);
            strcat(commands[0], " ");
        }
    }
    if (strlen(commands[0]) > 0) commands[0][strlen(commands[0]) - 1] = '\0';
    if (strlen(commands[1]) > 0) commands[1][strlen(commands[1]) - 1] = '\0';


    return commands;
}

char **split_without_first_substitution(char **data, int *len, int start, int end, char *fifo_substition){
    int new_len = start + *len - end - 1;
    
    char **new_data = malloc(sizeof(char *) * (new_len + 1));
    int k = 0;
    for(int i = 0 ; i < start ; ++i){
        // new_data[k] = data[i];
        new_data[k] = malloc(strlen(data[i])+1);
        strcpy(new_data[k], data[i]);
        ++k;
    }

    new_data[k] = malloc(sizeof(char) * (strlen(fifo_substition) + 1));
    strcpy(new_data[k], fifo_substition);
    ++k;

    for(int i = end + 1 ; i < *len ; ++i){
        // new_data[k] = data[i];
        new_data[k] = malloc(strlen(data[i])+1);
        strcpy(new_data[k], data[i]);
        ++k;
    }

    new_data[k] = NULL;

    *len = k;
    return new_data;
}
