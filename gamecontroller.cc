#include "gamecontroller.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

#include "gameboard.h"

using std::cout;
using std::endl;

GameController::GameController(GameBoard &&b)
    : board(ownedBoard),
      ownedBoard(std::move(b)),
      currentPlayer(Colour::Blue) {}

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
        handleCommand(line, in, out);
    }

    if (board.hasWinner()) {
        Colour winner = board.getWinner();
        out << "Student " << colourToString(winner)
            << " wins the game!" << endl;
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

    try {
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
                out << "Usage: geese <tileId>" << endl;
            } else {
                cmdGeese(tileId, out);
            }
        } else if (cmd == "build-res") {
            int vertexId;
            if (!(iss >> vertexId)) {
                out << "Usage: build-res <vertexId>" << endl;
            } else {
                cmdBuildResidence(vertexId, out);
            }
        } else if (cmd == "improve") {
            int vertexId;
            if (!(iss >> vertexId)) {
                out << "Usage: improve <vertexId>" << endl;
            } else {
                cmdImprove(vertexId, out);
            }
        } else if (cmd == "build-road") {
            int edgeId;
            if (!(iss >> edgeId)) {
                out << "Usage: build-road <edgeId>" << endl;
            } else {
                cmdBuildRoad(edgeId, out);
            }
        } else if (cmd == "save") {
            std::string filename;
            if (!(iss >> filename)) {
                out << "Usage: save <filename>" << endl;
            } else {
                cmdSave(filename, out);
            }
        } else if (cmd == "load") {
            std::string filename;
            if (!(iss >> filename)) {
                out << "Usage: load <filename>" << endl;
            } else {
                cmdLoad(filename, out);
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
            out << "Unknown command: " << cmd << ". Type 'help' for options." << endl;
        }
    } catch (const std::exception &e) {
        out << e.what() << endl;
    } catch (const char *msg) {
        out << msg << endl;
    }
}

// --- individual commands ---

void GameController::cmdHelp(std::ostream &out) const {
    out << "Available commands:\n"
        << "  help                 - show this help\n"
        << "  board                - print the game board\n"
        << "  status               - print all students' status\n"
        << "  criteria             - print current student's completed criteria\n"
        << "  roll                 - roll dice for current student\n"
        << "  geese <tileId>       - move geese to tile after rolling a 7\n"
        << "  build-res <vertex>   - build a residence at the given vertex\n"
        << "  improve <vertex>     - upgrade a residence at the given vertex\n"
        << "  build-road <edge>    - build a road (goal) on the given edge\n"
        << "  save <file>          - save the current game (TODO)\n"
        << "  load <file>          - load a saved game (TODO)\n"
        << "  next                 - end your turn\n"
        << "  quit                 - exit the game\n";
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

    int roll = board.rollDice(currentPlayer);  // <- GameBoard::rollDice should return int now
    rolledThisTurn = true;

    if (roll == 7) {
        awaitingGeesePlacement = true;
        out << "Student " << colourToString(currentPlayer)
            << ", choose where to place the GEESE (use 'geese <tileId>')." << endl;
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

        // Optionally: handle stealing here using board.getStealableColoursOnTile(...)
        // For now we just move the geese.
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

void GameController::cmdBuildResidence(int vertexId, std::ostream &out) {
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

void GameController::cmdBuildRoad(int edgeId, std::ostream &out) {
    try {
        board.achieveEdge(currentPlayer, edgeId);
        checkForWinner(out);
    } catch (const std::exception &e) {
        out << e.what() << endl;
    }
}

// --- save / load (stubs for now) ---

void GameController::cmdSave(const std::string &filename, std::ostream &out) {
    // TODO: hook into SaveManager once we define it.
    out << "Saving to '" << filename << "' is not implemented yet." << endl;
}

void GameController::cmdLoad(const std::string &filename, std::ostream &out) {
    // TODO: hook into SaveManager once we define it.
    out << "Loading from '" << filename << "' is not implemented yet." << endl;
}

// --- winner check ---

void GameController::checkForWinner(std::ostream &out) {
    if (board.hasWinner()) {
        Colour winner = board.getWinner();
        out << "Student " << colourToString(winner)
            << " has reached 10 course criteria!" << endl;
        quitRequested = true; // run() will print final message too
    }
}
