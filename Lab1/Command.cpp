#include "index.h"
#include "garbage.h"
#include "team.h"
#include "player.h"

using namespace std;

const char* MASTER_FILE = "Team.fl";
const char* SLAVE_FILE = "Players.fl";
const char* TEAM_INDEX_FILE = "Team.ind";
const char* PLAYER_INDEX_FILE = "Players.ind";
const char* TEAM_GARBAGE_FILE = "TeamGarbage.bin";
const char* PLAYER_GARBAGE_FILE = "PlayerGarbage.bin";

// Окремі індексні таблиці для команд і гравців
IndexTable team_index_table;
IndexTable player_index_table;
void insert_m(const char* name, const char* trainer_name)
{
    int id = generate_id(team_index_table);

    fstream file(MASTER_FILE, ios::binary | ios::app);
    Team record = { id, "", "", -1, false };
    strcpy_s(record.name, sizeof(record.name), name);
    strcpy_s(record.trainer_name, sizeof(record.trainer_name), trainer_name);

    file.seekp(0, ios::end);
    int pos = file.tellp() / sizeof(Team);
    file.write(reinterpret_cast<char*>(&record), sizeof(Team));
    file.close();

    team_index_table.records[id] = pos;
    save_index(TEAM_INDEX_FILE, team_index_table);

    printf("Inserted master record\nTeam: ID(%d), %s, %s\n", id, name, trainer_name);
}

void insert_s(const char* name, const char* birthdate, const char* team_name)
{
    int team_id = get_team_id_by_name(team_name);
    if (team_id == -1) {
        printf("Error: Team \"%s\" not found.\n", team_name);
        return;
    }

    int player_id = generate_id(player_index_table);

    fstream file(SLAVE_FILE, ios::binary | ios::app);
    Player record = { player_id, team_id, "","", - 1, false};
    strcpy_s(record.name, sizeof(record.name), name);
    strcpy_s(record.birthdate, sizeof(record.birthdate), birthdate);

    file.seekp(0, ios::end);
    int pos = file.tellp() / sizeof(Player);
    file.write(reinterpret_cast<char*>(&record), sizeof(Player));
    file.close();

    player_index_table.records[player_id] = pos;
    save_index(PLAYER_INDEX_FILE, player_index_table);

    printf("Inserted player record\nPlayer: ID(%d), %s, %s, Team ID: %d\n", player_id, name, birthdate, team_id);
}

/*void get_m(int team_id) {
    if (team_index_table.records.find(team_id) == team_index_table.records.end()) {
        cout << "Error: Team with ID " << team_id << " not found.\n";
        return;
    }

    fstream file(MASTER_FILE, ios::binary | ios::in);
    if (!file) {
        cout << "Error: Unable to open master file.\n";
        return;
    }

    int pos = team_index_table.records[team_id];
    file.seekg(pos * sizeof(Team), ios::beg);

    Team record;
    file.read(reinterpret_cast<char*>(&record), sizeof(Team));
    file.close();

    if (record.is_deleted) {
        cout << "Error: This team record has been deleted.\n";
        return;
    }

    cout << "Team ID: " << record.id << "\n";
    cout << "Name: " << record.name << "\n";
    cout << "Trainer: " << record.trainer_name << "\n";
}*/
void get_m(int team_id) {
    if (team_index_table.records.find(team_id) == team_index_table.records.end()) {
        cout << "Error: Team with ID " << team_id << " not found.\n";
        return;
    }

    fstream teamFile(MASTER_FILE, ios::binary | ios::in);
    if (!teamFile) {
        cout << "Error: Unable to open master file.\n";
        return;
    }

    int pos = team_index_table.records[team_id];
    teamFile.seekg(pos * sizeof(Team), ios::beg);

    Team team;
    teamFile.read(reinterpret_cast<char*>(&team), sizeof(Team));
    teamFile.close();

    if (team.is_deleted) {
        cout << "Error: This team record has been deleted.\n";
        return;
    }

    // Вивід інформації про команду
    cout << "Team ID: " << team.id << "\n";
    cout << "Name: " << team.name << "\n";
    cout << "Trainer: " << team.trainer_name << "\n";

    // Вивід гравців цієї команди
    fstream playerFile(SLAVE_FILE, ios::binary | ios::in);
    if (!playerFile) {
        cout << "Error: Unable to open player file.\n";
        return;
    }

    bool hasPlayers = false;
    cout << "Players:\n";

    for (const auto& entry : player_index_table.records) {
        int player_pos = entry.second;
        playerFile.seekg(player_pos * sizeof(Player), ios::beg);

        Player player;
        playerFile.read(reinterpret_cast<char*>(&player), sizeof(Player));

        if (!player.is_deleted && player.team_id == team.id) {
            hasPlayers = true;
            printf("ID: %d Name: %-10s Birthdate: %s\n",
                player.passport_code, player.name, player.birthdate);
        }
    }

    if (!hasPlayers) {
        cout << "No players found for this team.\n";
    }
    playerFile.close();
}


