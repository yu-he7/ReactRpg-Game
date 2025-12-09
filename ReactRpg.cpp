#include <iostream>
#include <algorithm>
#include <deque>
#include <vector>
#include <windows.h>
#include <conio.h>
#include <string>
#include <cstdlib>
#include <chrono>
#include <thread>
#include "sqlite3.h"


#define SCREEN_WIDTH 200
#define SCREEN_HEIGHT 100

using namespace std;

struct Character {
    string name;
    int level;
    int health;
    int attackPower;
    int experience;
    int experienceToNextLevel;
};

struct Monster {
    string name;
    int level;
    int health;
    int attackPower;
};

Character player = { "Hero", 1, 100, 20, 0, 50 };
Monster monsters[] = {
    // 몬스터 아이디어 (Monster)
    { "Goblin", 1, 30, 5 },            // 레벨 1 고블린, 체력 30, 공격력 5
    { "Troll", 3, 80, 15 },            // 레벨 3 트롤, 체력 80, 공격력 15
    { "Zombie", 2, 40, 10 },           // 레벨 2 좀비, 체력 40, 공격력 10
    { "Orc", 4, 100, 20 },             // 레벨 4 오크, 체력 100, 공격력 20
    { "Dragon", 5, 150, 30 },          // 레벨 5 드래곤, 체력 150, 공격력 30
    { "Fire Elemental", 6, 180, 40 },  // 레벨 6 불의 정수, 체력 180, 공격력 40
    { "Vampire", 7, 120, 25 },         // 레벨 7 뱀파이어, 체력 120, 공격력 25
    { "Griffin", 8, 200, 50 },         // 레벨 8 그리핀, 체력 200, 공격력 50
    { "Hydra", 10, 250, 60 },          // 레벨 10 히드라, 체력 250, 공격력 60
    { "Necromancer", 9, 150, 35 },     // 레벨 9 네크로맨서, 체력 150, 공격력 35
    { "Minotaur", 6, 160, 30 },        // 레벨 6 미노타우르, 체력 160, 공격력 30
    { "Phoenix", 12, 300, 70 },        // 레벨 12 피닉스, 체력 300, 공격력 70
    { "Grim Reaper", 15, 350, 80 },    // 레벨 15 그림 리퍼, 체력 350, 공격력 80
    { "Basilisk", 7, 140, 28 },        // 레벨 7 바실리스크, 체력 140, 공격력 28

};

enum class ItemType { Weapon, Armor, Accessory };



struct Item {
    string name;           // 아이템 이름
    string description;    // 아이템 설명
    ItemType type;         // 아이템 유형 (무기, 방어구, 장신구 등)
    int Power;         // 아이템이 제공하는 능력치 보너스
};

Item items[] = {
    // 무기 (Weapon) - 이전에 조정한 무기들 유지
    { "Iron Sword", "A sturdy sword that adds extra strength.", ItemType::Weapon, 12 },
    { "Steel Sword", "A sharper blade made from steel.", ItemType::Weapon, 18 },
    { "Flame Sword", "A sword engulfed in flames, adds burn damage.", ItemType::Weapon, 24 },
    { "Lightning Dagger", "A dagger infused with electric energy, adds shock damage.", ItemType::Weapon, 28 },
    { "Dark Blade", "A cursed sword that increases attack power significantly.", ItemType::Weapon, 35 },
    { "Frozen Spear", "A spear imbued with frost magic, slows enemies.", ItemType::Weapon, 40 },
    { "Holy Lance", "A lance blessed by the gods, capable of piercing any defense.", ItemType::Weapon, 50 },
    { "Demonic Scythe", "A scythe forged in darkness, extremely powerful.", ItemType::Weapon, 60 },

    // 방어구 (Armor) - 방어력 점진적으로 증가
    { "Iron Armor", "Basic armor providing minimal protection.", ItemType::Armor, 10 },
    { "Steel Armor", "Strong armor increasing defense significantly.", ItemType::Armor, 18 },
    { "Ethereal Robe", "A robe that grants agility and magic resistance.", ItemType::Armor, 25 },
    { "Dragon Scale Armor", "Armor made from dragon scales, provides high resistance.", ItemType::Armor, 35 },
    { "Divine Shield", "A shield that nullifies dark attacks.", ItemType::Armor, 45 },

    // 장신구 (Accessory) - 보조 능력치 증가
    { "Ring of Strength", "A ring that slightly increases strength.", ItemType::Accessory, 5 },
    { "Steal Ring of Strength", "A ring that greatly boosts strength.", ItemType::Accessory, 10 },
    { "Amulet of Wisdom", "A mystical amulet that enhances intelligence and magic resistance.", ItemType::Accessory, 15 },
    { "Cursed Pendant", "A pendant with a dark aura, greatly increasing attack power but reducing defense.", ItemType::Accessory, 20 },
    { "Divine Amulet", "A legendary amulet that balances attack and defense power.", ItemType::Accessory, 30 }
};


