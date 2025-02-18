#ifndef INDEX_H
#define INDEX_H

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>
#include <queue>

struct IndexEntry {
    int id;     // Ключ (team_id або passport_code)
    int pos;    // Позиція у файлі
};

struct IndexTable {
    std::queue<int> free_ids;    // Черга звільнених ID для повторного використання
    std::map<int, int> records;  // Мапа для зберігання ID -> позиція у файлі
    int counter = 1;             // Автоінкремент ID
};

void load_index(const char* filename, IndexTable& index);
void save_index(const char* filename, IndexTable& index);
int generate_id(IndexTable& index);
int get_team_id_by_name(const char* team_name);

#endif