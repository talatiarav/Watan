#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <iosfwd>
#include <memory>
#include <vector>

// Forward declarations of new enums/types.
// We only need full definitions in the .cc file.
enum class Colour;
enum class Resources;
enum class Assessment;

class Player;
class Tile;
class Vertex;
class Edge;
class BoardView;

/**
 * GameBoard
 *
 * Pure game model of the board state.
 * Responsibilities:
 *  - Owns Tiles, Vertices, Edges, Players, geese location, and BoardView.
 *  - Knows the graph connectivity between tiles/vertices/edges.
 *  - Implements high-level game rules:
 *      * rolling dice (resource distribution / geese)
 *      * completevertex / improvevertex / achieveedge
 *      * moving the geese, winner detection, etc.
 *
 * No input from cin; minimal printing (delegated to Player / BoardView / Tile).
 */
class GameBoard {
    // --- Data ---
    std::vector<std::unique_ptr<Tile>>   tiles;    // 19 tiles
    std::vector<std::unique_ptr<Vertex>> vertices; // 54 intersections
    std::vector<std::unique_ptr<Edge>>   edges;    // 72 edges
    std::vector<std::unique_ptr<Player>> players;  // exactly 4 players
    std::unique_ptr<BoardView>           view;     // ASCII display
    int                                   geeseTile = -1; // -1 = not on board

public:
    /**
     * Construct a GameBoard from an explicit layout.
     *
     * @param enhance   ANSI colour / enhanced mode for BoardView
     * @param values    dice values per tile, length 19
     * @param resources resource type per tile, length 19
     *
     * Throws std::invalid_argument if sizes != 19.
     */
    GameBoard(bool enhance,
              const std::vector<int> &values,
              const std::vector<Resources> &resources);

    /**
     * Factory for random board, matching the spec’s distribution of
     * values and resources.
     */
    static GameBoard createRandom(bool enhance);

    // --- Core game actions ---

    /**
     * Roll dice for the active player and handle resource distribution or geese.
     *
     * Semantics (port of old Board::roll):
     *  - Calls Player::rollDice() on the given Colour.
     *  - If roll == 7:
     *      * Any player with >= 10 resources loses half (rounded down),
     *        via Player::loseResourcesToGeese() (port of Student::loseResources()).
     *      * GameBoard does NOT move the geese; GameController must call moveGeese()
     *        afterwards and then drive the steal interaction.
     *  - If roll != 7:
     *      * For each tile whose value == roll, calls Tile::sendResources().
     *      * If no tile sends resources, prints the usual
     *            "No students gained resources."
     *
     * Throws std::runtime_error if the Colour isn’t found.
     */
    void rollDice(Colour activePlayer);

    /**
     * Complete a vertex (criterion) as an Assignment.
     *
     * Rules:
     *  - Vertex::canBeCompletedBy(player) must be true
     *      (no adjacent completion, adjacency to player’s goal unless in setup).
     *  - Player::resourcesCheck(Assessment::Assignment) must be true
     *      (1 Caffeine, 1 Lab, 1 Lecture, 1 Tutorial).
     *  - On success:
     *      * Vertex::complete(player)
     *      * Player::resourcesSpent(Assignment)
     *
     * Throws std::runtime_error("You cannot build here.") on illegal placement.
     * Throws std::runtime_error("You do not have enough resources.") if broke.
     */
    void completeVertex(Colour player, int vertexId);

    /**
     * Improve an already-completed vertex (Assignment -> Midterm -> Exam).
     *
     * Rules:
     *  - Vertex::canBeImprovedBy(player) must be true (correct owner, not already Exam).
     *  - Determine next Assessment:
     *      Assignment → Midterm, Midterm → Exam, Exam/None → illegal.
     *  - Player::resourcesCheck(next) must be true.
     *
     * Same exceptions as completeVertex().
     */
    void improveVertex(Colour player, int vertexId);

    /**
     * Achieve an edge (goal).
     *
     * Rules:
     *  - Edge::canBeAchievedBy(player) (adjacent owned vertex/edge).
     *  - Player::resourcesCheck(Assessment::Achievement)
     *      (1 Study + 1 Tutorial).
     *  - On success:
     *      * Edge::achieve(player)
     *      * Player::resourcesSpent(Achievement)
     */
    void achieveEdge(Colour player, int edgeId);

    /**
     * Move the geese to a new tile (called after a 7 is rolled).
     *
     * Rules:
     *  - tileId must be in [0, 18] and != current geeseTile.
     *  - Clears previous tile’s geese flag, sets new tile’s flag.
     *  - Notifies BoardView about the new geese location.
     *
     * Does NOT perform stealing — GameController will:
     *  - getStealableColoursOnTile(tileId, active),
     *  - pick a victim (if any),
     *  - call Player::stealFrom(victim).
     *
     * Throws std::invalid_argument on invalid tileId.
     */
    void moveGeese(Colour activePlayer, int tileId);

    // --- Printing / game state queries ---

    /**
     * Render the board via BoardView.
     */
    void printBoard(std::ostream &out) const;

    /**
     * Print status of all players in fixed order:
     * Blue, Red, Orange, Yellow.
     */
    void printStatus(std::ostream &out) const;

    /**
     * Print the criteria summary for a single player
     * (port of Student::printCriteria + spec 2.7).
     */
    void printCriteria(Colour player, std::ostream &out) const;

    /**
     * Any player has >= 10 course criteria (points)?
     */
    bool hasWinner() const;

    /**
     * Return the winning Colour.
     * Precondition: hasWinner() == true.
     * Throws std::logic_error if called too early.
     */
    Colour getWinner() const;

    // --- Helpers for GameController / SaveManager ---

    int getGeeseTile() const { return geeseTile; }

    /**
     * Returns the list of Colours present on tileId that can be stolen from,
     * excluding the active player.
     *
     * Implemented by reading the string from Tile::playersToStealFrom(active)
     * ("Blue,Red,...") and mapping to Colour values.
     */
    std::vector<Colour> getStealableColoursOnTile(int tileId, Colour active) const;

    const std::vector<std::unique_ptr<Tile>>   &getTiles()   const { return tiles; }
    const std::vector<std::unique_ptr<Player>> &getPlayers() const { return players; }

private:
    Player       *findPlayer(Colour colour);
    const Player *findPlayer(Colour colour) const;

    Vertex       *getVertex(int vertexId);
    const Vertex *getVertex(int vertexId) const;

    Edge         *getEdge(int edgeId);
    const Edge   *getEdge(int edgeId) const;

    void initializePlayers();
    void initializeBoardGraph(const std::vector<int> &values,
                              const std::vector<Resources> &resources,
                              bool enhance);
};

#endif // GAMEBOARD_H