struct Equipment {
    Item* weapon = nullptr;    // 무기
    Item* armor = nullptr;     // 방어구
    Item* accessory = nullptr; // 장신구
};

Equipment playerEquipment; // 플레이어의 장비


struct PlayerInventory {
    vector<Item> items;
};

PlayerInventory playerInventory;

void initializeDatabase(sqlite3*& db) {
    int rc = sqlite3_open("user_database.db", &db);

    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        exit(rc);
    }

    const char* create_users_table_query = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL UNIQUE,
            password TEXT NOT NULL
        );
    )";

    const char* create_player_table_query = R"(
        CREATE TABLE IF NOT EXISTS player_info (
            username TEXT NOT NULL UNIQUE,
            level INTEGER NOT NULL,
            health INTEGER NOT NULL,
            attack_power INTEGER NOT NULL,
            experience INTEGER NOT NULL,
            experience_to_next_level INTEGER NOT NULL
        );
    )";

    const char* create_player_items_table_query = R"(
        CREATE TABLE IF NOT EXISTS player_items (
            username TEXT NOT NULL,
            item_name TEXT NOT NULL,
            item_description TEXT NOT NULL,
            item_type INTEGER NOT NULL,
            item_power INTEGER NOT NULL,
            FOREIGN KEY (username) REFERENCES users(username)
        );
    )";

    char* errorMessage;

    rc = sqlite3_exec(db, create_users_table_query, 0, 0, &errorMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "Error creating users table: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
        sqlite3_close(db);
        exit(rc);
    }

    rc = sqlite3_exec(db, create_player_table_query, 0, 0, &errorMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "Error creating player_info table: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
        sqlite3_close(db);
        exit(rc);
    }

    rc = sqlite3_exec(db, create_player_items_table_query, 0, 0, &errorMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "Error creating player_items table: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
        sqlite3_close(db);
        exit(rc);
    }
}
void addItemToInventory(const string& itemName, const string& itemDescription, ItemType itemType, int power) {
    Item newItem = { itemName, itemDescription, itemType, power };
    playerInventory.items.push_back(newItem);
   
}

void savePlayerInfo(sqlite3* db, const Character& player) {
    const char* insert_query = R"(
        INSERT INTO player_info (username, level, health, attack_power, experience, experience_to_next_level) 
        VALUES (?, ?, ?, ?, ?, ?)
        ON CONFLICT(username) DO UPDATE SET
            level=excluded.level,
            health=excluded.health,
            attack_power=excluded.attack_power,
            experience=excluded.experience,
            experience_to_next_level=excluded.experience_to_next_level;
    )";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, insert_query, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, player.name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, player.level);
        sqlite3_bind_int(stmt, 3, player.health);
        sqlite3_bind_int(stmt, 4, player.attackPower);
        sqlite3_bind_int(stmt, 5, player.experience);
        sqlite3_bind_int(stmt, 6, player.experienceToNextLevel);


        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Error saving player info: " << sqlite3_errmsg(db) << std::endl;
        }
    }
    else {
        std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
}

bool loadPlayerInfo(sqlite3* db, const std::string& username) {
    const char* select_query = "SELECT level, health, attack_power, experience, experience_to_next_level FROM player_info WHERE username = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, select_query, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        player.name = username;

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            player.level = sqlite3_column_int(stmt, 0);
            player.health = sqlite3_column_int(stmt, 1);
            player.attackPower = sqlite3_column_int(stmt, 2);
            player.experience = sqlite3_column_int(stmt, 3);
            player.experienceToNextLevel = sqlite3_column_int(stmt, 4);
            sqlite3_finalize(stmt);
            return true;
        }
    }
    

    sqlite3_finalize(stmt);
    return false; // 해당 사용자의 정보가 없을 경우
}


