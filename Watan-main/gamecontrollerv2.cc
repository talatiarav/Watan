#include "gamecontroller.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <limits>
#include <random>

#include "gameboard.h"
#include "savemanager.h"
#include "resources.h"


using std::endl;

namespace {

bool parseColour(const std::string &s, Colour &out) {
    if (s == "Blue")   { out = Colour::Blue;   return true; }
    if (s == "Red")    { out = Colour::Red;    return true; }
    if (s == "Orange") { out = Colour::Orange; return true; }
    if (s == "Yellow") { out = Colour::Yellow; return true; }
    return false;
}

bool parseResource(const std::string &s, Resources &out) {
    if (s == "Caffeine") { out = Resources::Caffeine; return true; }
    if (s == "Lab")      { out = Resources::Lab;      return true; }
    if (s == "Lecture")  { out = Resources::Lecture;  return true; }
    if (s == "Study")    { out = Resources::Study;    return true; }
    if (s == "Tutorial") { out = Resources::Tutorial; return true; }
    return false;
}

std::string resourceToSpecName(Resources r) {
    switch (r) {
        case Resources::Caffeine: return "CAFFEINE";
        case Resources::Lab:      return "LAB";
        case Resources::Lecture:  return "LECTURE";
        case Resources::Study:    return "STUDY";
        case Resources::Tutorial: return "TUTORIAL";
        default:                  return "";
    }
}

} // namespace


GameController::GameController(GameBoard &&b, Colour startingPlayer)
    : board(std::move(b)),
      currentPlayer(startingPlayer),
      rolledThisTurn(false),
      awaitingGeesePlacement(false),
      quitRequested(false) {}

GameController::GameController(GameBoard &&b)
    : GameController(std::move(b), Colour::Blue) {}  // your old behavior

// --- static helpers ---

std::string GameController::colourToString(Colour c) {
    switch (c) {
        case Colour::Blue:   return "Blue";
        case Colour::Red:    return "Red";
        case Colour::Orange: return "Orange";
        case Colour::Yellow: return "Yellow";
        default:             return "Unknown";
    }
}

// --- turn / flow ---

void GameController::setupInitialAssignments(std::istream &in, std::ostream &out) {
    // Turn order for initial placements:
    // Blue, Red, Orange, Yellow, Yellow, Orange, Red, Blue
    const Colour order[8] = {
        Colour::Blue,
        Colour::Red,
        Colour::Orange,
        Colour::Yellow,
        Colour::Yellow,
        Colour::Orange,
        Colour::Red,
        Colour::Blue
    };

    for (Colour c : order) {
        while (true) {
            out << "Student " << colourToString(c)
                << ", where do you want to complete the assignment?" << std::endl;
            out << "> ";

            int vertexId;
            if (!(in >> vertexId)) {
                // bad input (EOF or non-integer)
                in.clear();
                in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                out << "Invalid input. Please enter an intersection id." << std::endl;
                continue;
            }

            try {
                board.placeInitialAssignment(c, vertexId);
            } catch (const std::exception &e) {
                out << e.what() << std::endl;
                // re-prompt same student
                continue;
            }

            // consume rest of line, then show board with new assignment
            in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            board.printBoard(out);
            break;  // move to next student in the order
        }
    }
}

void GameController::startNewTurn(std::ostream &out) {
    rolledThisTurn = false;
    awaitingGeesePlacement = false;

    out << "Student " << colourToString(currentPlayer) << "'s turn." << endl;

    // Spec says: "followed by the status of the student".
    // Our GameBoard::printStatus prints all students, which is fine / even nicer.
    board.printStatus(out);
}

void GameController::advancePlayer() {
    switch (currentPlayer) {
        case Colour::Blue:   currentPlayer = Colour::Red;    break;
        case Colour::Red:    currentPlayer = Colour::Orange; break;
        case Colour::Orange: currentPlayer = Colour::Yellow; break;
        case Colour::Yellow: currentPlayer = Colour::Blue;   break;
        default:             currentPlayer = Colour::Blue;   break;
    }
}

// --- main loop ---

void GameController::run(std::istream &in, std::ostream &out) {
    setupInitialAssignments(in, out);
    startNewTurn(out);

    std::string line;
    while (!quitRequested && !board.hasWinner() && std::getline(in, line)) {
        if (line.empty()) continue;

        try {
            handleCommand(line, in, out);
        } catch (const std::exception &e) {
            // GameBoard throws with spec strings for build/resource errors,
            // so just print the message.
            out << e.what() << endl;
        }

        // If winner was detected inside a command, we drop out of loop next iteration.
    }

    if (board.hasWinner()) {
        Colour winner = board.getWinner();
        out << "Student " << colourToString(winner) << " wins the game!" << endl;
    } else if (quitRequested) {
        out << "Game ended by user." << endl;
    }
}

