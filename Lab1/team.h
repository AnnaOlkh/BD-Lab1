#pragma once

struct Team
{
    int id;
    char name[50];
    char trainer_name[50];
    int first_player = -1; //adress of first player in the list
    bool is_deleted = false;
};