void savePlayerItems(sqlite3* db, const PlayerInventory& inventory, const std::string& username) {
    // 1. 기존 데이터 삭제
    const char* delete_query = "DELETE FROM player_items WHERE username = ?;";
    sqlite3_stmt* delete_stmt;

    if (sqlite3_prepare_v2(db, delete_query, -1, &delete_stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(delete_stmt, 1, username.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(delete_stmt) != SQLITE_DONE) {
            std::cerr << "Error deleting player items: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(delete_stmt);
    }
    else {
        std::cerr << "Error preparing delete statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    // 2. 새 데이터 삽입
    const char* insert_query = R"(
        INSERT INTO player_items (username, item_name, item_description, item_type, item_power) 
        VALUES (?, ?, ?, ?, ?);
    )";
    sqlite3_stmt* insert_stmt;

    for (const auto& item : inventory.items) {
        if (sqlite3_prepare_v2(db, insert_query, -1, &insert_stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(insert_stmt, 1, username.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(insert_stmt, 2, item.name.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(insert_stmt, 3, item.description.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(insert_stmt, 4, static_cast<int>(item.type));
            sqlite3_bind_int(insert_stmt, 5, item.Power);

            if (sqlite3_step(insert_stmt) != SQLITE_DONE) {
                std::cerr << "Error inserting player item: " << sqlite3_errmsg(db) << std::endl;
            }
            sqlite3_finalize(insert_stmt);
        }
        else {
            std::cerr << "Error preparing insert statement: " << sqlite3_errmsg(db) << std::endl;
        }
    }
}




bool loadPlayerItems(sqlite3* db, const std::string& username) {
    const char* select_query = "SELECT item_name, item_description, item_type, item_power FROM player_items WHERE username = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, select_query, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            string itemName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            string itemDescription = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            ItemType itemType = static_cast<ItemType>(sqlite3_column_int(stmt, 2));
            int itemPower = sqlite3_column_int(stmt, 3);

            // 아이템을 인벤토리에 추가
            addItemToInventory(itemName, itemDescription, itemType, itemPower);
        }
        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);
    return false; // 해당 사용자의 아이템 정보가 없을 경우
}


void registerUser(sqlite3* db, const std::string& username, const std::string& password) {
    const char* insert_query = "INSERT INTO users (username, password) VALUES (?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, insert_query, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC); // username 바인딩
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC); // password 바인딩

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            std::cout << "User registered successfully!" << std::endl;
            player.name = username;
            player.level = 1; // 기본 레벨
            player.health = 100; // 기본 체력
            player.attackPower = 20; // 기본 공격력
            player.experience = 0; // 기본 경험치
            player.experienceToNextLevel = 50; // 기본 레벨업에 필요한 경험치
            savePlayerInfo(db, player);
        }
        else {
            std::cerr << "Error registering user: " << sqlite3_errmsg(db) << std::endl;
        }
    }
    else {
        std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt); // 리소스 해제
}

bool loginUser(sqlite3* db, const std::string& username, const std::string& password) {
    const char* select_query = "SELECT password FROM users WHERE username = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, select_query, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC); // username 바인딩

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string stored_password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

            if (stored_password == password) { // 비밀번호 일치 확인
                sqlite3_finalize(stmt);
                return true;
            }
        }
    }

    sqlite3_finalize(stmt);
    return false;
}
//인벤토리 영역




