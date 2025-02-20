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

IndexTable team_index_table;
IndexTable player_index_table;

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
        cout << "Error: This team record has been _meted.\n";
        return;
    }

    cout << "Team ID: " << team.id << "\n";
    cout << "Name: " << team.name << "\n";
    cout << "Trainer: " << team.trainer_name << "\n";

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

void initialize_files() {
    std::fstream file;

    // Перевірка і створення файлу для команд
    file.open(MASTER_FILE, std::ios::binary | std::ios::in);
    if (!file) {
        //std::cout << "Creating " << MASTER_FILE << "...\n";
        file.open(MASTER_FILE, std::ios::binary | std::ios::out);
    }
    file.close();

    // Перевірка і створення файлу для гравців
    file.open(SLAVE_FILE, std::ios::binary | std::ios::in);
    if (!file) {
        //std::cout << "Creating " << SLAVE_FILE << "...\n";
        file.open(SLAVE_FILE, std::ios::binary | std::ios::out);
    }
    file.close();
}
void insert_m(const char* name, const char* trainer_name) {
    initialize_files();
    int id;

    fstream file(MASTER_FILE, ios::binary | ios::in | ios::out);

    int pos;
    if (!team_index_table.free_ids.empty()) {
        // Reuse free ID and its position
        id = team_index_table.free_ids.front();
        team_index_table.free_ids.pop();
        pos = team_index_table.records[id]; // Get position from index table
    }
    else {
        // Generate a new ID and append
        id = generate_id(team_index_table);
        file.seekp(0, ios::end);
        pos = file.tellp() / sizeof(Team);
    }

    // Create new team record
    Team record = { id, "", "", -1, false };
    strcpy_s(record.name, sizeof(record.name), name);
    strcpy_s(record.trainer_name, sizeof(record.trainer_name), trainer_name);

    // Write the record at the correct position
    file.seekp(pos * sizeof(Team));
    file.write(reinterpret_cast<char*>(&record), sizeof(Team));
    file.close();

    team_index_table.records[id] = pos;
    save_index(TEAM_INDEX_FILE, team_index_table);

    printf("Inserted master record\nTeam: ID(%d), %s, %s\n", id, name, trainer_name);
}
void insert_s(const char* name, const char* birthdate, const char* team_name) {
    int team_id = get_team_id_by_name(team_name);
    if (team_id == -1) {
        printf("Error: Team \"%s\" not found.\n", team_name);
        return;
    }

    int player_id;
    fstream file(SLAVE_FILE, ios::binary | ios::in | ios::out);
    fstream master_file(MASTER_FILE, ios::binary | ios::in | ios::out);

    int pos;
    if (!player_index_table.free_ids.empty()) {
        // Reuse a deleted player slot
        player_id = player_index_table.free_ids.front();
        player_index_table.free_ids.pop();
        pos = player_index_table.records[player_id]; // Get position from index table
    }
    else {
        // Generate new player ID and append
        player_id = generate_id(player_index_table);
        file.seekp(0, ios::end);
        pos = file.tellp() / sizeof(Player);
    }

    // Create new player record
    Player new_player = { player_id, team_id, "", "", -1, -1, false };
    strcpy_s(new_player.name, sizeof(new_player.name), name);
    strcpy_s(new_player.birthdate, sizeof(new_player.birthdate), birthdate);

    // Read team to update first_player if necessary
    int team_pos = team_index_table.records[team_id];
    master_file.seekg(team_pos * sizeof(Team));
    Team team;
    master_file.read(reinterpret_cast<char*>(&team), sizeof(Team));

    if (team.first_player == -1) {
        // No players exist for this team
        team.first_player = player_id;
    }
    else {
        // Find the last player and link it
        int last_pos = team.first_player;
        Player last_player;
        while (last_pos != -1) {
            int last_file_pos = player_index_table.records[last_pos];
            file.seekg(last_file_pos * sizeof(Player));
            file.read(reinterpret_cast<char*>(&last_player), sizeof(Player));
            if (last_player.next_player == -1) break;
            last_pos = last_player.next_player;
        }

        last_player.next_player = player_id;
        file.seekp(player_index_table.records[last_pos] * sizeof(Player));
        file.write(reinterpret_cast<char*>(&last_player), sizeof(Player));

        new_player.prev_player = last_player.passport_code;
    }

    // Write the player at the correct position
    file.seekp(pos * sizeof(Player));
    file.write(reinterpret_cast<char*>(&new_player), sizeof(Player));

    // Save updated team record
    master_file.seekp(team_pos * sizeof(Team));
    master_file.write(reinterpret_cast<char*>(&team), sizeof(Team));

    file.close();
    master_file.close();

    player_index_table.records[player_id] = pos;
    save_index(PLAYER_INDEX_FILE, player_index_table);
    printf("Inserted player record\nPlayer: ID(%d), %s, %s, Team ID: %d\n", player_id, name, birthdate, team_id);
}

