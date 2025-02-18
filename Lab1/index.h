#ifndef INDEX_H
#define INDEX_H

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>
#include <queue>

struct IndexEntry {
    int id;
    int pos;
};

struct IndexTable {
    std::queue<int> free_ids;
    std::map<int, int> records;  
    int counter = 1;             
};

void load_index(const char* filename, IndexTable& index);
void save_index(const char* filename, IndexTable& index);
int generate_id(IndexTable& index);
int get_team_id_by_name(const char* team_name);

#endif