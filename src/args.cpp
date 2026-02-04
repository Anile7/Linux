#include "args.hpp"
#include <cctype>
#include <vector>

using namespace std;

vector<string> parse_command_line(const string& line) {
    vector<string> tokens;
    string current_token;
    bool inside_quotes = false;

    for (char ch : line) {
        if (ch == '"') {
            inside_quotes = !inside_quotes;
        } else if (isspace(ch) && !inside_quotes) {
            if (!current_token.empty()) {
                tokens.push_back(current_token);
                current_token.clear();
            }
        } else {
            current_token += ch;
        }
    }

    if (!current_token.empty()) {
        tokens.push_back(current_token);
    }

    return tokens;
}
