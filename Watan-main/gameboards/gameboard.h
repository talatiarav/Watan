#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <iosfwd>
#include <memory>
#include <vector>
#include <stdexcept>

// Forward declarations for new types.
// These will be defined in your new headers later.
enum class Colour;      // Based on old enum Player {Blue, Red, ...}
enum class Resources;   // Based on old enum Resource {Caffeine, Lab, ...}
enum class Assessment;  // Based on old enum Type {Achievement, Assignment, ...}

class Player;
class Tile;
class Vertex;
class Edge;
class BoardView;

/**
 * GameBoard
 *
 * Pure game model of the board state:
 *  - owns Tiles, Vertices, Edges, Players, BoardView, geese location
 *  - enforces high-level game rules (rolling, building, improving, achieving)
 *  - has NO direct input (no cin)
 *  - exposes high-level operations used by GameController & SaveManager.
 *
 * NOTE: This is the refactored replacement for the old Board class
 * (board.h/board.cc) .
 */
class GameBoard {
    // --- Data ---
    std::vector<std::unique_ptr<Tile>>   tiles;    // size 19
    std::vector<std::unique_ptr<Vertex>> vertices; // size 54
    std::vector<std::unique_ptr<Edge>>   edges;    // size 72
    std::vector<std::unique_ptr<Player>> players;  // exactly 4 players
    std::unique_ptr<BoardView>           view;     // ASCII view (TextDisplay-based)
    int                                   geeseTile; // -1 = not on board

public:
    /**
     * Construct a GameBoard from an explicit layout.
     *
     * @param enhance   whether to use coloured ANSI-enhanced output in the view
     *                  (like TextDisplay's 'enhance' flag )
     * @param values    dice values for each tile (size must be 19)
     * @param resources resource type for each tile (size must be 19)
     *
     * Throws std::invalid_argument if sizes are incorrect.
     */
    GameBoard(bool enhance,
              const std::vector<int> &values,
              const std::vector<Resources> &resources);

    /**
     * Factory to build a randomly-generated board using the distribution
     * from the spec (3 TUTORIAL, 3 STUDY, 4 CAFFEINE, 4 LAB, 4 LECTURE, 1 NETFLIX
     * and the "one 2, one 12, two 3-6, two 8-11" value pattern) :contentReference[oaicite:5]{index=5}.
     *
     * Equivalent to the randomized constructor of old Board :contentReference[oaicite:6]{index=6}.
     */
    static GameBoard createRandom(bool enhance);

    // --- Core game actions ---

    /**
     * Roll dice for the active player and distribute resources or trigger geese.
     *
     * Behaviour:
     *  - Calls Player::rollDice() on the specified player.
     *  - If roll == 7:
     *      * Checks each player; anyone with >= 10 resources loses half
     *        via Player::loseResourcesToGeese() (see Student::loseResources) :contentReference[oaicite:7]{index=7}.
     *      * Does NOT move the geese; GameController should subsequently call moveGeese().
     *  - If roll != 7:
     *      * For each Tile whose value == roll and !hasGeese(),
     *        calls Tile::sendResources().
     *      * If no-one gained, prints "No students gained resources."
     *        (same semantics as Board::roll) :contentReference[oaicite:8]{index=8}.
     *
     * Throws std::runtime_error if the Colour does not map to a known Player.
     */
    void rollDice(Colour activePlayer);

    /**
     * Complete a vertex (course criterion) during the main game (not setup).
     *
     * Enforces:
     *  - Vertex::canBeCompletedBy(colour) (adjacency/occupancy rules).
     *  - Player::resourcesCheck(Assessment::Assignment).
     *  - Updates Player (resourcesSpent, ownership tracking, points).
     *  - Updates Vertex (owner + assessment) via Vertex::complete(colour).
     *  - Notifies the BoardView (Vertex will notify observers).
     *
     * Throws:
     *  - std::runtime_error("You cannot build here.")    if illegal placement
     *  - std::runtime_error("You do not have enough resources.") if short on resources.
     */
    void completeVertex(Colour player, int vertexId);

    /**
     * Improve an already-completed vertex (Assignment -> Midterm -> Exam).
     *
     * Enforces:
     *  - Vertex::canBeImprovedBy(colour) (ownership & max level).
     *  - Player::resourcesCheck(Assessment::Midterm or ::Exam, based on current level).
     *  - Updates Player (resourcesSpent).
     *  - Updates Vertex::improve().
     *
     * Throws std::runtime_error on illegal build or insufficient resources,
     * with the same messages as completeVertex().
     */
    void improveVertex(Colour player, int vertexId);

