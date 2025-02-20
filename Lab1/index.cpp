#include "index.h"
#include "Team.h"

void load_index(const char* filename, IndexTable& index) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return;

    int count;
    file.read(reinterpret_cast<char*>(&count), sizeof(int));
    int max_id = 0;

    for (int i = 0; i < count; i++) {
        int id, pos;
        file.read(reinterpret_cast<char*>(&id), sizeof(int));
        file.read(reinterpret_cast<char*>(&pos), sizeof(int));
        index.records[id] = pos;
        if (id > max_id) {
            max_id = id;
        }
    }
    index.counter = max_id + 1;
    file.close();
}

void save_index(const char* filename, IndexTable& index) {
    std::ofstream file(filename, std::ios::binary | std::ios::trunc);
    int count = index.records.size();
    file.write(reinterpret_cast<const char*>(&count), sizeof(int));

    for (const auto& entry : index.records) {
        file.write(reinterpret_cast<const char*>(&entry.first), sizeof(int));
        file.write(reinterpret_cast<const char*>(&entry.second), sizeof(int));
    }

    file.close();
}

int generate_id(IndexTable& index) {
    if (!index.free_ids.empty()) {
        int id = index.free_ids.front();
        index.free_ids.pop();
        return id;
    }
    return index.counter++;
}
int get_team_id_by_name(const char* team_name) {
    std :: ifstream file("Team.fl", std :: ios::binary);
    Team team;
    while (file.read(reinterpret_cast<char*>(&team), sizeof(Team))) {
        if (strcmp(team.name, team_name) == 0 && !team.is_deleted) {
            file.close();
            return team.id;
        }
    }
    file.close();
    return -1;
}