//아이템 영역 
void equipItem(Item* item) {
    switch (item->type) {
    case ItemType::Weapon:
        if (playerEquipment.weapon != nullptr) {
            cout << "You already have a weapon equipped. Replacing it with " << item->name << ".\n";
            // 기존 무기의 Power 값을 빼고 새로운 무기의 Power 값을 더함
            player.attackPower -= playerEquipment.weapon->Power;
        }
        playerEquipment.weapon = item;
        player.attackPower += item->Power;  // 아이템의 Power 값만큼 공격력 증가
        cout << "You equipped " << item->name << " and gained " << item->Power << " attack power.\n";
        break;

    case ItemType::Armor:
        if (playerEquipment.armor != nullptr) {
            cout << "You already have armor equipped. Replacing it with " << item->name << ".\n";
            player.health -= playerEquipment.armor->Power;

        }
        playerEquipment.armor = item;
        player.health += item->Power;  // 아이템의 Power 값만큼 공격력 증가
        cout << "You equipped " << item->name << " and gained " << item->Power << " health.\n";
        break;

    case ItemType::Accessory:
        if (playerEquipment.accessory != nullptr) {
            cout << "You already have an accessory equipped. Replacing it with " << item->name << ".\n";
            player.attackPower -= playerEquipment.accessory->Power;

        }
        playerEquipment.accessory = item;
        player.attackPower += item->Power;  // 아이템의 Power 값만큼 공격력 증가
        cout << "You equipped " << item->name << " and gained " << item->Power << " attack power.\n";
        break;

    default:
        cout << "Invalid item type.\n";
        break;
    }
}

void dropItem() {
    int randNum = rand() % 100;  // 0~99 사이의 랜덤 숫자 생성

    if (player.level >= 1 && player.level <= 3) {
        if (randNum < 40) addItemToInventory("Iron Sword", "A sturdy sword", ItemType::Weapon, 12);
        else if (randNum < 70) addItemToInventory("Iron Armor", "Basic armor providing minimal protection.", ItemType::Armor, 10);
        else addItemToInventory("Ring of Strength", "A ring that slightly increases strength.", ItemType::Accessory, 5);
    }
    else if (player.level >= 4 && player.level <= 6) {
        if (randNum < 30) addItemToInventory("Steel Sword", "A sharper blade", ItemType::Weapon, 18);
        else if (randNum < 60) addItemToInventory("Steel Armor", "Strong armor increasing defense.", ItemType::Armor, 18);
        else addItemToInventory("Steal Ring of Strength", "A ring that greatly boosts strength.", ItemType::Accessory, 10);
    }
    else if (player.level >= 7 && player.level <= 9) {
        if (randNum < 30) addItemToInventory("Flame Sword", "A sword engulfed in flames", ItemType::Weapon, 24);
        else if (randNum < 60) addItemToInventory("Ethereal Robe", "A robe that grants agility and magic resistance.", ItemType::Armor, 25);
        else addItemToInventory("Amulet of Wisdom", "A mystical amulet that enhances intelligence and magic resistance.", ItemType::Accessory, 15);
    }
    else if (player.level >= 10 && player.level <= 12) {
        if (randNum < 30) addItemToInventory("Dark Blade", "A cursed sword", ItemType::Weapon, 35);
        else if (randNum < 60) addItemToInventory("Dragon Scale Armor", "Armor made from dragon scales, provides high resistance.", ItemType::Armor, 35);
        else addItemToInventory("Cursed Pendant", "A pendant with a dark aura, greatly increasing attack power but reducing defense.", ItemType::Accessory, 20);
    }
    else {
        if (randNum < 30) addItemToInventory("Holy Lance", "A lance blessed by the gods", ItemType::Weapon, 50);
        else if (randNum < 60) addItemToInventory("Divine Shield", "A shield that nullifies dark attacks.", ItemType::Armor, 45);
        else addItemToInventory("Divine Amulet", "A legendary amulet that balances attack and defense power.", ItemType::Accessory, 30);
    }
}

