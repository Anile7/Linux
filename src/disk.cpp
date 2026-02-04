// src/disk.cpp
#include "disk.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <cstdint>   
#include <cstring>   

using namespace std;

#pragma pack(push, 1)
struct GPTHeader {
    char signature[8];
    uint32_t revision;
    uint32_t header_size;
    uint32_t crc32_header;
    uint32_t reserved;
    uint64_t current_lba;
    uint64_t backup_lba;
    uint64_t first_usable_lba;
    uint64_t last_usable_lba;
    unsigned char disk_guid[16];
    uint64_t partition_entries_lba;
    uint32_t partition_count;         
    uint32_t partition_entry_size;   
    uint32_t crc32_partitions;
};

struct GPTEntry {
    unsigned char type_guid[16];
    unsigned char partition_guid[16];
    uint64_t first_lba;
    uint64_t last_lba;
    uint64_t attributes;
    uint16_t name[36];
};
#pragma pack(pop)

void print_sector_hex(const unsigned char* buffer) {
    for (int i = 0; i < 512; ++i) {
        printf("%02X ", buffer[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

bool analyze_gpt_partition(const string& device) {
    int fd = open(device.c_str(), O_RDONLY);
    if (fd < 0) {
        cout << "Ошибка: не удалось открыть устройство " << device << endl;
        return false;
    }

    unsigned char sector[512];

    // Читаем сектор 1 (LBA 1) — там находится GPT-заголовок
    if (lseek(fd, 512, SEEK_SET) == -1 || read(fd, sector, 512) != 512) {
        cout << "Ошибка чтения заголовка GPT (сектор 1)\n";
        close(fd);
        return false;
    }

    GPTHeader* header = reinterpret_cast<GPTHeader*>(sector);

    if (strncmp(header->signature, "EFI PART", 8) != 0) {
        cout << "Сигнатура GPT не найдена (ожидается \"EFI PART\")\n";
        close(fd);
        return false;
    }

    cout << "===== GPT-таблица обнаружена =====\n";
    cout << "GUID диска: ";
    for (int i = 0; i < 16; ++i) {
        printf("%02X", header->disk_guid[i]);
        if (i == 3 || i == 5 || i == 7 || i == 9) cout << "-";
    }
    cout << "\nКоличество записей в таблице разделов: " << header->partition_count << endl;
    cout << "Размер одной записи раздела: " << header->partition_entry_size << " байт\n";
    cout << "Первый используемый LBA: " << header->first_usable_lba << endl;
    cout << "Последний используемый LBA: " << header->last_usable_lba << endl;

    close(fd);
    return true;
}

void analyze_mbr_partition(const string& device_path) {
    cout << "===== Анализ MBR =====\n";

    int fd = open(device_path.c_str(), O_RDONLY);
    if (fd < 0) {
        cout << "Ошибка: не удалось открыть устройство " << device_path << endl;
        return;
    }

    unsigned char mbr_sector[512];
    if (read(fd, mbr_sector, 512) != 512) {
        cout << "Ошибка чтения MBR (сектор 0)\n";
        close(fd);
        return;
    }
    close(fd);

    if (mbr_sector[510] == 0x55 && mbr_sector[511] == 0xAA) {
        cout << "Сигнатура MBR в порядке (0x55AA)\n";
    } else {
        cout << "Сигнатура MBR отсутствует или повреждена\n";
        return;
    }

    unsigned char boot_flag = mbr_sector[446];
    cout << "Флаг активности первого раздела: 0x" << hex << (int)boot_flag << dec << endl;

    if (boot_flag == 0x80) {
        cout << "Первый раздел помечен как загрузочный (active)\n";
    } else if (boot_flag == 0x00) {
        cout << "Первый раздел не является загрузочным\n";
    } else {
        cout << "Неизвестное значение флага активности (нестандартное)\n";
    }

    cout << "Запись первого раздела (байты 446–461):\n  ";
    for (int i = 446; i < 462; ++i) {
        printf("%02X ", mbr_sector[i]);
    }
    cout << endl;
}
