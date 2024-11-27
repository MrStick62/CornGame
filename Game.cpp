#include <filesystem>
#include <iostream>
#include <algorithm>
#include <cmath>

#include "Game.hpp"

const std::string CornGame::homeDirectory(getenv("HOME"));
const std::string CornGame::savePath = homeDirectory + "/corn";
const std::string CornGame::saveFile = "/save.txt";

std::function<bool(const std::string&, CornErrorType type)> CornGame::print = nullptr;
std::function<std::string()> CornGame::request = nullptr;

unsigned int CornGame::money = 0;
unsigned int CornGame::energy = 100;
unsigned int CornGame::field = 0;
unsigned int CornGame::ready = 0;

CornShopItem CornGame::corn(0, 2, 1);
CornShopItem CornGame::seeds(1, 10, 7);
CornShopItem CornGame::energyDrink(0, 20, 14);

std::chrono::time_point<std::chrono::system_clock> CornGame::lastRun;

const std::unordered_map<std::string, std::function<bool(std::vector<std::string>&)>> CornGame::commandMap = {
    { "show", CornGame::show },
    { "shop", CornGame::shop },
    { "plant", CornGame::plant },
    { "harvest", CornGame::harvest },
    { "purchase", CornGame::purchase },
    { "sell", CornGame::sell },
};

std::unordered_map<std::string, CornShopItem&> CornGame::shopMap = {
    { "corn", CornGame::corn },
    { "seeds", CornGame::seeds },
    { "RedBull", CornGame::energyDrink },
};

bool CornGame::loadGame() {
    std::ifstream gameData(savePath + saveFile);

    if (!gameData) {
        return createSaveData();
    }

    try {
        std::time_t time;
        gameData >> time;
        lastRun = std::chrono::system_clock::from_time_t(time);

        std::string emptyLine;
        std::getline(gameData, emptyLine);
        money = getData(gameData);
        corn.amount = getData(gameData);
        energy = getData(gameData);
        seeds.amount = getData(gameData);
        field = getData(gameData);
        ready = getData(gameData);
    } catch (std::exception& e) {
        print(e.what(), CornErrorType::ERROR);
        return false;
    }

    // Update the values dependant on how much time has passed
    const auto now = std::chrono::system_clock::now();
    const unsigned int duration = std::chrono::duration_cast<std::chrono::seconds>(now - lastRun).count();
    energy += duration / 5;
    if (energy > 100) {
        energy = 100;
    }

    ready += std::sqrt(field) * duration / 10;
    if (ready > field) {
        ready = field;
    }

    gameData.close();
    return true;
}

int CornGame::getData(std::ifstream& gameData) {
    try {
        std::string line;
        std::getline(gameData, line);
        return std::stoi(line);
    } catch (std::exception& e) {
        throw std::runtime_error(
            "Failed to load game data, the following file is corrupted: " 
            + savePath + saveFile + '\n'
        );
    }
}

int CornGame::timeDistance() {
    return 0;
}

bool CornGame::createSaveData() {
    std::error_code ec;
    if (!std::filesystem::create_directory(savePath, ec) && ec) {
        print("Failed to create directory: \"" + savePath + "\"\n", CornErrorType::ERROR);
        return false;
    }

    return saveGame();
}

bool CornGame::saveGame() {
    std::ofstream gameData(savePath + saveFile);
    if (!gameData) {
        print("Failed to write file: \"" + savePath + saveFile + "\"\n", CornErrorType::ERROR);
        return false;
    }

    lastRun = std::chrono::system_clock::now();
    std::time_t now = std::chrono::system_clock::to_time_t(lastRun);

    gameData << now << '\n' << money << '\n' << corn.amount << '\n' << energy 
        << '\n' << seeds.amount << '\n' << field << '\n' << ready << std::endl;

    gameData.close();

    return true;
}

bool CornGame::runCommand(std::vector<std::string>& args, const std::function<bool(const std::string&, CornErrorType)>& printMethod, const std::function<std::string()>& requestMethod) {
    print = printMethod;
    request = requestMethod;

    if (args.empty()) {
        return loadGame() && show(args) && saveGame();
    }

    std::string arg1 = args[0];
    if (arg1 == "reset") {
        return CornGame::reset(args);
    }

    auto it = commandMap.find(arg1);
    if (it != commandMap.end()) {
        return loadGame() && it->second(args) && saveGame();
    }

    print("\"" + arg1 + "\"" + " not recognized as a valid argument\n", CornErrorType::WARNING);
    return false;
}

/*
 * User commands
 */

bool CornGame::reset(std::vector<std::string>& args) {
    return print("Are you sure you would like to reset all game data? [y/n]: ", CornErrorType::LOG) && (request() == "y") && saveGame();
}