    /**
     * Achieve an edge (goal).
     *
     * Enforces:
     *  - Edge::canBeAchievedBy(colour) (adjacent owned vertex/edge) :contentReference[oaicite:9]{index=9}.
     *  - Player::resourcesCheck(Assessment::Achievement)
     *      (one STUDY and one TUTORIAL resource).
     *  - Updates Player::resourcesSpent(Assessment::Achievement).
     *  - Updates Edge::achieve(colour).
     *
     * Throws std::runtime_error on illegal build or insufficient resources.
     */
    void achieveEdge(Colour player, int edgeId);

    /**
     * Move the geese to a new tile (called after rolling a 7).
     *
     * Behaviour:
     *  - Turns off geese flag on previous tile (if any).
     *  - Validates tileId (0–18 and not equal to current geese tile).
     *  - Sets geeseTile and toggles geese on the new Tile.
     *  - Notifies BoardView via BoardView::notifyGeese(tileId).
     *
     * NOTE: This does NOT perform stealing. GameController should:
     *   - Call getStealableColoursOnTile(tileId, activePlayer),
     *   - Ask user which Colour to steal from (if any),
     *   - Then call Player::stealFrom(victim).
     *
     * Throws std::invalid_argument on invalid tileId or same as current.
     */
    void moveGeese(Colour activePlayer, int tileId);

    // --- Query / printing helpers ---

    /**
     * Render the board to the given stream via BoardView.
     * Equivalent to: std::cout << *td; in the old code .
     */
    void printBoard(std::ostream &out) const;

    /**
     * Print status of all players in fixed order:
     * Blue, Red, Orange, Yellow.
     *
     * Delegates to Player::printStatus(out), similar to Board::status()
     * and Student::printStatus() .
     */
    void printStatus(std::ostream &out) const;

    /**
     * Print only the criteria (completions) of the given player's vertices.
     *
     * Delegates to Player::printCriteria(out), which mirrors
     * Student::printCriteria semantics. :contentReference[oaicite:12]{index=12}
     */
    void printCriteria(Colour player, std::ostream &out) const;

    /**
     * Returns true if any player has at least 10 course criteria (points).
     * Uses Player::getPoints() (port of Student::getPoints) :contentReference[oaicite:13]{index=13}.
     */
    bool hasWinner() const;

    /**
     * Returns the Colour of the winning player.
     * Precondition: hasWinner() == true.
     *
     * Throws std::logic_error if no winner yet.
     */
    Colour getWinner() const;

    // --- Helper accessors used by SaveManager / GameController ---

    /**
     * Return the zero-based tile index where the geese currently are,
     * or -1 if they are not on the board.
     */
    int getGeeseTile() const { return geeseTile; }

    /**
     * Convenience: return a list of Colours that can be stolen from on
     * the given tile, excluding the active player.
     *
     * Implemented by calling Tile::playersToStealFrom(active) and mapping
     * the result string to Colour values (mirrors old Tile::playersToStealFrom) .
     */
    std::vector<Colour> getStealableColoursOnTile(int tileId, Colour active) const;

    /**
     * Expose read-only access to Tiles for SaveManager, etc.
     */
    const std::vector<std::unique_ptr<Tile>> &getTiles() const { return tiles; }

    /**
     * Expose read-only access to Players for SaveManager, etc.
     */
    const std::vector<std::unique_ptr<Player>> &getPlayers() const { return players; }

private:
    // Look up a Player by Colour (non-const and const versions).
    Player       *findPlayer(Colour colour);
    const Player *findPlayer(Colour colour) const;

    // Validate vertex/edge indices and return raw pointers.
    Vertex       *getVertex(int vertexId);
    const Vertex *getVertex(int vertexId) const;

    Edge         *getEdge(int edgeId);
    const Edge   *getEdge(int edgeId) const;

    // Internal helper to build the 4 players.
    void initializePlayers();

    // Internal helper to build tiles/vertices/edges and hook them up
    // to the view (based on the same topology as in Board::rowSetup,
    // update, updateCriterionsInTile, updateCriterionsNeighbor) :contentReference[oaicite:15]{index=15}.
    void initializeBoardGraph(const std::vector<int> &values,
                              const std::vector<Resources> &resources,
                              bool enhance);
};

#endif // GAMEBOARD_H
