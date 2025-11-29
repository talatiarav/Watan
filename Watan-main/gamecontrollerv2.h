export module GameController;

import <iosfwd>;
import <string>;

import Colour;
import GameBoard;

// Handles the main flow of the game: turns, commands, dice rolling,
// trading, building actions, saving/loading, and coordinating with the GameBoard.
// Acts as the controller in the MVC structure, interpreting user input
// and telling the model (board/players) what actions to perform.

export class GameController {
public:
    explicit GameController(GameBoard &&board);

    GameController(GameBoard &&board, Colour startingPlayer);

    // Main game loop: reads commands until a winner, quit, or EOF.
    // Returns true if the player wants to play again, false otherwise.
    bool run(std::istream &in, std::ostream &out, bool loadedFile);

private:
    GameBoard board;   // owned board

    Colour currentPlayer;
    bool rolledThisTurn = false;
    bool awaitingGeesePlacement = false;
    bool quitRequested = false;

    void setupInitialAssignments(std::istream &in, std::ostream &out);

    void startNewTurn(std::ostream &out);
    void advancePlayer();
    static std::string colourToString(Colour c);

    void handleCommand(const std::string &line,
                       std::istream &in,
                       std::ostream &out);

    void cmdHelp(std::ostream &out) const;
    void cmdBoard(std::ostream &out) const;
    void cmdStatus(std::ostream &out) const;
    void cmdCriteria(std::ostream &out) const;

    void cmdRoll(std::istream &in, std::ostream &out);
    void cmdGeese(int tileId, std::istream &in, std::ostream &out);

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

    void checkForWinner(std::ostream &out);

    void saveBackupOnEOF(std::ostream &out);
};
