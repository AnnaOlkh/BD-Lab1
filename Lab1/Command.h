#pragma once
#include<string>
void get_s(int player_id);
void get_m(int team_id);

void insert_s(const char* name, const char* birthdate, const char* team_name);
void insert_m(const char* name, const char* trainer_name);
void initialize();
void del_s(int player_id);
void del_m(int team_id);
void ut_s();
void ut_m();
void update_m(int team_id, const std::string& field, const std::string& new_value);
void update_s(int player_id, const std::string& field, const std::string& new_value);
void calc_s();
void calc_m();
void help();