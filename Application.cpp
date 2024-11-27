#include <iostream>
#include <fstream>
#include <vector>
#include <functional>

#include "Game.hpp"

bool log(const std::string& content) {
    std::cout << content << std::flush;
    return true;
}

bool warn(const std::string& content) {
    std::cerr << content << std::flush;
    return true;
}

bool error(const std::string& content) {
    std::cerr << content << std::flush;
    return true;
}

const std::unordered_map<CornErrorType, std::function<bool(const std::string&)>> printType = {
    { CornErrorType::LOG, log },
    { CornErrorType::WARNING, warn },
    { CornErrorType::ERROR, error }
};

// Implementation of the print method from Game class
bool print(const std::string& content, CornErrorType type) {
    const auto it = printType.find(type);
    if (it != printType.end()) {
        return it->second(content);
    }
    return false;
}

// Implementation of the request method from Game class
std::string request() {
    std::string input;
    std::cin >> input;
    return input;
}

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);
    return !CornGame::runCommand(args, print, request);
}