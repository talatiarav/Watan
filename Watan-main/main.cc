#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

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
    bool enhance = false;        
    std::string boardFile;
    std::string loadFile;
    int seed = -1;               // -1 means no seed specified, use time

    // Parse command line arguments: -board <file>, -load <file>, -seed <value>, -enhance
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-board" && i + 1 < argc) {
            boardFile = argv[++i];
        } else if (arg == "-load" && i + 1 < argc) {
            loadFile = argv[++i];
        } else if (arg == "-seed" && i + 1 < argc) {
            seed = std::atoi(argv[++i]);
        } else if (arg == "-enhance") {
            enhance = true;  // Enable colored output
        }
    }

    // Set the random seed for the entire program
    if (seed != -1) {
        // User specified a seed, use it
        std::srand(static_cast<unsigned int>(seed));
    } else {
        // No seed specified: use a fixed default seed for deterministic behavior
        std::srand(0);
    }

    // Enable colored output for player names if enhance mode is on
    if (enhance) {
        Colour_enableColors(true);
    }

    bool playAgain = true;
    
    while (playAgain) {
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
            playAgain = controller.run(std::cin, std::cout);
            
            // After first game, clear the load file so subsequent games are fresh
            if (playAgain) {
                loadFile.clear();
            }
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
            return 1;
        }
    }

    return 0;
}
