// src/main.cpp
#include <iostream>
#include <string>
#include <vector>
#include <csignal>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <pthread.h>
#include <pwd.h>
#include <time.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "args.hpp"          
#include "path.hpp"        
#include "built.hpp"         
#include "h.hpp"            
#include "exec.hpp"         
#include "vfs_man.hpp"
#include "vfs.hpp"

using namespace std;

void handle_sighup(int) {
    cout << "\nConfiguration reloaded" << endl;
}

int main() {
    cout << unitbuf;
    cerr << unitbuf;

    fuse_start();
    signal(SIGHUP, handle_sighup);

    auto history = open_history_file(); 

    string input;
    while (true) {
        cerr << "$ ";
        if (!getline(cin, input)) break;
        if (input.empty()) continue;

        append_to_history(history, input);  

        if (process_builtin_command(input)) 
            continue;

        vector<string> args = parse_command_line(input);  
        if (args.empty()) continue;

        string cmd = resolve_command_path(args[0]);  
        if (cmd.empty()) {
            cout << args[0] << ": command not found\n";
            continue;
        }

        run_external_command(cmd, args);  
    }

    return 0;
}
