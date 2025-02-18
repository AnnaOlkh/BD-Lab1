#pragma once
struct Player
{
    int passport_code;
    int team_id;
    char name[50];
    char birthdate[11];
    int next_player = -1;
    int prev_player = -1;
    bool is_deleted;
}; 