#include "savemanager.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "gameboard.h"
#include "resources.h"
#include "player.h"
#include "assessment.h"

namespace {

// 0=Blue,1=Red,2=Orange,3=Yellow
int colourToIndex(Colour c) {
    switch (c) {
        case Colour::Blue:   return 0;
        case Colour::Red:    return 1;
        case Colour::Orange: return 2;
        case Colour::Yellow: return 3;
        default:             return 0;
    }
}

Colour indexToColour(int idx) {
    switch (idx) {
        case 0:  return Colour::Blue;
        case 1:  return Colour::Red;
        case 2:  return Colour::Orange;
        case 3:  return Colour::Yellow;
        default: throw std::runtime_error("Invalid turn index in save file.");
    }
}

// Map save-file resource codes to Resources:
// 0 = CAFFEINE, 1 = LAB, 2 = LECTURE, 3 = STUDY, 4 = TUTORIAL, 5 = NETFLIX
Resources codeToResource(int code) {
    switch (code) {
        case 0: return Resources::Caffeine;
        case 1: return Resources::Lab;
        case 2: return Resources::Lecture;
        case 3: return Resources::Study;
        case 4: return Resources::Tutorial;
        case 5: return Resources::Netflix;
        default:
            throw std::runtime_error("Invalid resource code in board layout.");
    }
}

Assessment levelToAssessment(int level) {
    switch (level) {
        case 1: return Assessment::Assignment;
        case 2: return Assessment::Midterm;
        case 3: return Assessment::Exam;
        default:
            throw std::runtime_error("Invalid criterion level in save file.");
    }
}

// Parsed data for one player's line
struct PlayerSaveData {
    int caff   = 0;
    int lab    = 0;
    int lect   = 0;
    int study  = 0;
    int tut    = 0;
    std::vector<int> edgeIds;
    std::vector<std::pair<int,int>> vertexLevels; // (vertexId, level)
};

PlayerSaveData parsePlayerLine(const std::string &line) {
    PlayerSaveData data;
    std::istringstream iss{line};

    if (!(iss >> data.caff >> data.lab >> data.lect >> data.study >> data.tut)) {
        throw std::runtime_error("Malformed player line in save file (resources).");
    }

    std::string tok;
    enum class Section { None, Goals, Criteria };
    Section section = Section::None;
    int pendingVertexId = -1;

    while (iss >> tok) {
        if (tok == "g") {
            section = Section::Goals;
        } else if (tok == "c") {
            section = Section::Criteria;
            pendingVertexId = -1;
        } else if (section == Section::Goals) {
            // edge id
            data.edgeIds.push_back(std::stoi(tok));
        } else if (section == Section::Criteria) {
            // alternating: vertexId, level
            if (pendingVertexId == -1) {
                pendingVertexId = std::stoi(tok);
            } else {
                int level = std::stoi(tok);
                data.vertexLevels.emplace_back(pendingVertexId, level);
                pendingVertexId = -1;
            }
        } else {
            // token before 'g' – shouldn't happen in valid saves, but ignore
        }
    }

    if (section == Section::Criteria && pendingVertexId != -1) {
        throw std::runtime_error("Unpaired vertex id in criteria section.");
    }

    return data;
}

} // namespace

void SaveManager::saveGame(const GameBoard &board,
                           Colour currentPlayer,
                           const std::string &filename) {
    std::ofstream out{filename};
    if (!out) {
        throw std::runtime_error("Could not open save file '" + filename + "' for writing.");
    }

    // Line 1: current turn index
    out << colourToIndex(currentPlayer) << '\n';

    // Lines 2–5: players, in fixed order: Blue, Red, Orange, Yellow.
    const auto &players = board.getPlayers();
    if (players.size() != 4) {
        throw std::runtime_error("SaveManager::saveGame: expected 4 players.");
    }

    for (const auto &p : players) {
        // Player::encodeForSave() must return:
        // "<numCaffeines> <numLabs> <numLectures> <numStudies> <numTutorials> "
        // "g <goalIds...> c <criterionId state>..."
        out << p->encodeForSave() << '\n';
    }

    // Line 6: board layout
    out << board.encodeBoardLayoutForSave() << '\n';

    // Line 7: geese tile
    out << board.getGeeseTile() << '\n';
}

GameBoard SaveManager::loadGame(bool enhance,
                                const std::string &filename,
                                Colour &currentPlayerOut) {
    std::ifstream in{filename};
    if (!in) {
        throw std::runtime_error("Could not open save file '" + filename + "' for reading.");
    }

    std::string line;

    // Line 1: current turn index
    if (!std::getline(in, line)) {
        throw std::runtime_error("Save file is missing current turn line.");
    }
    {
        std::istringstream iss{line};
        int idx;
        if (!(iss >> idx)) {
            throw std::runtime_error("Invalid current turn line in save file.");
        }
        currentPlayerOut = indexToColour(idx);
    }

    // Lines 2–5: per-player lines
    std::vector<std::string> playerLines;
    playerLines.reserve(4);
    for (int i = 0; i < 4; ++i) {
        if (!std::getline(in, line)) {
            throw std::runtime_error("Save file is missing player lines.");
        }
        playerLines.push_back(line);
    }

    // Line 6: board layout
    if (!std::getline(in, line)) {
        throw std::runtime_error("Save file is missing board layout line.");
    }

    std::istringstream boardIss{line};
    std::vector<int> values;
    std::vector<Resources> resources;

    int resCode, val;
    while (boardIss >> resCode >> val) {
        resources.push_back(codeToResource(resCode));
        values.push_back(val);
    }

    if (values.size() != 19 || resources.size() != 19) {
        throw std::runtime_error("Expected 19 tiles in board layout line.");
    }

    // Line 7: geese tile index
    if (!std::getline(in, line)) {
        throw std::runtime_error("Save file is missing geese line.");
    }
    int geeseIndex = -1;
    {
        std::istringstream iss{line};
        if (!(iss >> geeseIndex)) {
            throw std::runtime_error("Invalid geese line in save file.");
        }
    }

    // --- Construct the base GameBoard from tile layout ---
    GameBoard board{enhance, values, resources};

    // --- Rebuild players' resources, roads, and residences ---
    auto &players = const_cast<std::vector<std::unique_ptr<Player>>&>(board.getPlayers());
    if (players.size() != 4) {
        throw std::runtime_error("SaveManager::loadGame: expected 4 players in board.");
    }

    for (int i = 0; i < 4; ++i) {
        PlayerSaveData data = parsePlayerLine(playerLines[i]);

        Player *p = players[i].get();
        Colour colour = indexToColour(i);

        // 1. Resources
        p->setResourcesFromSave(
            data.caff,
            data.lab,
            data.lect,
            data.study,
            data.tut
        );

        // 2. Roads (edges)
        for (int edgeId : data.edgeIds) {
            board.setEdgeForLoad(colour, edgeId);
        }

        // 3. Residences (vertices)
        for (auto &pair : data.vertexLevels) {
            int vertexId = pair.first;
            int level    = pair.second;
            Assessment a = levelToAssessment(level);
            board.setVertexForLoad(colour, vertexId, a);
        }
    }

    // Apply geese position if any
    if (geeseIndex >= 0) {
        // moveGeese ignores activePlayer for legality, so any colour is fine here.
        board.moveGeese(Colour::Blue, geeseIndex);
    }

    return board;
}