bool CornGame::show(std::vector<std::string>& args) {
    std::ostringstream oss;
    oss << "         - INFO - \n";
    oss << "        Money: $" << money << '\n';
    oss << "         Corn: " << corn.amount << '\n';
    oss << "        Seeds: " << seeds.amount << '\n';
    oss << "         --------\n";
    oss << "        Field: " << ready << '/' << field << '\n';
    oss << "       Energy: " << energy << "/100\n";
    return print(oss.str(), CornErrorType::LOG);
}

bool CornGame::shop(std::vector<std::string>& args) {
    std::ostringstream oss;
    oss << "         - SHOP -\n";
    oss << "       Seeds: $" << seeds.cost << '\n';
    oss << "     RedBull: $" << energyDrink.cost << '\n';
    return print(oss.str(), CornErrorType::LOG);
}

bool CornGame::sell(std::vector<std::string>& args) {
    unsigned int toSell = 1;
    if (args.size() > 1) {
        try {
            toSell = std::stoi(args[1]);
        } catch (std::exception e) {
            std::ostringstream oss;
            oss << "Invalid argument \"" << args[1] << "\" for number of items to sell.\n";
            print(oss.str(), CornErrorType::ERROR);
            return false;
        }
    }

    if (toSell > corn.amount) {
        print("You do not have enough corn to sell!\n", CornErrorType::ERROR);
        return false;
    }

    std::ostringstream oss;
    oss << "Successfully sold " << toSell << " corn for $" << toSell * corn.sellPrice << '\n';
    print(oss.str(), CornErrorType::LOG);
    corn.amount -= toSell;
    money += toSell * corn.sellPrice;
    return true;
}

bool CornGame::plant(std::vector<std::string>& args) {
    unsigned int toPlant = 1;
    if (args.size() > 1) {
        try {
            toPlant = std::stoi(args[1]);
        } catch (std::exception e) {
            std::ostringstream oss;
            oss << "Invalid argument \"" << args[1] << "\" for number of items to plant.\n";
            print(oss.str(), CornErrorType::ERROR);
            return false;
        }
    }

    unsigned int energyCost = toPlant * 5;
    if (seeds.amount < toPlant) {
        print("Not enough seeds!\n", CornErrorType::WARNING);
        return false;
    } else if (energy - energyCost < 0) {
        print("Not enough energy!\n", CornErrorType::WARNING);
        return false;
    }

    std::ostringstream oss;
    oss << "Successfully planted " << toPlant << " corn.\n";
    oss << "This cost " << energyCost << " energy.\n";
    print(oss.str(), CornErrorType::LOG);

    energy -= energyCost;
    seeds.amount -= toPlant;
    field += toPlant;
    return true;
}

bool CornGame::harvest(std::vector<std::string>& args) {
    unsigned int toHarvest = 1;
    if (args.size() > 1) {
        try {
            toHarvest = std::stoi(args[1]);
        } catch (std::exception e) {
            std::ostringstream oss;
            oss << "Invalid argument \"" << args[1] << "\" for number of items to harvest.\n";
            print(oss.str(), CornErrorType::ERROR);
            return false;
        }
    }

    int energyCost = toHarvest * 2;
    if (ready < toHarvest) {
        print("Not enough corn is ready for harvest!\n", CornErrorType::WARNING);
        return false;
    } else if (energy - energyCost < 0) {
        print("Not enough energy!\n", CornErrorType::WARNING);
        return false;
    }

    std::ostringstream oss;
    oss << "Successfully harvested " << toHarvest << " corn.\n";
    oss << "This cost " << energyCost << " energy.\n";
    print(oss.str(), CornErrorType::LOG);

    energy -= energyCost;
    ready -= toHarvest;
    corn.amount += toHarvest;
    return true;
}

bool CornGame::purchase(std::vector<std::string>& args) {
    if (args.size() < 2) {
        print("Insufficient arguments to perform command \"purchase\"\n", CornErrorType::WARNING);
        return false;
    }

    std::string arg1 = args[1];

    auto it = shopMap.find(arg1);
    if (it != shopMap.end()) {
        unsigned int toPurchase = 1;
        if (args.size() > 2) {
            try {
                toPurchase = std::stoi(args[2]);
            } catch (std::exception e) {
                std::ostringstream oss;
                oss << "Invalid argument \"" << args[1] << "\" for number of items to purchase.\n";
                print(oss.str(), CornErrorType::ERROR);
                return false;
            }
        }

        CornShopItem &item = it->second;
        unsigned int cost = item.cost * toPurchase;
        if (cost > money) {
            print("Insufficient funds!\n", CornErrorType::WARNING);
            return false;
        }

        std::ostringstream oss;
        oss << "Successfully purchased " << toPurchase << " " << arg1 << " for $" << cost << '\n';
        print(oss.str(), CornErrorType::LOG);
        money -= item.cost * toPurchase;
        item.amount = item.amount + toPurchase;
        return true;
    }

    std::ostringstream oss;
    oss << "No item available for purchase called \"" << arg1 << "\"\n";
    print(oss.str(), CornErrorType::WARNING);
    return false;
}