// --- command parsing ---

void GameController::handleCommand(const std::string &line,
                                   std::istream &in,
                                   std::ostream &out) {
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    if (cmd.empty()) return;

    if (awaitingGeesePlacement) {
        int tileId;
        try {
            tileId = std::stoi(cmd);
        } catch (...) {
            out << "Invalid tile. Please enter a tile number." << std::endl;
            return; // stay in geese mode, ask again next loop
        }

        try {
            cmdGeese(tileId, in, out);   // this should call board.moveGeese(...)
            awaitingGeesePlacement = false;
        } catch (const std::exception &e) {
            out << e.what() << std::endl;
            // still waiting for a valid tile, so don't clear the flag
        }

        return; // don't treat this as a normal command
    }

    // Normalize to lowercase if you want; for now assume lower-case input.
    if (cmd == "help") {
        cmdHelp(out);
    } else if (cmd == "board") {
        cmdBoard(out);
    } else if (cmd == "status") {
        cmdStatus(out);
    } else if (cmd == "criteria") {
        cmdCriteria(out);
    } else if (cmd == "roll") {
        cmdRoll(in, out);
    } else if (cmd == "load") {
        cmdSetLoadedDice(out);
    } else if (cmd == "fair") {
        cmdSetFairDice(out);
    } else if (cmd == "trade") {
        std::string targetStr, giveStr, takeStr;
        if (!(iss >> targetStr >> giveStr >> takeStr)) {
            out << "Usage: trade <player>|bank <give> <take>\n";
            return;
        }
        cmdTrade(targetStr, giveStr, takeStr, in, out);
    } else if (cmd == "geese") {
        int tileId;
        if (!(iss >> tileId)) {
            out << "Invalid command." << endl;
        } else {
            cmdGeese(tileId, in, out);
        }
    } else if (cmd == "complete") {
        int vertexId;
        if (!(iss >> vertexId)) {
            out << "Invalid command." << endl;
        } else {
            cmdComplete(vertexId, out);
        }
    } else if (cmd == "improve") {
        int vertexId;
        if (!(iss >> vertexId)) {
            out << "Invalid command." << endl;
        } else {
            cmdImprove(vertexId, out);
        }
    } else if (cmd == "achieve") {
        int edgeId;
        if (!(iss >> edgeId)) {
            out << "Invalid command." << endl;
        } else {
            cmdAchieve(edgeId, out);
        }
    } else if (cmd == "save") {
        std::string filename;
        if (!(iss >> filename)) {
            out << "Invalid command." << endl;
        } else {
            cmdSave(filename, out);
        }
    } else if (cmd == "next") {
        if (!rolledThisTurn) {
            out << "You must roll before ending your turn." << endl;
        } else if (awaitingGeesePlacement) {
            out << "You must move the GEESE before ending your turn." << endl;
        } else {
            advancePlayer();
            startNewTurn(out);
        }
    } else if (cmd == "quit") {
        quitRequested = true;
    } else {
        out << "Invalid command." << endl;
    }
}

// --- individual commands ---

void GameController::cmdHelp(std::ostream &out) const {
    out << "Valid commands:\n"
        << "board\n"
        << "status\n"
        << "criteria\n"
        << "achieve <goal>\n"
        << "complete <criterion>\n"
        << "improve <criterion>\n"
        // trade not yet implemented in this refactor:
        // << "trade <colour> <give> <take>\n"
        << "next\n"
        << "save <file>\n"
        << "help" << endl;
}

void GameController::cmdBoard(std::ostream &out) const {
    board.printBoard(out);
}

void GameController::cmdStatus(std::ostream &out) const {
    board.printStatus(out);
}

void GameController::cmdCriteria(std::ostream &out) const {
    board.printCriteria(currentPlayer, out);
}

// --- dice / geese ---

void GameController::cmdRoll(std::istream &in, std::ostream &out) {
    if (rolledThisTurn) {
        out << "You have already rolled this turn." << endl;
        return;
    }
    if (awaitingGeesePlacement) {
        out << "You must move the GEESE before rolling again." << endl;
        return;
    }

    Player *p = board.getPlayer(currentPlayer);
    if (!p) {
        throw std::runtime_error("roll: unknown current player.");
    }

    // If using loaded dice, ask for a value between 2 and 12.
    if (p->isLoadedDice()) {
        int toLoad;
        while (true) {
            out << "Input a roll between 2 and 12:" << std::endl;
            out << "> ";
            if (!(in >> toLoad)) {
                // bad input: clear and ignore one token
                in.clear();
                std::string junk;
                in >> junk;
                out << "Invalid input." << std::endl;
                continue;
            }
            if (toLoad < 2 || toLoad > 12) {
                out << "Invalid roll." << std::endl;
                continue;
            }
            break;
        }
        p->setLoadedRoll(toLoad);

        // Consume the leftover newline so the next std::getline in run() works:
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }


    int roll = board.rollDice(currentPlayer);
    rolledThisTurn = true;

    if (roll == 7) {
        awaitingGeesePlacement = true;
        out << "Choose where to place the GEESE." << endl;
        // GameController will then expect a 'geese <tileId>' command.
    }
}

