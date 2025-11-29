export module SaveManager;

import <string>;

import Colour;

class GameBoard;

// Handles saving and loading game state to and from files.
// Reads/writes player data, board configuration, and geese position
// using the format specified in the Watan project document.

export class SaveManager {
public:
    // Writes the game to <filename> in the watan-savefile format:
    //
    // Line 1: <curTurnIndex>
    //          where 0=Blue, 1=Red, 2=Orange, 3=Yellow
    // Lines 2-5: one line per player in that fixed order, each via Player::encodeForSave()
    // Line 6: board layout (see GameBoard::encodeBoardLayoutForSave)
    // Line 7: geese tile index (or -1 if not placed)
    static void saveGame(const GameBoard &board,
                         Colour currentPlayer,
                         const std::string &filename);

    // Skeleton for loading (not fully implemented yet):
    //
    //   - `enhance` is passed through to GameBoard ctor (for BoardView style)
    //   - `currentPlayerOut` is set to the player whose turn it is
    static GameBoard loadGame(bool enhance,
                              const std::string &filename,
                              Colour &currentPlayerOut);
};
