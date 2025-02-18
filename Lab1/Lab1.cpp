#include "command.h"
#include "garbage.h"
#include "team.h"
#include "player.h"
#include <iostream>

using namespace std;


int main()
{
    initialize();  // Load index and garbage data

    string command;
    while (true)
    {
        cout << "> ";
        cin >> command;

        if (command == "exit") break;
        else if (command == "insert-m")
        {
            cout << "Print team's name and trainer of team\n";
            char name[50], trainer[50];
            cin >> ws;
            cin.getline(name, 50);
            cin.getline(trainer, 50);
            insert_m(name, trainer);
        }
        else if (command == "insert-s")
        {
            cout << "Print player's name, birthday dat and the name of team\n";
            char name[50], birthdate[10], team[50];
            cin >> ws;
            cin.getline(name, 50);
            cin.getline(birthdate, 10);
            cin.getline(team, 50);
            insert_s(name, birthdate, team);
        }
        else if (command == "get-m") {
            int team_id;
            cin >> team_id;
            get_m(team_id);
        }
        else if (command == "get-s") {
            int player_id;
            cin >> player_id;
            get_s(player_id);
        }
        else if (command == "del-m") {
            int team_id;
            cin >> team_id;
            del_m(team_id);
        }
        else if (command == "del-s") {
            int player_id;
            cin >> player_id;
            del_s(player_id);
        }
        else if (command == "ut-m") {
            ut_m();
        }
        else if (command == "ut-s") {
            ut_s();
        }
        else if (command == "calc-m") {
            calc_m();
        }
        else if (command == "calc-s") {
            calc_s();
        }
        else if (command == "update-m") {
            int team_id;
            string field, new_value;
            cin >> team_id >> field >> new_value;
            update_m(team_id, field, new_value);
        }
        else if (command == "update-s") {
            int player_id;
            string field, new_value;
            cin >> player_id >> field >> new_value;
            update_s(player_id, field, new_value);
        }
        else {
            cout << "Unknown command.\n";
        }
    }
}