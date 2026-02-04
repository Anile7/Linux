#include "exec.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>

using namespace std;

void run_external_command(const string& executable_path, const vector<string>& arguments) {
    vector<string> arg_storage = arguments;
    vector<char*> argv;

    for (auto& arg : arg_storage) {
        argv.push_back(arg.data());
    }
    argv.push_back(nullptr);

    pid_t child_pid = fork();

    if (child_pid == 0) {
        execvp(executable_path.c_str(), argv.data());
        cerr << "Ошибка выполнения: " << arguments[0] << endl;
        exit(1);
    } else if (child_pid > 0) {
        int status;
        waitpid(child_pid, &status, 0);
    } else {
        cerr << "Ошибка при создании процесса\n";
    }
}
