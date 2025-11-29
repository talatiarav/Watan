#ifndef BOARDVIEW_H
#define BOARDVIEW_H

#include <iosfwd>
#include <string>
#include <vector>

#include "colour.h"
#include "resources.h"
#include "assessment.h"

// Forward declarations to avoid circular includes
class Vertex;
class Edge;

/**
 * BoardView
 *
 * ASCII text display for the Watan board.
 * Refactor of old TextDisplay:
 *  - tracks criteria (vertices) and goals (edges) display tokens
 *  - tracks tile resources, values, and indices
 *  - tracks geese position
 *  - supports enhanced (coloured) and plain modes
 */
class BoardView {
    int  geeseAt  = -1;
    bool enhance  = false;

    // Display strings for vertices (criteria) and edges (goals)
    std::vector<std::string> criteriaString; // size 54
    std::vector<std::string> goalsString;    // size 72

    // Per-tile strings (19 tiles)
    std::vector<std::string> resourcesString;    // centred resource name
    std::vector<std::string> valuesString;       // centred value (blank for Netflix)
    std::vector<std::string> tileNumberString = {
        "       0     ", "       1     ",
        "       2     ", "       3     ", "       4     ",
        "       5     ", "       6     ", "       7     ",
        "       8     ", "       9     ", "      10     ",
        "      11     ", "      12     ", "      13     ",
        "      14     ", "      15     ", "      16     ",
        "      17     ", "      18     "
    };

    // Helpers
    static std::string centre(const std::string &s, int width);

public:
    // values/resources are size 19, matching the 19 tiles
    BoardView(bool enhance,
              const std::vector<int>       &values,
              const std::vector<Resources> &resources);

    // Notifications from model
    void notify(Vertex *vertex);
    void notify(Edge   *edge);
    void notifyGeese(int tileId);

    // Render whole board
    void render(std::ostream &out) const;
};

#endif // BOARDVIEW_H
