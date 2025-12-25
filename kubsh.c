#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>

#define MAX_INPUT 1024
#define HISTORY_SIZE 100

// --- история команд ---
char history[HISTORY_SIZE][MAX_INPUT];
int history_count = 0;

void add_to_history(const char *cmd) {
    if (strlen(cmd) == 0) return;

    if (history_count >= HISTORY_SIZE) {
        for (int i = 1; i < HISTORY_SIZE; i++) {
            strcpy(history[i-1], history[i]);
        }
        history_count--;
    }

    strcpy(history[history_count], cmd);
    history_count++;
}

void load_history() {
    FILE *file = fopen(".kubsh_history", "r");
    if (!file) return;

    char line[MAX_INPUT];
    while (fgets(line, MAX_INPUT, file) && history_count < HISTORY_SIZE) {
        line[strcspn(line, "\n")] = 0;
        strcpy(history[history_count], line);
        history_count++;
    }

    fclose(file);
}

void save_history() {
    FILE *file = fopen(".kubsh_history", "w");
    if (!file) return;

    for (int i = 0; i < history_count; i++) {
        fprintf(file, "%s\n", history[i]);
    }

    fclose(file);
}

// --- выполнение внешних команд ---
int execute_command(char *args[]) {
    pid_t pid = fork();

    if (pid == 0) { // дочерний процесс
        execvp(args[0], args);
        printf("kubsh: команда не найдена: %s\n", args[0]);
        exit(1);
    } else if (pid > 0) { // родитель
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    } else {
        printf("kubsh: ошибка fork\n");
        return 1;
    }
}

// --- обработчик SIGHUP ---
void handle_sighup(int sig) {
    const char *msg = "\nConfiguration reloaded\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

// --- функции для пользователей (VFS) ---
void add_user(const char *username) {
    char users_dir[256];
    snprintf(users_dir, sizeof(users_dir), "%s/users", getenv("HOME"));

    char user_path[256];
    snprintf(user_path, sizeof(user_path), "%s/%s", users_dir, username);

    if (access(user_path, F_OK) == 0) {
        printf("Пользователь %s уже существует\n", username);
        return;
    }

    if (mkdir(user_path, 0755) != 0) {
        perror("Ошибка создания каталога пользователя");
        return;
    }

    char file_path[256];
    FILE *f;

    // id
    snprintf(file_path, sizeof(file_path), "%s/id", user_path);
    f = fopen(file_path, "w");
    if (!f) { perror("Ошибка создания id"); return; }
    fprintf(f, "%d", 1000 + history_count);
    fclose(f);

    // home
    snprintf(file_path, sizeof(file_path), "%s/home", user_path);
    f = fopen(file_path, "w");
    if (!f) { perror("Ошибка создания home"); return; }
    fprintf(f, "%s/%s", getenv("HOME"), username);
    fclose(f);

    // shell
    snprintf(file_path, sizeof(file_path), "%s/shell", user_path);
    f = fopen(file_path, "w");
    if (!f) { perror("Ошибка создания shell"); return; }
    fprintf(f, "/bin/bash");
    fclose(f);

    printf("Пользователь %s создан\n", username);
}

void delete_user(const char *username) {
    char user_path[256];
    snprintf(user_path, sizeof(user_path), "%s/users/%s", getenv("HOME"), username);

    if (access(user_path, F_OK) != 0) {
        printf("Пользователь %s не найден\n", username);
        return;
    }

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", user_path);
    system(cmd);

    printf("Пользователь %s удалён\n", username);
}

// --- основной shell ---
int main() {
    char buf[MAX_INPUT];

    // создаём ~/users
    char users_dir[256];
    snprintf(users_dir, sizeof(users_dir), "%s/users", getenv("HOME"));
    if (access(users_dir, F_OK) != 0) {
        if (mkdir(users_dir, 0755) != 0) {
            perror("Ошибка создания директории ~/users");
            return 1;
        }
        printf("Создана директория: %s\n", users_dir);
    }

    // установка обработчика сигнала
    signal(SIGHUP, handle_sighup);

    load_history();

    while (1) {
        printf("kubsh> ");

        if (fgets(buf, MAX_INPUT, stdin) == NULL) {
            printf("\n");
            break;
        }

        buf[strcspn(buf, "\n")] = 0;

        // --- встроенная команда \q ---
        if (strcmp(buf, "\\q") == 0) {
            printf("Выход из shell\n");
            break;
        }

        // --- встроенная команда echo ---
        if (strncmp(buf, "echo", 4) == 0 && (buf[4] == ' ' || buf[4] == '\0')) {
            char *args = buf + 5;
            if (args) printf("%s\n", args);
            add_to_history(buf);
            continue;
        }

        // --- встроенная команда \e ---
        if (strncmp(buf, "\\e", 2) == 0 && (buf[2] == ' ' || buf[2] == '\0')) {
            char *var_name = buf + 3;
            if (strlen(var_name) == 0) {
                printf("Использование: \\e VAR_NAME\n");
            } else {
                char *value = getenv(var_name);
                if (value) printf("%s=%s\n", var_name, value);
                else printf("Переменная не найдена\n");
            }
            add_to_history(buf);
            continue;
        }

        // --- встроенная команда \l ---
        if (strncmp(buf, "\\l", 2) == 0 && (buf[2] == ' ' || buf[2] == '\0')) {
            char *device = buf + 3;
            if (strlen(device) == 0) {
                printf("Использование: \\l /dev/disk\n");
            } else {
                char cmd[256];
                snprintf(cmd, sizeof(cmd), "lsblk %s 2>/dev/null || echo 'Ошибка'", device);
                system(cmd);
            }
            add_to_history(buf);
            continue;
        }

        // --- команды adduser / deluser ---
        if (strncmp(buf, "adduser ", 8) == 0) {
            char *username = buf + 8;
            add_user(username);
            add_to_history(buf);
            continue;
        }

        if (strncmp(buf, "deluser ", 8) == 0) {
            char *username = buf + 8;
            delete_user(username);
            add_to_history(buf);
            continue;
        }

        // --- внешняя команда ---
        if (strlen(buf) > 0) {
            char *args[MAX_INPUT/2 + 1];
            int arg_count = 0;
            char *token = strtok(buf, " ");
            while (token && arg_count < MAX_INPUT/2) {
                args[arg_count++] = token;
                token = strtok(NULL, " ");
            }
            args[arg_count] = NULL;

            execute_command(args);
            add_to_history(buf);
        }
    }

    save_history();
    return 0;
}