void GameController::cmdGeese(int tileId, std::istream &in, std::ostream &out) {
    if (!awaitingGeesePlacement) {
        out << "You may only move the GEESE immediately after rolling a 7." << std::endl;
        return;
    }

    // Move the geese; GameBoard will validate tileId.
    // If this throws, let the exception propagate so handleCommand knows it failed
    board.moveGeese(currentPlayer, tileId);

    // Determine who can be stolen from on this tile.
    auto stealable = board.getStealableColoursOnTile(tileId, currentPlayer);

    if (stealable.empty()) {
        // Edge case: no one else here has resources.
        out << "Student " << colourToString(currentPlayer)
            << " has no students to steal from." << std::endl;
        awaitingGeesePlacement = false;
        return;
    }

    // Print list of possible victims.
    out << "Student " << colourToString(currentPlayer)
        << " can choose to steal from ";
    for (size_t i = 0; i < stealable.size(); ++i) {
        if (i > 0) out << ", ";
        out << colourToString(stealable[i]);
    }
    out << "." << std::endl;

    // Ask which student to steal from.
    out << "Choose a student to steal from." << std::endl;
    out << "> ";

    Colour targetColour;
    std::string targetStr;

    while (true) {
        if (!(in >> targetStr)) {
            // Input error / EOF: just bail out gracefully.
            in.clear();
            return;
        }

        if (!parseColour(targetStr, targetColour)) {
            out << "Invalid player. Valid players: Blue, Red, Orange, Yellow" << std::endl;
        } else {
            bool isAllowed = false;
            for (Colour c : stealable) {
                if (c == targetColour) {
                    isAllowed = true;
                    break;
                }
            }
            if (!isAllowed) {
                out << "You must choose a student from the list." << std::endl;
            } else {
                break;  // valid choice
            }
        }

        out << "> ";
    }

    // Now actually steal one random resource from targetColour.
    Player *thief = board.getPlayer(currentPlayer);
    Player *victim = board.getPlayer(targetColour);

    if (!thief || !victim) {
        throw std::runtime_error("Internal error: player not found for geese stealing.");
    }

    int totalRes = victim->numResources();
    if (totalRes <= 0) {
        // Should not happen because we filtered on resources, but be safe.
        out << "Student " << colourToString(currentPlayer)
            << " has no students to steal from." << std::endl;
        awaitingGeesePlacement = false;
        return;
    }

    // Build a pool of resource cards proportional to counts.
    std::vector<Resources> pool;
    pool.reserve(totalRes);

    const Resources types[5] = {
        Resources::Caffeine,
        Resources::Lab,
        Resources::Lecture,
        Resources::Study,
        Resources::Tutorial
    };

    for (Resources r : types) {
        int count = victim->getResourceCount(r);
        for (int i = 0; i < count; ++i) {
            pool.push_back(r);
        }
    }

    if (pool.empty()) {
        out << "Student " << colourToString(currentPlayer)
            << " has no students to steal from." << std::endl;
        awaitingGeesePlacement = false;
        return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, static_cast<int>(pool.size() - 1));

    Resources stolen = pool[dist(gen)];

    // Transfer 1 resource card.
    victim->removeResource(stolen, 1);
    thief->addResource(stolen, 1);

    out << "Student " << colourToString(currentPlayer)
        << " steals " << resourceToSpecName(stolen)
        << " from student " << colourToString(targetColour) << "." << std::endl;

    // Clear to end of line so the next getline in run() is clean.
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    
    // Clear the awaiting flag so the player can continue their turn
    awaitingGeesePlacement = false;
}


// --- build / improve ---

void GameController::cmdComplete(int vertexId, std::ostream &out) {
    try {
        board.completeVertex(currentPlayer, vertexId);
        checkForWinner(out);
    } catch (const std::exception &e) {
        out << e.what() << endl;
    }
}

void GameController::cmdImprove(int vertexId, std::ostream &out) {
    try {
        board.improveVertex(currentPlayer, vertexId);
        checkForWinner(out);
    } catch (const std::exception &e) {
        out << e.what() << endl;
    }
}

