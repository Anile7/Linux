#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <cstdlib>

using namespace std;

vector<string> history;
string HISTORY_FILE;

string getHomeDir() {
    const char* home = getenv("HOME");
    if (home) return string(home) + "/.kubsh_history";
    return ".kubsh_history";
}

void loadHistory() {
    HISTORY_FILE = getHomeDir();
    ifstream file(HISTORY_FILE);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            if (!line.empty()) history.push_back(line);
        }    
    }
}

void saveHistory() {
    ofstream file(HISTORY_FILE);
    if (file.is_open()) {
        for (const auto& cmd : history) {
            file << cmd << endl;
        }
    }
}

void showHistory() {
    cout << "History of commands: " << endl;
    for (int i = 0; i < history.size(); i++) {
        cout << i + 1 << ": " << history[i] << endl;
    }
}

int main() {
    loadHistory();

    string input;

    cout << "Kubsh started. Type 'exit' or '\\q' to quit." << endl;

    while (true) {
        cout << "$ ";

        if (!getline(cin, input)) {
            cout << endl;
            break;
        }

        if (input.empty()) continue;

        history.push_back(input);

        if (input == "exit" || input == "\\q") {
            cout << "Exiting kubsh." << endl;            
            break;
        }
        else if (input == "history") {
            showHistory();
        }
        else if (input.find("echo ") == 0) {
            string text = input.substr(5);
            cout << text << endl;
        }
        else {
            cout << "You entered: " << input << endl;
        }
    }

    saveHistory();

    return 0;
}