/*void insert_m(const char* name, const char* trainer_name) {
    initialize_files();
    int id = generate_id(team_index_table);

    fstream file(MASTER_FILE, ios::binary | ios::in | ios::out);
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

void insert_s(const char* name, const char* birthdate, const char* team_name) {
    int team_id = get_team_id_by_name(team_name);
    if (team_id == -1) {
        printf("Error: Team \"%s\" not found.\n", team_name);
        return;
    }

    int player_id = generate_id(player_index_table);
    Player new_player = { player_id, team_id, "", "", -1, -1, false };
    strcpy_s(new_player.name, sizeof(new_player.name), name);
    strcpy_s(new_player.birthdate, sizeof(new_player.birthdate), birthdate);

    fstream file(SLAVE_FILE, ios::binary | ios::in | ios::out);
    fstream master_file(MASTER_FILE, ios::binary | ios::in | ios::out);

    int team_pos = team_index_table.records[team_id];
    master_file.seekg(team_pos * sizeof(Team));
    Team team;
    master_file.read(reinterpret_cast<char*>(&team), sizeof(Team));

    int pos;
    if (!player_index_table.free_ids.empty()) {
        pos = player_index_table.free_ids.front();
        player_index_table.free_ids.pop();
    }
    else {
        file.seekp(0, ios::end);
        pos = file.tellp() / sizeof(Player);
    }

    if (team.first_player == -1) {
        // No players exist for this team, this is the first one
        team.first_player = player_id;  // Store the player ID
    }
    else {
        int last_pos = team.first_player;
        Player last_player;
        while (last_pos != -1) {
            file.seekg(player_index_table.records[last_pos] * sizeof(Player));
            file.read(reinterpret_cast<char*>(&last_player), sizeof(Player));
            if (last_player.next_player == -1) break;
            last_pos = last_player.next_player;
        }

        last_player.next_player = player_id;
        file.seekp(player_index_table.records[last_pos] * sizeof(Player));
        file.write(reinterpret_cast<char*>(&last_player), sizeof(Player));

        new_player.prev_player = last_player.passport_code;
    }

    file.seekp(pos * sizeof(Player));
    file.write(reinterpret_cast<char*>(&new_player), sizeof(Player));

    master_file.seekp(team_pos * sizeof(Team));
    master_file.write(reinterpret_cast<char*>(&team), sizeof(Team));

    file.close();
    master_file.close();

    player_index_table.records[player_id] = pos;
    save_index(PLAYER_INDEX_FILE, player_index_table);
    printf("Inserted player record\nPlayer: ID(%d), %s, %s, Team ID: %d\n", player_id, name, birthdate, team_id);
}*/

void del_s(int player_id) {
    if (player_index_table.records.find(player_id) == player_index_table.records.end()) {
        cout << "Error: Player with ID " << player_id << " not found.\n";
        return;
    }

    fstream file(SLAVE_FILE, ios::binary | ios::in | ios::out);
    int pos = player_index_table.records[player_id];
    file.seekg(pos * sizeof(Player), ios::beg);

    Player player;
    file.read(reinterpret_cast<char*>(&player), sizeof(Player));

    if (player.is_deleted) {
        cout << "Error: Player is already deleted.\n";
        return;
    }

    fstream master_file(MASTER_FILE, ios::binary | ios::in | ios::out);
    int team_pos = team_index_table.records[player.team_id];
    master_file.seekg(team_pos * sizeof(Team));

    Team team;
    master_file.read(reinterpret_cast<char*>(&team), sizeof(Team));

    // **1?? Update first_player in Team if needed**
    if (team.first_player == player_id) {
        team.first_player = (player.next_player == -1) ? -1 : player.next_player;
    }

    // **2?? Update prev_player and next_player links**
    if (player.prev_player != -1) {
        int prev_pos = player_index_table.records[player.prev_player];
        file.seekg(prev_pos * sizeof(Player));
        Player prev_player;
        file.read(reinterpret_cast<char*>(&prev_player), sizeof(Player));
        prev_player.next_player = player.next_player;  // Link previous to next
        file.seekp(prev_pos * sizeof(Player));
        file.write(reinterpret_cast<char*>(&prev_player), sizeof(Player));
    }

    if (player.next_player != -1) {
        int next_pos = player_index_table.records[player.next_player];
        file.seekg(next_pos * sizeof(Player));
        Player next_player;
        file.read(reinterpret_cast<char*>(&next_player), sizeof(Player));
        next_player.prev_player = player.prev_player;  // Link next to previous
        file.seekp(next_pos * sizeof(Player));
        file.write(reinterpret_cast<char*>(&next_player), sizeof(Player));
    }

    // **3?? Mark player as deleted**
    player.is_deleted = true;
    file.seekp(pos * sizeof(Player));
    file.write(reinterpret_cast<char*>(&player), sizeof(Player));

    // **4?? Save updated team**
    master_file.seekp(team_pos * sizeof(Team));
    master_file.write(reinterpret_cast<char*>(&team), sizeof(Team));

    master_file.close();
    file.close();

    // **5?? Free the ID and add position to garbage collection**
    player_index_table.records.erase(player_id);
    player_index_table.free_ids.push(player_id);

    save_index(PLAYER_INDEX_FILE, player_index_table);
    save_trash(PLAYER_GARBAGE_FILE, player_index_table.free_ids);

    cout << "Player ID " << player_id << " has been deleted.\n";
}
void del_m(int team_id) {
    if (team_index_table.records.find(team_id) == team_index_table.records.end()) {
        cout << "Error: Team with ID " << team_id << " not found.\n";
        return;
    }

    fstream teamsData(MASTER_FILE, ios::binary | ios::in | ios::out);
    int pos = team_index_table.records[team_id];
    teamsData.seekg(pos * sizeof(Team), ios::beg);

    Team team;
    teamsData.read(reinterpret_cast<char*>(&team), sizeof(Team));

    if (team.is_deleted) {
        cout << "Error: This team record has been deleted.\n";
        teamsData.close();
        return;
    }

    // **Properly delete all linked players**
    fstream playersData(SLAVE_FILE, ios::binary | ios::in | ios::out);
    int player_pos = team.first_player;
    while (player_pos != -1) {
        if (player_index_table.records.find(player_pos) == player_index_table.records.end()) {
            cout << "Error: Player position not found in index table.\n";
            break;  // Prevent infinite loop
        }

        int file_pos = player_index_table.records[player_pos];  // Ensure this is a file position
        playersData.seekg(file_pos * sizeof(Player));
        Player player;
        playersData.read(reinterpret_cast<char*>(&player), sizeof(Player));

        int next_pos = (player.next_player == -1) ? -1 : player.next_player;  // Store next player ID, NOT position

        del_s(player_pos);  // Pass correct ID, `del_s` will find position

        player_pos = next_pos;  // Move to next player
    }
    playersData.close();


    // **Mark the team as deleted**
    team.is_deleted = true;
    team.first_player = -1;  // Remove reference to first player
    teamsData.seekp(pos * sizeof(Team));
    teamsData.write(reinterpret_cast<char*>(&team), sizeof(Team));

    // **Remove team from index and add to garbage collection**
    team_index_table.records.erase(team_id);
    team_index_table.free_ids.push(team_id);

    teamsData.close();
    save_index(TEAM_INDEX_FILE, team_index_table);
    save_trash(TEAM_GARBAGE_FILE, team_index_table.free_ids);

    cout << "Team ID " << team_id << " and all its players have been deleted.\n";
}


