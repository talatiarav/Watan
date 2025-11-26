#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <iosfwd>
#include <string>

#include "colour.h"
#include "gameboard.h"

/**
 * GameController
 *
 * High-level orchestration of the game:
 *  - manages turn order
 *  - parses text commands from an input stream
 *  - calls into GameBoard to perform game actions
 *  - enforces basic turn rules (roll once per turn, must roll before next)
 *
 * This replaces the old main.cc + Board-driven loop.
 */
class GameController {
public:
    // Takes ownership of an already-constructed GameBoard.
    explicit GameController(GameBoard &&board);

    GameController(GameBoard &&board, Colour startingPlayer);

    // Main game loop: reads commands until a winner, quit, or EOF.
    void run(std::istream &in, std::ostream &out);

private:
    GameBoard board;   // owned board

    Colour currentPlayer;
    bool rolledThisTurn = false;
    bool awaitingGeesePlacement = false;
    bool quitRequested = false;

    // --- turn / flow helpers ---
    void setupInitialAssignments(std::istream &in, std::ostream &out);

    void startNewTurn(std::ostream &out);
    void advancePlayer();
    static std::string colourToString(Colour c);

    // --- command handling ---
    void handleCommand(const std::string &line,
                       std::istream &in,
                       std::ostream &out);

    void cmdHelp(std::ostream &out) const;
    void cmdBoard(std::ostream &out) const;
    void cmdStatus(std::ostream &out) const;
    void cmdCriteria(std::ostream &out) const;

    void cmdRoll(std::istream &in, std::ostream &out);
    void cmdGeese(int tileId, std::ostream &out);

    void cmdComplete(int vertexId, std::ostream &out);
    void cmdImprove(int vertexId, std::ostream &out);
    void cmdAchieve(int edgeId, std::ostream &out);

    void cmdSave(const std::string &filename, std::ostream &out);

    void cmdTrade(const std::string &targetStr,
                  const std::string &giveStr,
                  const std::string &takeStr,
                  std::istream &in,
                  std::ostream &out);

    void cmdSetFairDice(std::ostream &out);
    void cmdSetLoadedDice(std::ostream &out);

    // After any build/improve, check if someone has 10 points.
    void checkForWinner(std::ostream &out);
};

#endif // GAMECONTROLLER_H