void GameController::cmdAchieve(int edgeId, std::ostream &out) {
    try {
        board.achieveEdge(currentPlayer, edgeId);
        checkForWinner(out);
    } catch (const std::exception &e) {
        out << e.what() << endl;
    }
}

// --- save

void GameController::cmdSave(const std::string &filename, std::ostream &out) {
    try {
        SaveManager::saveGame(board, currentPlayer, filename);
        out << "Saved game to '" << filename << "'." << endl;
    } catch (const std::exception &e) {
        out << e.what() << endl;
    }
}

void GameController::cmdTrade(const std::string &targetStr,
                              const std::string &giveStr,
                              const std::string &takeStr,
                              std::istream &in,
                              std::ostream &out) {
    bool bankTrade = false;
    Colour otherColour;

    if (targetStr == "bank" || targetStr == "Bank") {
        bankTrade = true;
    } else if (!parseColour(targetStr, otherColour)) {
        out << "Invalid player. Valid players: Blue, Red, Orange, Yellow, bank\n";
        return;
    }

    Resources give, take;
    if (!parseResource(giveStr, give)) {
        out << "Invalid resource to give. Valid: Caffeine, Lab, Lecture, Study, Tutorial\n";
        return;
    }
    if (!parseResource(takeStr, take)) {
        out << "Invalid resource to take. Valid: Caffeine, Lab, Lecture, Study, Tutorial\n";
        return;
    }

    Player *me = board.getPlayer(currentPlayer);
    if (!me) {
        throw std::runtime_error("Internal error: current player not found");
    }

    // ----- Bank trade: 4 of give for 1 of take -----
    if (bankTrade) {
        if (me->getResourceCount(give) < 4) {
            out << "Student " << currentPlayer << " does not have enough "
                << give << " to trade with the bank. Trade unsuccessful.\n";
            return;
        }

        out << "Student " << currentPlayer << " wants to trade four " << give
            << " for one " << take << " with the bank. Confirm this trade?\n";
        out << "> ";

        std::string answer;
        in >> answer;
        while (answer != "yes" && answer != "no") {
            out << "Please confirm with yes or no.\n";
            out << "> ";
            in >> answer;
        }
        if (answer == "no") {
            out << "Trade unsuccessful.\n";
            return;
        }

        me->removeResource(give, 4);
        me->addResource(take, 1);
        out << "Trade successful.\n";
        return;
    }

    // ----- Player-to-player trade -----
    if (otherColour == currentPlayer) {
        out << "You cannot trade with yourself.\n";
        return;
    }

    Player *other = board.getPlayer(otherColour);
    if (!other) {
        out << "Unknown player.\n";
        return;
    }

    if (me->getResourceCount(give) < 1) {
        out << "Student " << currentPlayer << " does not have enough "
            << give << " to trade. Trade unsuccessful.\n";
        return;
    }
    if (other->getResourceCount(take) < 1) {
        out << "Student " << otherColour << " does not have enough "
            << take << " to trade. Trade unsuccessful.\n";
        return;
    }

    out << "Student " << currentPlayer << " offers Student " << otherColour
        << " one " << give << " for one " << take
        << ". Does Student " << otherColour << " accept this offer?\n";
    out << "> ";

    std::string answer;
    in >> answer;
    while (answer != "yes" && answer != "no") {
        out << "Please accept or decline the trade offer with yes or no.\n";
        out << "> ";
        in >> answer;
    }
    if (answer == "no") {
        out << "Trade unsuccessful.\n";
        return;
    }

    // Perform swap
    me->removeResource(give, 1);
    other->addResource(give, 1);

    other->removeResource(take, 1);
    me->addResource(take, 1);

    out << "Trade successful.\n";
}


// --- winner check ---

void GameController::checkForWinner(std::ostream &out) {
    if (board.hasWinner()) {
        Colour winner = board.getWinner();
        out << "Student " << colourToString(winner)
            << " has reached 10 course criteria!" << endl;
        quitRequested = true;
    }
}

void GameController::cmdSetFairDice(std::ostream &out) {
    Player *p = board.getPlayer(currentPlayer);
    if (!p) {
        throw std::runtime_error("fair: unknown current player.");
    }
    p->useFairDice();
    out << "Using fair dice this turn." << std::endl;
}

void GameController::cmdSetLoadedDice(std::ostream &out) {
    Player *p = board.getPlayer(currentPlayer);
    if (!p) {
        throw std::runtime_error("load: unknown current player.");
    }
    p->useLoadedDice();
    out << "Using loaded dice this turn." << std::endl;
}
