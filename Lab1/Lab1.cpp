#include <iostream>
#include "team.h"
#include "player.h"
#include "index.h"

using namespace std;

int main()
{
    load_index();
    while (true)
    {
        string command;
        cin >> command;
        if (command == "insert_m")
        {
            int team_counter = 1;
            char name[50], trainer_name[50];
            cin >> name >> trainer_name;
            insert_m(team_counter, name, trainer_name);
            team_counter++;
        }
        if (command == "insert_s")
        {
            int pl_counter = 1;
            char name[50], team_name[50], birthday_data[10];
            cin >> name >> birthday_data >> team_name;
            insert_s(pl_counter, name, birthday_data, team_name);
            pl_counter++;
        }
    }
}