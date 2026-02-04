#include "built.hpp"
#include <iostream>
#include <cstdlib>
#include "disk.hpp"

using namespace std;

bool process_builtin_command(const string& command) {
    if (command == "\\q") {
        exit(0);
    }

    if (command.rfind("echo ", 0) == 0 || command.rfind("debug ", 0) == 0) {
        size_t space_pos = command.find(' ');
        if (space_pos != string::npos) {
            string message = command.substr(space_pos + 1);
            if (message.size() >= 2 && message.front() == '\'' && message.back() == '\'') {
                message = message.substr(1, message.size() - 2);
            }
            cout << message << endl;
        }
        return true;
    }

    if (command.rfind("\\e ", 0) == 0) {
        string variable_name = command.substr(3);

        if (variable_name.empty() || variable_name[0] != '$') {
            cout << "Неверная команда. Надо: \\e $VARNAME\n";
            return true;
        }

        const char* value = getenv(variable_name.substr(1).c_str());
        if (!value) return true;

        string env_value = value;
        size_t start = 0, colon_pos;

        while ((colon_pos = env_value.find(':', start)) != string::npos) {
            cout << env_value.substr(start, colon_pos - start) << endl;
            start = colon_pos + 1;
        }
        cout << env_value.substr(start) << endl;
        return true;
    }

    if (command.rfind("\\l ", 0) == 0) {
        string device_path = command.substr(3);
        analyze_mbr_partition(device_path);
        analyze_gpt_partition(device_path);
        return true;
    }

    return false;
}
