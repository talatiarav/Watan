export module Tile;

import <vector>;
import <string>;

import Subject;
import Resources;

// Forward declarations
class Vertex;
class Player;
enum class Colour : int;

// Represents a single board tile with a resource type and value.
// Tracks which criteria surround it and whether the GEESE are on it.
// Used to determine resource production when dice are rolled.

export class Tile : public Subject {
    int id;
    int value;
    Resources resource;
    bool geeseHere;
    std::vector<Vertex*> vertices;

public:
    // Constructor
    Tile(int id, Resources resource, int value);
    
    // Accessors
    int getId() const;
    int getValue() const;
    Resources getResource() const;
    bool hasGeese() const;
    
    // Modifiers
    void toggleGeese();
    bool sendResources();
    std::string playersToStealFrom(Colour active) const;
    void addVertex(Vertex* v);
};