void ut_m() {
    fstream file(MASTER_FILE, ios::binary | ios::in);
    if (!file) {
        cout << "Error: Unable to open master file.\n";
        return;
    }
    cout << "ID\tName\t\tTrainer\t\tfirst player\tDeleted\n";

    Team team;
    while (file.read(reinterpret_cast<char*>(&team), sizeof(Team))) {
        printf("%-6d %-20s %-20s %-13d %s\n",
            team.id, team.name, team.trainer_name, team.first_player,
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

    cout << "ID\tName\t\tBirthdate\tTeam ID\tprev player\tnext player\tDeleted\n";

    Player player;
    while (file.read(reinterpret_cast<char*>(&player), sizeof(Player))) {
        printf("%-6d %-20s %-12s %-8d %-12d %-12d %s\n",
            player.passport_code, player.name, player.birthdate,
            player.team_id, player.prev_player, player.next_player,
            player.is_deleted ? "Yes" : "No");
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

    int pos = team_index_table.records[team_id];
    file.seekg(pos * sizeof(Team), ios::beg);

    Team record;
    file.read(reinterpret_cast<char*>(&record), sizeof(Team));

    if (record.is_deleted) {
        cout << "Error: This team record has been deleted.\n";
        return;
    }

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

    int pos = player_index_table.records[player_id];
    file.seekg(pos * sizeof(Player), ios::beg);

    Player record;
    file.read(reinterpret_cast<char*>(&record), sizeof(Player));

    if (record.is_deleted) {
        cout << "Error: This player record has been deleted.\n";
        return;
    }

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
        if (team.is_deleted) continue;

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
void help() {
    cout << "\nAVAILABLE COMMANDS\n";
    cout << "insert-m    - Add a new team (name, trainer)\n";
    cout << "insert-s    - Add a new player (name, birthdate, team)\n";
    cout << "get-m       - Get team details by ID (including players)\n";
    cout << "get-s       - Get player details by ID\n";
    cout << "del-m       - Delete a team by ID (removes all its players too)\n";
    cout << "del-s       - Delete a player by ID\n";
    cout << "ut-m        - Show all teams (including deleted ones)\n";
    cout << "ut-s        - Show all players (including deleted ones)\n";
    cout << "calc-m      - Count all teams and display number of players per team\n";
    cout << "calc-s      - Count total number of players\n";
    cout << "update-m    - Update a team's field (name/trainer)\n";
    cout << "update-s    - Update a player's field (name/birthdate)\n";
    cout << "help        - Show this help menu\n";
    cout << "exit        - Exit the program\n";
}
