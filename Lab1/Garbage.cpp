#include "garbage.h"

void load_trash(const char* filename, std::queue<int>& free_ids) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return;

    int count;
    file.read(reinterpret_cast<char*>(&count), sizeof(int));

    for (int i = 0; i < count; i++) {
        int id;
        file.read(reinterpret_cast<char*>(&id), sizeof(int));
        free_ids.push(id);
    }

    file.close();
}

void save_trash(const char* filename, std::queue<int>& free_ids) {
    std::ofstream file(filename, std::ios::binary | std::ios::trunc);
    int count = free_ids.size();
    file.write(reinterpret_cast<const char*>(&count), sizeof(int));

    std::queue<int> temp_queue = free_ids;
    while (!temp_queue.empty()) {
        int id = temp_queue.front();
        temp_queue.pop();
        file.write(reinterpret_cast<const char*>(&id), sizeof(int));
    }

    file.close();
}
