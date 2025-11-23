#include "gamecontroller.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

#include "gameboard.h"
#include "savemanager.h"


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
    startNewTurn(out);

    std::string line;
    while (!quitRequested && !board.hasWinner() && std::getline(in, line)) {
        if (line.empty()) continue;

        try {
            handleCommand(line, out);
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

void GameController::handleCommand(const std::string &line, std::ostream &out) {
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;
    if (cmd.empty()) return;

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
        cmdRoll(out);
    } else if (cmd == "geese") {
        int tileId;
        if (!(iss >> tileId)) {
            out << "Invalid command." << endl;
        } else {
            cmdGeese(tileId, out);
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

void GameController::cmdRoll(std::ostream &out) {
    if (rolledThisTurn) {
        out << "You have already rolled this turn." << endl;
        return;
    }
    if (awaitingGeesePlacement) {
        out << "You must move the GEESE before rolling again." << endl;
        return;
    }

    int roll = board.rollDice(currentPlayer);
    rolledThisTurn = true;

    if (roll == 7) {
        awaitingGeesePlacement = true;
        out << "Student " << colourToString(currentPlayer)
            << ", choose where to place the GEESE." << endl;
        // GameController will then expect a 'geese <tileId>' command.
    }
}

void GameController::cmdGeese(int tileId, std::ostream &out) {
    if (!awaitingGeesePlacement) {
        out << "You may only move the GEESE immediately after rolling a 7." << endl;
        return;
    }

    try {
        board.moveGeese(currentPlayer, tileId);
        awaitingGeesePlacement = false;

        // Optional: show potential stealing targets.
        auto stealable = board.getStealableColoursOnTile(tileId, currentPlayer);
        if (!stealable.empty()) {
            out << "You may steal from: ";
            for (size_t i = 0; i < stealable.size(); ++i) {
                if (i > 0) out << ", ";
                out << colourToString(stealable[i]);
            }
            out << "." << endl;
            out << "(Stealing logic not implemented yet.)" << endl;
        }
    } catch (const std::exception &e) {
        out << e.what() << endl;
    }
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


// --- winner check ---

void GameController::checkForWinner(std::ostream &out) {
    if (board.hasWinner()) {
        Colour winner = board.getWinner();
        out << "Student " << colourToString(winner)
            << " has reached 10 course criteria!" << endl;
        quitRequested = true;
    }
}
