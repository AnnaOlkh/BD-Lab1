#pragma once
struct Player
{
    int passport_code;
    int team_id;
    char name[50];
    char birthdate[10];
    int next_player = -1; // Linked list address
    bool is_deleted;
}; 