#pragma once
#include <string>
#include <fstream>

std::ofstream open_history_file();
void append_to_history(std::ofstream& history_file, const std::string& command);
