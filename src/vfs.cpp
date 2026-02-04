#define FUSE_USE_VERSION 35 

#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <pwd.h> 
#include <sys/types.h>
#include <cerrno>
#include <ctime>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <iostream>

#include "vfs.hpp"

int run_cmd(const char* cmd, char* const argv[]) {
    pid_t pid = fork();
    if (pid < 0) {
        return -1;
    }

    if (pid == 0) {
        execvp(cmd, argv);
        _exit(127); 
    }

    int status = 0;
    if (waitpid(pid, &status, 0) == -1) {
        return -1;
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) return 0;
    return -1;
}

bool valid_shell(struct passwd* pwd)
{
    if (!pwd || !pwd->pw_shell) return false;
    size_t len = strlen(pwd->pw_shell);
    if (len < 2) return false;
    return (strcmp(pwd->pw_shell + len - 2, "sh") == 0);
}

int users_getattr(const char* path, struct stat* st, struct fuse_file_info* fi) {
    (void) fi; 
    memset(st, 0, sizeof(struct stat));

    time_t now = time(NULL); 
    st->st_atime = st->st_mtime = st->st_ctime = now;

    if (strcmp(path, "/") == 0) {
        st->st_mode = S_IFDIR | 0755; 
        st->st_uid = getuid();
        st->st_gid = getgid();
        st->st_nlink = 2;

        return 0;
    }

    char username[256] = {0};
    char filename[256] = {0};

    if (sscanf(path, "/%255[^/]/%255[^/]", username, filename) == 2) {
        struct passwd* pwd = getpwnam(username);
        if (!pwd) return -ENOENT; 

        if (strcmp(filename, "id") != 0 &&
            strcmp(filename, "home") != 0 &&
            strcmp(filename, "shell") != 0) {
            return -ENOENT;
        }

        st->st_mode = S_IFREG | 0644; //rw - owner, r-- others
        st->st_uid = pwd->pw_uid;
        st->st_gid = pwd->pw_gid;
        st->st_nlink = 1;

        if (strcmp(filename, "id") == 0) {
            st->st_size = 16;
        } else if (strcmp(filename, "home") == 0) {
            st->st_size = pwd->pw_dir ? (off_t)strlen(pwd->pw_dir) : 0;
        } else {
            st->st_size = pwd->pw_shell ? (off_t)strlen(pwd->pw_shell) : 0;
        }
        return 0;
    }

    if (sscanf(path, "/%255[^/]", username) == 1) {
        struct passwd* pwd = getpwnam(username);
        if (!pwd) return -ENOENT;

        st->st_mode = S_IFDIR | 0755;
        st->st_uid = pwd->pw_uid;
        st->st_gid = pwd->pw_gid;
        st->st_nlink = 2;
        return 0;
    }

    return -ENOENT;
}

int users_readdir(
    const char* path,
    void* buf,
    fuse_fill_dir_t filler,
    off_t offset,
    struct fuse_file_info* fi,
    enum fuse_readdir_flags flags
) {
    (void) offset;
    (void) fi;
    (void) flags;

    filler(buf, ".", NULL, 0, FUSE_FILL_DIR_PLUS);
    filler(buf, "..", NULL, 0, FUSE_FILL_DIR_PLUS);

    if (strcmp(path, "/") == 0) {
        struct passwd* pwd;
        setpwent();
        while ((pwd = getpwent()) != NULL) {
            if (valid_shell(pwd)) {
                filler(buf, pwd->pw_name, NULL, 0, FUSE_FILL_DIR_PLUS);
            }
        }
        endpwent();
        return 0;
    }

    char username[256] = {0};
    if (sscanf(path, "/%255[^/]", username) == 1) {
        struct passwd* pwd = getpwnam(username);
        if (!pwd) return -ENOENT;

        filler(buf, "id", NULL, 0, FUSE_FILL_DIR_PLUS);
        filler(buf, "home", NULL, 0, FUSE_FILL_DIR_PLUS);
        filler(buf, "shell", NULL, 0, FUSE_FILL_DIR_PLUS);
        return 0;
    }

    return -ENOENT;
}

int users_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
    (void) fi;

    char username[256];
    char filename[256];

    std::sscanf(path, "/%255[^/]/%255[^/]", username, filename);

    struct passwd* pwd = getpwnam(username);
    if (!pwd) return -ENOENT;

    char content[256] = {0}; 

    if (std::strcmp(filename, "id") == 0) {
        std::snprintf(content, sizeof(content), "%d", pwd->pw_uid);
    }
    else if (std::strcmp(filename, "home") == 0) {
        std::snprintf(content, sizeof(content), "%s", pwd->pw_dir);
    }
    else if (std::strcmp(filename, "shell") == 0) {
        std::snprintf(content, sizeof(content), "%s", pwd->pw_shell);
    } else {
        return -ENOENT;
    }

    size_t len = std::strlen(content);
    if ((size_t)offset >= len)
        return 0;

    if (offset + size > len)
        size = len - offset;

    std::memcpy(buf, content + offset, size);
    return size;
}


int users_mkdir(const char* path, mode_t mode) {
    (void) mode;

    char username[256] = {0};
    if (sscanf(path, "/%255[^/]", username) != 1) {
        return -EINVAL;
    }
    if (strchr(path + 1, '/') != NULL) {
        return -EPERM;
    }
    struct passwd* pwd = getpwnam(username);
    if (pwd != NULL) return -EEXIST;

    char* const argv[] = {
        (char*)"adduser",
        (char*)"--disabled-password",
        (char*)"--gecos",
        (char*)"",
        username,
        NULL
    };

    if (run_cmd("adduser", argv) == 0) return 0;
    return -EIO;
}

int users_rmdir(const char* path) {
    char username[256] = {0};
    if (sscanf(path, "/%255[^/]", username) != 1) {
        return -EINVAL;
    }

    if (strchr(path + 1, '/') != NULL) {
        return -EPERM;
    }

    struct passwd* pwd = getpwnam(username);
    if (!pwd) return -ENOENT;

    char* const argv[] = {
        (char*)"userdel",
        (char*)"--remove",
        username,
        NULL
    };

    if (run_cmd("userdel", argv) == 0) return 0;
    return -EIO;
}

struct fuse_operations users_operations = {};

void init_users_operations() {
    users_operations.getattr = users_getattr;
    users_operations.readdir = users_readdir;
    users_operations.read = users_read;
    users_operations.mkdir = users_mkdir;
    users_operations.rmdir = users_rmdir;
}