void get_s(int player_id) {
    if (player_index_table.records.find(player_id) == player_index_table.records.end()) {
        cout << "Error: Player with ID " << player_id << " not found.\n";
        return;
    }

    fstream file(SLAVE_FILE, ios::binary | ios::in);
    if (!file) {
        cout << "Error: Unable to open player file.\n";
        return;
    }

    int pos = player_index_table.records[player_id];
    file.seekg(pos * sizeof(Player), ios::beg);

    Player record;
    file.read(reinterpret_cast<char*>(&record), sizeof(Player));
    file.close();

    if (record.is_deleted) {
        cout << "Error: This player record has been deleted.\n";
        return;
    }

    cout << "Player ID: " << record.passport_code << "\n";
    cout << "Name: " << record.name << "\n";
    cout << "Birthdate: " << record.birthdate << "\n";
    cout << "Team ID: " << record.team_id << "\n";
}

void del_s(int player_id) {
    // Check if the player exists
    if (player_index_table.records.find(player_id) == player_index_table.records.end()) {
        cout << "Error: Player with ID " << player_id << " not found.\n";
        return;
    }

    fstream file(SLAVE_FILE, ios::binary | ios::in | ios::out);
    if (!file) {
        cout << "Error: Unable to open player file.\n";
        return;
    }

    // Get the position of the player record
    int pos = player_index_table.records[player_id];
    file.seekg(pos * sizeof(Player), ios::beg);

    Player player;
    file.read(reinterpret_cast<char*>(&player), sizeof(Player));

    // If already deleted, exit
    if (player.is_deleted) {
        cout << "Error: Player is already deleted.\n";
        return;
    }

    // Mark player as deleted
    player.is_deleted = true;
    file.seekp(pos * sizeof(Player), ios::beg);
    file.write(reinterpret_cast<char*>(&player), sizeof(Player));

    // Move last record to this position
    int last_pos = (player_index_table.records.size() - 1) * sizeof(Player);
    if (pos != last_pos) {
        file.seekg(last_pos, ios::beg);
        Player last_player;
        file.read(reinterpret_cast<char*>(&last_player), sizeof(Player));

        file.seekp(pos * sizeof(Player), ios::beg);
        file.write(reinterpret_cast<char*>(&last_player), sizeof(Player));

        // Update index
        player_index_table.records[last_player.passport_code] = pos;
    }

    // Remove from index and add to garbage queue
    player_index_table.records.erase(player_id);
    player_index_table.free_ids.push(player_id);

    file.close();
    save_index(PLAYER_INDEX_FILE, player_index_table);
    save_trash(PLAYER_GARBAGE_FILE, player_index_table.free_ids);

    cout << "Player ID " << player_id << " has been deleted.\n";
}
void del_m(int team_id) {
    // Check if the team exists
    if (team_index_table.records.find(team_id) == team_index_table.records.end()) {
        cout << "Error: Team with ID " << team_id << " not found.\n";
        return;
    }

    fstream file(MASTER_FILE, ios::binary | ios::in | ios::out);
    if (!file) {
        cout << "Error: Unable to open master file.\n";
        return;
    }

    // Get the position of the team record
    int pos = team_index_table.records[team_id];
    file.seekg(pos * sizeof(Team), ios::beg);

    Team team;
    file.read(reinterpret_cast<char*>(&team), sizeof(Team));

    // If already deleted, exit
    if (team.is_deleted) {
        cout << "Error: Team is already deleted.\n";
        return;
    }

    // Delete all players associated with this team
    vector<int> players_to_delete;
    for (const auto& entry : player_index_table.records) {
        if (entry.second == team.id) {
            players_to_delete.push_back(entry.first);
        }
    }

    for (int player_id : players_to_delete) {
        del_s(player_id);
    }

    // Mark team as deleted
    team.is_deleted = true;
    file.seekp(pos * sizeof(Team), ios::beg);
    file.write(reinterpret_cast<char*>(&team), sizeof(Team));

    // Move last record to this position
    int last_pos = (team_index_table.records.size() - 1) * sizeof(Team);
    if (pos != last_pos) {
        file.seekg(last_pos, ios::beg);
        Team last_team;
        file.read(reinterpret_cast<char*>(&last_team), sizeof(Team));

        file.seekp(pos * sizeof(Team), ios::beg);
        file.write(reinterpret_cast<char*>(&last_team), sizeof(Team));

        // Update index
        team_index_table.records[last_team.id] = pos;
    }

    // Remove from index and add to garbage queue
    team_index_table.records.erase(team_id);
    team_index_table.free_ids.push(team_id);

    file.close();
    save_index(TEAM_INDEX_FILE, team_index_table);
    save_trash(TEAM_GARBAGE_FILE, team_index_table.free_ids);

    cout << "Team ID " << team_id << " has been deleted.\n";
}

