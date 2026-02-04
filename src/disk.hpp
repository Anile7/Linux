#pragma once
#include <string>
#include <iostream>

using namespace std;

void print_sector_hex(const unsigned char* buffer);
bool analyze_gpt_partition(const string& device);
void analyze_mbr_partition(const string& device_path);
