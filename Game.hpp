#ifndef GAME_H
#define GAME_H

#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <ctime>

enum class CornErrorType {
    LOG,
    WARNING,
    ERROR
};

struct CornShopItem {
    unsigned int amount;
    unsigned int cost;
    unsigned int sellPrice;

    CornShopItem(int a, int c, int p) : amount(a), cost(c), sellPrice(p) {};
};

class CornGame {
public:
    static bool runCommand(std::vector<std::string>& args, const std::function<bool(const std::string&, CornErrorType)>& print, const std::function<std::string()>& requestMethod);

protected:
    static unsigned int money;
    static unsigned int energy;
    static unsigned int field;
    static unsigned int ready;
    static unsigned int price;

    static CornShopItem corn;
    static CornShopItem seeds;
    static CornShopItem energyDrink;

    static const std::string homeDirectory;
    static const std::string savePath;
    static const std::string saveFile;

private:
    // Data storage functionality
    static bool loadGame();
    static bool saveGame();
    static bool createSaveData();
    static int getData(std::ifstream& gameData);

    // Calculate time between command runs
    static int timeDistance();

    // Commands
    static bool reset(std::vector<std::string>& args);
    static bool show(std::vector<std::string>& args);
    static bool use(std::vector<std::string>& args);
    static bool shop(std::vector<std::string>& args);
    static bool sell(std::vector<std::string>& args);
    static bool plant(std::vector<std::string>& args);
    static bool explore(std::vector<std::string>& args);
    static bool harvest(std::vector<std::string>& args);
    static bool purchase(std::vector<std::string>& args);

    // Time since command run
    static std::chrono::time_point<std::chrono::system_clock> lastRun;

    // Hash map for commands
    static const std::unordered_map<std::string, std::function<bool(std::vector<std::string>&)>> commandMap;

    // Hash map for shop
    static std::unordered_map<std::string, CornShopItem&> shopMap;
    
    // Custom implementation for log output
    static std::function<bool(const std::string&, CornErrorType)> print;
    static std::function<std::string()> request;

public:
    // Not used in program, but allow for custom output apart from show command
    static int getMoney() { loadGame(); return money; }
    static CornShopItem getCorn() { loadGame(); return corn; }
    static int getEnergy() { loadGame(); return energy; }
    static CornShopItem getSeeds() { loadGame(); return seeds; }
    static int getField() { loadGame(); return field; }
    static int getReady() { loadGame(); return ready; }
    
};

#endif // GAME_H