export module BoardView;

import <iosfwd>;
import <string>;
import <vector>;

import Colour;
import Resources;
import Assessment;

class Vertex;
class Edge;

// This class is the View in our MVC setup. It prints the Watan board in ASCII form.
// It keeps track of what each vertex and edge should look like, updates them when the model changes,
// and draws the full board with resources, values, and the GEESE when render() is called.

export class BoardView {
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