void showInventory() {
    int check = 0;
    while (1) {
        system("cls");
        cout << "====== Inventory ======\n";
        cout << "Player Level: " << player.level << "\n";
        cout << "Health: " << player.health << "/" << (100 + (player.level - 1) * 20) << "\n";
        cout << "Attack Power: " << player.attackPower << "\n";

        // 경험치 바 출력
        int expBarWidth = 20; // 경험치 바 길이 (20칸)
        float expRatio = (float)player.experience / player.experienceToNextLevel;
        int filledBars = expRatio * expBarWidth; // 채워진 부분의 길이 계산

        cout << "Experience: " << player.experience << " / " << player.experienceToNextLevel << " [";
        for (int i = 0; i < expBarWidth; i++) {
            if (i < filledBars) cout << "█"; // 채워진 부분
            else cout << "-"; // 빈 부분
        }
        cout << "] " << (int)(expRatio * 100) << "%\n";

        cout << "======================\n";

        cout << "Equipped Items:\n";
        if (playerEquipment.weapon != nullptr)
            cout << "Weapon: " << playerEquipment.weapon->name << " - " << playerEquipment.weapon->description << "\n";
        else cout << "Weapon: None\n";

        if (playerEquipment.armor != nullptr)
            cout << "Armor: " << playerEquipment.armor->name << " - " << playerEquipment.armor->description << "\n";
        else cout << "Armor: None\n";

        if (playerEquipment.accessory != nullptr)
            cout << "Accessory: " << playerEquipment.accessory->name << " - " << playerEquipment.accessory->description << "\n";
        else cout << "Accessory: None\n";

        cout << "======================\n";

        cout << "Items in your inventory:\n";
        if (playerInventory.items.empty()) {
            cout << "You have no items in your inventory.\n";
        }
        else {
            for (int i = 0; i < playerInventory.items.size(); ++i) {
                cout << (i + 1) << ". " << playerInventory.items[i].name << ": " << playerInventory.items[i].description << "\n";
            }
        }
        cout << "======================\n";
        cout << "1. Equip item\n";
        cout << "2. Return to the main menu.\n";

        if (check == 1) break;
        int input;
        while (true) {
            cout << "Enter choice: ";
            cin >> input;

            if (input == 1) {
                if (playerInventory.items.empty()) {
                    cout << "You have no items in your inventory.\n";
                    Sleep(2000);
                    break;
                }
                cout << "Enter the number of the item to equip: ";
                int num;
                cin >> num;

                if (num >= 1 && num <= playerInventory.items.size()) {
                    equipItem(&playerInventory.items[num - 1]);  // 아이템을 장착
                    break;
                }
                else {
                    cout << "Invalid item number. Please try again.\n";
                }
            }
            else if (input == 2) {
                check = 1;
                break;  // 인벤토리 화면 종료
            }
            else {
                cout << "Invalid choice. Please select again.\n";
            }
        }
    }

    cout << "======================\n";
    cout << "Press any key to return to the main menu.\n";
    _getch();
}

//키 입력 
bool KeyGame(int monsterLevel) {
    int numKeys = min(monsterLevel + 2, 10);  // 몬스터 레벨에 따라 입력할 키 개수 증가 (최대 10개)
    int baseTimeLimit = 2000;  // 기본 시간 제한 (2초)
    int timeLimit = max(baseTimeLimit - (monsterLevel * 100), 800);  // 몬스터 레벨이 높을수록 시간 감소 (최소 0.8초)
    bool success = true;

    srand(static_cast<unsigned>(time(0))); // 랜덤 시드 초기화

    for (int i = 0; i < numKeys; ++i) {
        char randomKey = 'A' + rand() % 26;  // 랜덤 키 생성
        char userInput;
        bool keySuccess = false;

        cout << "Press the key: " << randomKey << " within " << (timeLimit / 1000.0) << " seconds!\n";

        auto startTime = chrono::steady_clock::now();

        while (true) {
            auto currentTime = chrono::steady_clock::now();
            auto elapsedTime = chrono::duration_cast<chrono::milliseconds>(currentTime - startTime).count();

            if (elapsedTime > timeLimit) {
                cout << "Time's up! You missed the key!\n";
                success = false;
                break;  // 시간 초과 실패
            }

            if (_kbhit()) {
                userInput = _getch();
                if (toupper(userInput) == randomKey) {
                    keySuccess = true;
                    break;
                }
                else {
                    cout << "Wrong key! Try again quickly!\n";
                }
            }
            this_thread::sleep_for(chrono::milliseconds(50));
        }

        if (!keySuccess) {
            break; // 한 번이라도 실패하면 전체 실패 처리
        }
    }

    if (success) {
        cout << "Success! You pressed all keys correctly.\n";
    }
    else {
        cout << "Failed! One or more keys were missed or incorrect.\n";
    }
    return success;
}