void ut_m() {
    fstream file(MASTER_FILE, ios::binary | ios::in);
    if (!file) {
        cout << "Error: Unable to open master file.\n";
        return;
    }
    cout << "ID\tName\t\tTrainer\t\tDeleted\n";

    Team team;
    while (file.read(reinterpret_cast<char*>(&team), sizeof(Team))) {
        printf("%d\t%-15s\t %-15s\t%s\n",
            team.id, team.name, team.trainer_name,
            team.is_deleted ? "Yes" : "No");
    }
    file.close();
}
void ut_s() {
    fstream file(SLAVE_FILE, ios::binary | ios::in);
    if (!file) {
        cout << "Error: Unable to open player file.\n";
        return;
    }

    cout << "ID\tName\t\tBirthdate\tTeam ID\tDeleted\n";

    Player player;
    while (file.read(reinterpret_cast<char*>(&player), sizeof(Player))) {
        printf("%d\t%-15s\t %-10s\t %-6d\t %s\n",
            player.passport_code, player.name, player.birthdate,
            player.team_id, player.is_deleted ? "Yes" : "No");
    }
    file.close();
}
void update_m(int team_id, const std::string& field, const std::string& new_value) {
    if (team_index_table.records.find(team_id) == team_index_table.records.end()) {
        cout << "Error: Team with ID " << team_id << " not found.\n";
        return;
    }

    fstream file(MASTER_FILE, ios::binary | ios::in | ios::out);
    if (!file) {
        cout << "Error: Unable to open master file.\n";
        return;
    }

    // Знаходимо позицію команди у файлі
    int pos = team_index_table.records[team_id];
    file.seekg(pos * sizeof(Team), ios::beg);

    Team record;
    file.read(reinterpret_cast<char*>(&record), sizeof(Team));

    // Перевірка чи запис не видалений
    if (record.is_deleted) {
        cout << "Error: This team record has been deleted.\n";
        return;
    }

    // Оновлення відповідного поля
    if (field == "name") {
        strncpy_s(record.name, new_value.c_str(), sizeof(record.name) - 1);
    }
    else if (field == "trainer_name") {
        strncpy_s(record.trainer_name, new_value.c_str(), sizeof(record.trainer_name) - 1);
    }
    else {
        cout << "Error: Invalid field name.\n";
        return;
    }

    // Запис зміненого запису назад у файл
    file.seekp(pos * sizeof(Team), ios::beg);
    file.write(reinterpret_cast<char*>(&record), sizeof(Team));
    file.close();

    cout << "Team ID " << team_id << " updated successfully.\n";
}

