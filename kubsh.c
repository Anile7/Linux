#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>

#define MAX_INPUT 1024

void handle_sighup(int sig) {
    write(STDOUT_FILENO, "Configuration reloaded\n", 23);
}

int main() {
    char buf[MAX_INPUT];

    // создать ~/users БЕЗ ВЫВОДА
    char users_dir[256];
    snprintf(users_dir, sizeof(users_dir), "%s/users", getenv("HOME"));
    mkdir(users_dir, 0755);

    signal(SIGHUP, handle_sighup);

    while (fgets(buf, MAX_INPUT, stdin)) {
        buf[strcspn(buf, "\n")] = 0;

        // выход
        if (strcmp(buf, "\\q") == 0) {
            break;
        }

        // debug
        if (strncmp(buf, "debug ", 6) == 0) {
            char *msg = buf + 6;

            // убрать кавычки если есть
            if (msg[0] == '"' && msg[strlen(msg) - 1] == '"') {
                msg[strlen(msg) - 1] = 0;
                msg++;
            }

            printf("%s\n", msg);
            continue;
        }

        // env
        if (strncmp(buf, "\\e ", 3) == 0) {
            char *var = buf + 3;

            if (var[0] == '$')
                var++;

            char *val = getenv(var);
            if (!val) {
                printf("Variable not found\n");
            } else {
                // FIX: если нет ':', печатаем одной строкой
                if (strchr(val, ':') == NULL) {
                    printf("%s\n", val);
                } else {
                    char *copy = strdup(val);
                    char *tok = strtok(copy, ":");
                    while (tok) {
                        printf("%s\n", tok);
                        tok = strtok(NULL, ":");
                    }
                    free(copy);
                }
            }
            continue;
        }

        // внешние команды
        char *args[64];
        int i = 0;
        char *tok = strtok(buf, " ");
        while (tok) {
            args[i++] = tok;
            tok = strtok(NULL, " ");
        }
        args[i] = NULL;

        pid_t pid = fork();
        if (pid == 0) {
            execvp(args[0], args);
            printf("%s: command not found\n", args[0]);
            exit(1);
        }
        wait(NULL);
    }

    return 0;
}