void attackMonster(Monster& monster) {
    int lvl = monster.level;
    bool sucess = KeyGame(lvl);
    if (sucess) {
        int damage = player.attackPower;
        monster.health -= damage;
        cout << "\nYou attack the " << monster.name << " and deal " << damage << " damage!\n";
    }
    else cout << "\nYou failed to press the key in time. You cannot attack this turn!\n";
}

// 힐 사용
void heal() {
    int healAmount = rand() % 20 + 10;
    player.health += healAmount;
    if (player.health > 100 + (player.level - 1) * 20) {
        player.health = 100 + (player.level - 1) * 20;
    }
    cout << "You used a health potion and restored " << healAmount << " HP!\n";
}

void surren() {
    player.experience /= 2;
    cout << "\n항복하여 경험치가 반으로 줄었습니다...\n";
}

// 몬스터 공격 함수
void monsterAttack(Monster& monster) {
    int damage = monster.attackPower;
    player.health -= damage;
    cout << "The " << monster.name << " attacks you and deals " << damage << " damage!\n";
}

void levelUp(Character& character) {
    while (character.experience >= character.experienceToNextLevel) {
        character.experience -= character.experienceToNextLevel; // 남은 경험치 유지한 채 레벨업
        character.level++; // 레벨 증가

        // 체력 증가 공식 (기본 체력 + 20씩 증가)
        character.health += 20;

        // 경험치 증가 공식
        if (character.level <= 5) {
            character.experienceToNextLevel += 50;
            character.attackPower += 3;
        }
        else if (character.level <= 10) {
            character.experienceToNextLevel += 100;
            character.attackPower += 5;
        }
        else {
            character.experienceToNextLevel += 200;
            character.attackPower += 7;
        }

        std::cout << "\n*** Congratulations! You leveled up to Level " << character.level << "! ***\n";
        std::cout << "New Stats - Health: " << character.health
            << ", Attack Power: " << character.attackPower
            << ", Experience Needed for Next Level: " << character.experienceToNextLevel << "\n\n";
    }
}


const string titleLogin = R"(
                                                    
 _____ _____ _____ _____ _____    _____ _____ _____ 
| __  |   __|  _  |     |_   _|  | __  |  _  |   __|
|    -|   __|     |   --| | |    |    -|   __|  |  |
|__|__|_____|__|__|_____| |_|    |__|__|__|  |_____|

           1. PRESS 'L' TO LOGIN
           2. PRESS 'S' TO SIGN UP
           3. PRESS 'Q' TO QUIT
                                                                                       
)";
const string titleStart = R"( 
                                                    
 _____ _____ _____ _____ _____    _____ _____ _____ 
| __  |   __|  _  |     |_   _|  | __  |  _  |   __|
|    -|   __|     |   --| | |    |    -|   __|  |  |
|__|__|_____|__|__|_____| |_|    |__|__|__|  |_____|

           1. PRESS 'S' TO START
           2. PRESS 'Q' TO QUIT
                                                                                       
)";

// 게임 시작 대기시간
void startGame() {
    cout << "Game is starting...\n";
    Sleep(2000);
    system("cls");
}

// 게임 메뉴
void GameMenus()
{
    string menu = R"(
                                                
 _____ _____ _____ _____ _____    _____ _____ _____ 
| __  |   __|  _  |     |_   _|  | __  |  _  |   __|
|    -|   __|     |   --| | |    |    -|   __|  |  |
|__|__|_____|__|__|_____| |_|    |__|__|__|  |_____|
             
             PRESS TO NUMBER
                                  
             (1. Embark on an Adventure
              2. Check Inventory
              3. Challenge to a Duel
              4. Exit Game             )
   )";
    cout << menu << "\n";
}

