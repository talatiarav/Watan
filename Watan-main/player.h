export module Player;

import <iosfwd>;
import <map>;
import <memory>;
import <string>;
import <vector>;
import <iostream>;

import Colour;
import Resources;
import Assessment;
import Dice;

// Forward declarations to avoid circular include.
class Vertex;
class Edge;

/**
 * Player
 *
 * Refactor of old Student:
 *  - Tracks colour, resources, owned vertices (criteria) and edges (goals).
 *  - Knows how to:
 *      * roll its dice
 *      * check/spend resources for builds/improvements
 *      * lose resources to the geese
 *      * encode itself into the savefile format
 *      * print status / completed criteria summaries
 */
export class Player {
    Colour colour;
    std::map<Resources, int> resources;
    std::vector<Vertex *>    ownedVertices; // in order of completion
    std::vector<Edge  *>     ownedEdges;    // in order of achievement
    std::unique_ptr<Dice>    dice;          // fair/loaded dice for this player

public:
    explicit Player(Colour colour);

    // --- identity / dice ---

    Colour getColour() const { return colour; }

    // Replace this player’s dice with a new dice object.
    // EDIT: I'm not sure if we need this particularly since i've also added the functions below which
    // characterizes the dice effectively
    void setDice(std::unique_ptr<Dice> newDice);

    // Dice control
    void useFairDice();
    void useLoadedDice();
    bool isLoadedDice() const;
    void setLoadedRoll(int value);   // only meaningful if currently using loaded dice

    // Rolls this player’s dice. If no dice yet, defaults to fair.
    int rollDice();

    // --- resources / ownership ---

    // Adjust resources (positive or negative).
    void addResources(Resources resource, int amount);

    // Total number of resources (sum over all types).
    int numResources() const;

    // Direct read-only access if needed by SaveManager/Controller.
    const std::map<Resources,int> &getResourceMap() const { return resources; }

    // Record that this player now owns this vertex/edge.
    // These should be called when a build/achievement actually succeeds.
    void addVertex(Vertex *v);
    void addEdge(Edge *e);

    // --- saving ---

    // Encodes just the resources: "<numCaffeines> <numLabs> <numLectures> <numStudies> <numTutorials>"
    std::string encodeResourcesForSave() const;

    // Encodes achieved goals: "<goalId> ..." (space-separated, in order).
    std::string encodeGoalsForSave() const;

    // Encodes completed criteria: "<vertexId> <upgradeNum> ..." where upgradeNum:
    // 1 = Assignment, 2 = Midterm, 3 = Exam.
    std::string encodeVerticesForSave() const;

    // Full save line:
    // "<numCaffeines> <numLabs> <numLectures> <numStudies> <numTutorials> g <goals> c <criteria>"
    std::string encodeForSave() const;

    // --- points / rules ---

    // Total course criteria points:
    // Assignment = 1, Midterm = 2, Exam = 3.
    int getPoints() const;

    // Check if this player has enough resources to perform an action of the given type.
    bool resourcesCheck(Assessment type) const;

    // Deduct resources for an action of the given type (assumes resourcesCheck(type) == true).
    void resourcesSpent(Assessment type);

    // Geese: if numResources() >= 10, loses half (rounded down), printing loss breakdown.
    void loseResourcesToGeese(std::ostream &out = std::cout);

    // Clear everything (used when resetting game state).
    void reset();

    // --- printing / status ---

    // "<colour> has <numCC> course criteria, <numCaffeines> caffeines, <numLabs> labs, <numLectures> lectures, <numStudies> studies, and <numTutorials> tutorials."
    void printStatus(std::ostream &out) const;

    // Prints detailed completions, as per spec 2.7:
    // "<colour> has completed:\n<vertexId> <upgradeNum>\n..."
    void printCriteria(std::ostream &out) const;

    // Serialize this player's state in watan-savefile format:
    // "<Caff> <Lab> <Lect> <Study> <Tut> g <edges...> c <vertex level>..."

    // For SaveManager: overwrite this player's resources with the given counts.
    void setResourcesFromSave(int caffeines,
                              int labs,
                              int lectures,
                              int studies,
                              int tutorials);

    int getResourceCount(Resources r) const;
    void addResource(Resources r, int amount);
    void removeResource(Resources r, int amount);

private:
    // Helper to format the resource counts as a human-readable string.
    // "<numCaffeines> caffeines, <numLabs> labs, <numLectures> lectures, <numStudies> studies, and <numTutorials> tutorials"
    std::string formatResourcesStatus() const;
};
