#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstring>
#include "team.h"
#include "player.h"
#include "index.h"

using namespace std;

const char* MASTER_FILE = "Team.fl";
const char* SLAVE_FILE = "Players.fl";
const char* INDEX_FILE = "Team.ind";

// Вектор для індексної таблиці
vector<IndexEntry> index_table; 

// Функція завантаження індексної таблиці в пам’ять
void load_index() {
    ifstream file(INDEX_FILE, ios::binary);
    if (file) {
        IndexEntry entry;
        while (file.read(reinterpret_cast<char*>(&entry), sizeof(IndexEntry))) {
            index_table.push_back(entry);
        }
    }
    file.close();
}

// Функція збереження індексної таблиці у файл
void save_index() {
    ofstream file(INDEX_FILE, ios::binary | ios::trunc);
    for (const auto& entry : index_table) {
        file.write(reinterpret_cast<const char*>(&entry), sizeof(IndexEntry));
    }
    file.close();
}

int get_team_id_by_name(const char* team_name) {
    ifstream file("Team.fl", ios::binary);
    Team team;
    while (file.read(reinterpret_cast<char*>(&team), sizeof(Team))) {
        if (strcmp(team.name, team_name) == 0 && !team.is_deleted) {
            return team.id;
        }
    }
    return -1; // Команда не знайдена
}

void insert_m(int id, const char* name, const char* trainer_name) {
    // Відкриваємо файл у режимі дописування (ios::app)
    fstream file(MASTER_FILE, ios::binary | ios::app);

    // Формуємо структуру запису
    Team record = { id, "", "", -1, false};
    strcpy_s(record.name, sizeof(record.name), name);
    strcpy_s(record.trainer_name, sizeof(record.trainer_name), trainer_name);


    // Визначаємо позицію запису у файлі
    file.seekp(0, ios::end);
    int pos = file.tellp() / sizeof(Team);

    // Записуємо новий запис у файл
    file.write(reinterpret_cast<char*>(&record), sizeof(Team));
    file.close();

    // Додаємо запис у індексну таблицю
    index_table.push_back({ id, pos });

    // Сортуємо індексну таблицю за зростанням ID
    sort(index_table.begin(), index_table.end(), [](const IndexEntry& a, const IndexEntry& b) {
        return a.id < b.id;
        });

    save_index();

    printf("Inserted master record\nTeam: ID(%d), %s, %s\n", id, name, trainer_name);
}

void insert_s(int passport_code, const char* name, const char* birthdate, const char* team_name) {
    // Отримуємо ID команди за її назвою
    int team_id = get_team_id_by_name(team_name);
    if (team_id == -1) {
        printf("Error: Team \"%s\" not found.\n", team_name);
        return;
    }

    // Відкриваємо slave-файл для запису
    fstream file(SLAVE_FILE, ios::binary | ios::app);

    // Формуємо структуру запису
    Player record = { passport_code, team_id, "", -1, false };
    strcpy_s(record.name, sizeof(record.name), name);
    strcpy_s(record.birthdate, sizeof(record.birthdate), birthdate);
    record.next_player = -1;

    // Визначаємо позицію запису у файлі
    file.seekp(0, ios::end);
    int pos = file.tellp() / sizeof(Player);

    // Записуємо новий запис у файл
    file.write(reinterpret_cast<char*>(&record), sizeof(Player));
    file.close();

    // Додаємо запис у індексну таблицю
    index_table.push_back({ passport_code, pos });

    // Сортуємо індексну таблицю за зростанням ключа
    sort(index_table.begin(), index_table.end(), [](const IndexEntry& a, const IndexEntry& b) {
        return a.id < b.id;
        });

    save_index();

    printf("Inserted player record\nPlayer: Passport(%d), %s,  %s, %d\n", 
        passport_code, name, birthdate, team_name);
}