void battle(Monster& monster) {
    int cnt = 0;
    while (player.health > 0 && monster.health > 0) {
        // 매 턴마다 화면 초기화
        system("cls");

        // 현재 상태 출력
        cout << "====== Battle ======\n";
        cout << "Your Health: " << max(player.health, 0) << "\n";
        cout << monster.name << "'s Health: " << max(monster.health, 0) << "\n";
        cout << "====================\n";
        cout << "1. Attack\n";
        cout << "2. Use Heal\n";
        cout << "3. Surren\n";
        cout << "====================\n";
        cout << "Choose your action: ";
        string input;
        int RsHp = monster.health;
        cin.sync();
        getline(cin, input);

        //cin >> input;

        if (input == "1") {
            // 플레이어의 공격
            attackMonster(monster);
            if (monster.health <= 0) {
                if (monster.name == "Necromancer") {
                    if (cnt != 0) {
                        cnt = 0;
                        break;
                    }
                    cnt++;
                    monster.health = RsHp;
                    cout << "네크로맨서는 한번 부활합니다!";
                }
                else {
                    cout << "You defeated the " << monster.name << "!\n";
                    int expGain = monster.level * 10;
                    player.experience += expGain;
                    cout << "You gained " << expGain << " experience points!\n";
                    levelUp(player);
                    dropItem();
                    break;
                }
            }
        }
        else if (input == "2") {
            // 힐 사용
            heal();
        }
        else if (input == "3") {
            surren();
            break;
        }
        else {
            cout << "Invalid choice. Try again.\n";
            Sleep(1000);
            continue;
        }

        // 몬스터의 공격
        if (monster.health > 0) {
            monsterAttack(monster);
        }

        // 플레이어가 사망했는지 확인
        if (player.health <= 0) {
            player.experience /= 2;
            cout << "\nGame Over! You have been defeated...\n";
            cout << "경험치가 반으로 줄었습니다....\n";
            exit(0);
        }
        Sleep(1000);
    }

    if (monster.health <= 0) {
        cout << "You emerge victorious!\n";
        player.health = (100 + (player.level - 1) * 20);
        Sleep(2000);
    }
}


void embarkOnAdventure() {
    system("cls");
    cout << "You embark on a thrilling adventure...\n";
    Sleep(2000);

    // 랜덤 몬스터 선택
    int level = player.level;
    Monster monster;

    int randNum = rand() % 100;

    // 레벨에 따라 등장할 몬스터를 결정
    if (level <= 2) {
        // 60% , 30%, 10%
        if (randNum < 60) {
            monster = monsters[0]; // Goblin
        }
        else if(randNum < 30) {
            monster = monsters[2]; // zombie
        }
        else {
            monster = monsters[1]; //troll
        }
    }
    else if (level <= 4) {
        //50% , 30% , 20%
        if (randNum < 50) {
            monster = monsters[2]; //zombie
        }
        else if (randNum < 80) {
            monster = monsters[1]; // troll
        }
        else {
            monster = monsters[3]; // orc
        }
    }
    else if(level <= 7) {
        // 30% , 40% , 20%, 10%
        if (randNum < 30) {
            monster = monsters[3]; // Orc
        }
        else if (randNum < 70) {
            monster = monsters[4]; // Dragon
        }
        else if (randNum < 90) monster = monsters[5]; // fire elem
        else monster = monsters[6]; // vampire 
    }
    else if(level <= 11){
        if (randNum < 30) monster = monsters[6]; // vampire
        else if (randNum < 60) monster = monsters[7]; //griffin
        else if (randNum < 80) monster = monsters[8]; // hydra
        else monster = monsters[9]; //necro
    }
    else {
        if (randNum < 30) monster = monsters[8]; // hydra
        else if (randNum < 60) monster = monsters[10]; //mino
        else if (randNum < 80) monster = monsters[11]; // phoenixite
        else monster = monsters[12]; //grim
    }

    cout << "You encounter a Level " << monster.level << " " << monster.name << "!\n";

    // 전투 시작
    battle(monster);

    cout << "======================\n";
    cout << "Press any key to return to the main menu.\n";
    _getch();
    Sleep(1000);
    system("cls");
}

void Inventory() {
    showInventory();
}

void Duel() {
    cout << "아직 준비중입니다.\n";
    Sleep(2000);
}



void SetConsoleSize(int width, int height)
{
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    SMALL_RECT windowSize = { 0, 0, width - 1, height - 1 };
    COORD bufferSize = { width, height };

    // 버퍼 크기 설정
    SetConsoleScreenBufferSize(console, bufferSize);
    // 창 크기 설정
    SetConsoleWindowInfo(console, TRUE, &windowSize);
}

