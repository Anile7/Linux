#include "path.hpp"
#include <unistd.h>
#include <sstream>
#include <cstdlib>

using namespace std;

string resolve_command_path(const string& command_name) {
    if (command_name.find('/') != string::npos) {
        return (access(command_name.c_str(), X_OK) == 0) ? command_name : "";
    }

    const char* path_env = getenv("PATH");
    if (!path_env) return "";

    stringstream path_stream(path_env);
    string directory;

    while (getline(path_stream, directory, ':')) {
        string full_path = directory + "/" + command_name;
        if (access(full_path.c_str(), X_OK) == 0) {
            return full_path;
        }
    }

    return "";
}
