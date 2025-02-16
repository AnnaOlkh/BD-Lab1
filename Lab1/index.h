#pragma once
struct IndexEntry {
    int id;       // Первинний ключ
    int position; // Номер запису у файлі (позиція)
};
void load_index();
void save_index();
void insert_m(int id, const char* name, const char* team_name);
void insert_s(int passport_code, const char* name, const char* birthdate, const char* team_name);