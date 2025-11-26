#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "gameboard.h"
#include "gamecontroller.h"
#include "savemanager.h"
#include "resources.h"
#include "colour.h"

// Helper to map layout file resource codes to Resources.
// 0 = CAFFEINE, 1 = LAB, 2 = LECTURE, 3 = STUDY, 4 = TUTORIAL, 5 = NETFLIX
static Resources codeToResource(int code) {
    switch (code) {
        case 0: return Resources::Caffeine;
        case 1: return Resources::Lab;
        case 2: return Resources::Lecture;
        case 3: return Resources::Study;
        case 4: return Resources::Tutorial;
        case 5: return Resources::Netflix;
        default:
            throw std::runtime_error("Invalid resource code in board layout file.");
    }
}

// Load a board layout from a "-board" file (same format as save line 6).
static GameBoard createBoardFromLayoutFile(const std::string &filename,
                                           bool enhance) {
    std::ifstream in{filename};
    if (!in) {
        throw std::runtime_error("Could not open board file '" + filename + "'.");
    }

    std::string line;
    if (!std::getline(in, line)) {
        throw std::runtime_error("Board file is empty.");
    }

    std::istringstream iss{line};
    std::vector<int> values;
    std::vector<Resources> resources;

    int resCode, val;
    while (iss >> resCode >> val) {
        resources.push_back(codeToResource(resCode));
        values.push_back(val);
    }

    if (values.size() != 19 || resources.size() != 19) {
        throw std::runtime_error("Board file must specify exactly 19 tiles.");
    }

    return GameBoard{enhance, values, resources};
}

int main(int argc, char *argv[]) {
    bool enhance = false;        // if you later want fancy text display
    std::string boardFile;
    std::string loadFile;

    // Very simple arg parsing: we care about -board <file> and -load <file>.
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-board" && i + 1 < argc) {
            boardFile = argv[++i];
        } else if (arg == "-load" && i + 1 < argc) {
            loadFile = argv[++i];
        }
        // You can add -seed, -enhance, etc. here later.
    }

    try {
        GameBoard board = GameBoard::createRandom(enhance);
        Colour startingPlayer = Colour::Blue;

        if (!loadFile.empty()) {
            // Full game load
            Colour loadedPlayer;
            board = SaveManager::loadGame(enhance, loadFile, loadedPlayer);
            startingPlayer = loadedPlayer;
        } else if (!boardFile.empty()) {
            // Board layout from file, but fresh game
            board = createBoardFromLayoutFile(boardFile, enhance);
            startingPlayer = Colour::Blue;
        } else {
            // Already set: random board, Blue starts
        }

        GameController controller(std::move(board), startingPlayer);
        controller.run(std::cin, std::cout);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
