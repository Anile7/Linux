#define FUSE_USE_VERSION 35

#include <fuse3/fuse.h>
#include <pthread.h>
#include <unistd.h>

#include "vfs.hpp"  

void* fuse_thread_function(void* arg) {
    (void)arg;

    init_users_operations();

    const char* fuse_argv[] = {
        "vfs_users",
        "-f", 
        "-omax_idle_threads=10000",
        "-odefault_permissions",
        "-oauto_unmount",
        "/opt/users"    
    };

    int fuse_argc = sizeof(fuse_argv) / sizeof(fuse_argv[0]);

    fuse_main(fuse_argc, (char**)fuse_argv, &users_operations, NULL);

    return NULL;
}

void fuse_start() {
    pthread_t fuse_thread;
    pthread_create(&fuse_thread, NULL, fuse_thread_function, NULL);
}


