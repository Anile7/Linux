#include "h.hpp"
#include <cstdlib>
#include <iostream>

using namespace std;

ofstream open_history_file() {
    const char* home_dir = getenv("HOME");
    string history_path = string(home_dir ? home_dir : ".") + "/.kubsh_history";
    return ofstream(history_path, ios::app);
}

void append_to_history(ofstream& history_file, const string& command) {
    history_file << command << endl;
    history_file.flush();
}