void SetColor(int color)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void HideCursor()
{
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

void DisplayLoginScreen()
{
    SetColor(11); // 밝은 파란색으로 설정
    cout << titleLogin << "\n";
    SetColor(7); // 기본 색상으로 되돌림
}

void DisplayStartScreen()
{
    SetColor(11); // 밝은 파란색으로 설정
    cout << titleStart << "\n";
    SetColor(7); // 기본 색상으로 되돌림
}


int main() {
    sqlite3* db;
    initializeDatabase(db);
    string username, password;
    SetConsoleSize(SCREEN_WIDTH, SCREEN_HEIGHT); // 콘솔 창 크기를 크게 설정
    HideCursor(); // 커서를 숨깁니다
    bool TS = true;
    int check = 1;
    while (check) {
        DisplayLoginScreen(); // 로그인 확인 화면 출력
        TS = true;
        while (TS) {
            if (GetAsyncKeyState('L') & 0x8000) {
                
                cout << "Enter username : ";
                cin >> username;
                cout << "Enter password : ";
                cin >> password;

                if (loginUser(db, username, password)) {
                    cout << "Login successful!\n";
                    if (!loadPlayerInfo(db, username)) {
                        cout << "No player info found. Starting a new game...\n";
                        // 초기화 코드 (예: 기본 레벨 및 스탯)
                        player.name = username;
                        player.level = 1; // 기본 레벨
                        player.health = 100; // 기본 체력
                        player.attackPower = 20; // 기본 공격력
                        player.experience = 0; // 기본 경험치
                        player.experienceToNextLevel = 50; // 기본 레벨업에 필요한 경험치
                        savePlayerInfo(db, player);
                    }
                    else {
                        // 플레이어의 아이템 정보도 불러온다
                        loadPlayerItems(db, username);
                    }
                    check = 0;
                    Sleep(2000);
                }
                else {
                    cout << "Invalid username or password\n";
                    cout << "Return to menu...\n";
                    Sleep(2000);
                    system("cls");
                }
                TS = false;
            }
            else if (GetAsyncKeyState('S') & 0x8000) {
                cout << "Enter username : ";
                cin >> username;
                cout << "Enter password : ";
                cin >> password;
                registerUser(db, username, password);
                // 초기화 코드 (예: 기본 레벨 및 스탯)
                
                check = 0;
                TS = false;
            }
            else if (GetAsyncKeyState('Q') & 0x8000) return 0;


        }
    }
    
    system("cls");
    TS = true;
    DisplayStartScreen(); // 게임 시작 여부 화면 출력
    while (TS) {
        if (GetAsyncKeyState('S') & 0x8000) {
            TS = false;
            system("cls");
        }
        else if (GetAsyncKeyState('Q') & 0x8000) {
            savePlayerInfo(db, player); // 플레이어 정보 저장
            savePlayerItems(db, playerInventory, player.name); // 아이템 정보 저장
            sqlite3_close(db); // 데이터베이스 닫기
            return 0;
        }
    }
    startGame();
   
    while (1) {
        TS = true;
        GameMenus();
        while (TS)
        {
            if (GetAsyncKeyState('1') & 0x8000) {
                embarkOnAdventure();
                TS = false;
                system("cls");
            }
            else if (GetAsyncKeyState('2') & 0x8000) {
                Inventory();
                TS = false;
                system("cls");
            }
            else if (GetAsyncKeyState('3') & 0x8000) {
                Duel();
                TS = false;
                system("cls");
            }
            else if (GetAsyncKeyState('4') & 0x8000) {
                if (playerEquipment.weapon != nullptr) {
                    player.attackPower -= playerEquipment.weapon->Power;
                }
                if (playerEquipment.armor != nullptr) {
                    player.health -= playerEquipment.armor->Power;
                }
                if (playerEquipment.accessory != nullptr) {
                    player.attackPower -= playerEquipment.accessory->Power;
                }
                savePlayerInfo(db, player); // 플레이어 정보 저장
                savePlayerItems(db, playerInventory, player.name); // 아이템 정보 저장
                sqlite3_close(db); // 데이터베이스 닫기
                return 0;
            }
        }
    }
    savePlayerInfo(db, player); // 플레이어 정보 저장
    savePlayerItems(db, playerInventory, player.name); // 아이템 정보 저장
    sqlite3_close(db); // 데이터베이스 닫기
    return 0;
}
