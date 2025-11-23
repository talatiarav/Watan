#include "savemanager.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "gameboard.h"
#include "player.h"   // for Player::encodeForSave once you add it
#include "resources.h"

namespace {

// fixed order mapping for first line
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
    // We rely on GameBoard keeping players in that order.
    const auto &players = board.getPlayers();
    if (players.size() != 4) {
        throw std::runtime_error("SaveManager::saveGame: expected 4 players.");
    }

    for (const auto &p : players) {
        // TODO: ensure your new Player class has:
        //   std::string encodeForSave() const;
        //
        // that returns:
        //   "<numCaffeines> <numLabs> <numLectures> <numStudies> <numTutorials> "
        //   "g <goalIds...> c <criterionId state>..."
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

    // Lines 2–5: player lines (we’ll just read them for now)
    std::vector<std::string> playerLines;
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
        Resources r;
        switch (resCode) {
            case 0: r = Resources::Caffeine; break;
            case 1: r = Resources::Lab;      break;
            case 2: r = Resources::Lecture;  break;
            case 3: r = Resources::Study;    break;
            case 4: r = Resources::Tutorial; break;
            case 5: r = Resources::Netflix;  break;
            default:
                throw std::runtime_error("Invalid resource code in board layout.");
        }
        resources.push_back(r);
        values.push_back(val);
    }

    if (values.size() != 19 || resources.size() != 19) {
        throw std::runtime_error("Expected 19 tiles in board layout line.");
    }

    // Line 7: geese tile (we'll apply it after constructing the board)
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

    // --- Construct base GameBoard from layout ---
    GameBoard board{enhance, values, resources};

    // --- TODO: reconstruct players & buildings from playerLines ---
    //
    // For each i in [0..3]:
    //   - parse playerLines[i] into:
    //       numCaff, numLab, numLect, numStudy, numTut,
    //       list of edge IDs (after "g"),
    //       list of (vertexId, level) pairs (after "c")
    //   - for resources, call something like:
    //       player->resetState();
    //       player->grantResources(Resources::Caffeine, numCaff);
    //       ...
    //   - for each edgeId, call:
    //       board.achieveEdge(colourForIndex(i), edgeId);   // or a "silent" variant that doesn't deduct resources
    //   - for each (vertexId, level), call:
    //       board.completeVertex(...) / improveVertex(...) enough times,
    //       or provide a direct "setAssessmentLevel" API on Vertex/Player.
    //
    // Because we haven't finalized those direct reconstruction APIs yet, this
    // part is left as an exercise to hook up once Player/Vertex/Edge are done.

    // Apply geese position if any
    if (geeseIndex >= 0) {
        // We ignore the activePlayer in moveGeese here, so we can pass anything.
        board.moveGeese(Colour::Blue, geeseIndex);
    }

    return board;
}