void update_s(int player_id, const std::string& field, const std::string& new_value) {
    if (player_index_table.records.find(player_id) == player_index_table.records.end()) {
        cout << "Error: Player with ID " << player_id << " not found.\n";
        return;
    }

    fstream file(SLAVE_FILE, ios::binary | ios::in | ios::out);
    if (!file) {
        cout << "Error: Unable to open player file.\n";
        return;
    }

    // Знаходимо позицію гравця у файлі
    int pos = player_index_table.records[player_id];
    file.seekg(pos * sizeof(Player), ios::beg);

    Player record;
    file.read(reinterpret_cast<char*>(&record), sizeof(Player));

    // Перевірка чи запис не видалений
    if (record.is_deleted) {
        cout << "Error: This player record has been deleted.\n";
        return;
    }

    // Оновлення відповідного поля
    if (field == "name") {
        strncpy_s(record.name, new_value.c_str(), sizeof(record.name) - 1);
    }
    else if (field == "birthdate") {
        strncpy_s(record.birthdate, new_value.c_str(), sizeof(record.birthdate) - 1);
    }
    else {
        cout << "Error: Invalid field name.\n";
        return;
    }

    // Запис зміненого запису назад у файл
    file.seekp(pos * sizeof(Player), ios::beg);
    file.write(reinterpret_cast<char*>(&record), sizeof(Player));
    file.close();

    cout << "Player ID " << player_id << " updated successfully.\n";
}
void calc_s() {
    fstream file(SLAVE_FILE, ios::binary | ios::in);
    if (!file) {
        cout << "Error: Unable to open player file.\n";
        return;
    }

    int total_players = 0;
    Player player;

    while (file.read(reinterpret_cast<char*>(&player), sizeof(Player))) {
        if (!player.is_deleted) {
            total_players++;
        }
    }
    cout << "Total players: " << total_players << "\n";

    file.close();
}
void calc_m() {
    fstream teamFile(MASTER_FILE, ios::binary | ios::in);
    if (!teamFile) {
        cout << "Error: Unable to open master file.\n";
        return;
    }

    fstream playerFile(SLAVE_FILE, ios::binary | ios::in);
    if (!playerFile) {
        cout << "Error: Unable to open player file.\n";
        teamFile.close();
        return;
    }

    int total_teams = 0;
    int total_players = 0;

    Team team;
    while (teamFile.read(reinterpret_cast<char*>(&team), sizeof(Team))) {
        if (team.is_deleted) continue;  // Пропускаємо видалені команди

        total_teams++;
        int team_players_count = 0;

        for (const auto& entry : player_index_table.records) {
            int pos = entry.second;
            playerFile.seekg(pos * sizeof(Player), ios::beg);

            Player player;
            playerFile.read(reinterpret_cast<char*>(&player), sizeof(Player));

            if (!player.is_deleted && player.team_id == team.id) {
                team_players_count++;
                total_players++;
            }
        }

        printf("Team ID: %d Name: %-15s Players: %d\n", team.id, team.name, team_players_count);
    }
    cout << "Total teams: " << total_teams << "\n";
    cout << "Total players: " << total_players << "\n";

    teamFile.close();
    playerFile.close();
}

void initialize()
{
    load_index(TEAM_INDEX_FILE, team_index_table);
    load_index(PLAYER_INDEX_FILE, player_index_table);
    load_trash(TEAM_GARBAGE_FILE, team_index_table.free_ids);
    load_trash(PLAYER_GARBAGE_FILE, player_index_table.free_ids);
}