#pragma once

struct Team
{
    int id;
    char name[50];
    char trainer_name[50];
    int first_player = -1;
    bool is_deleted = false;
};
