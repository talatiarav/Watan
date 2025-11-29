export module Tile;

import <vector>;
import <string>;

import Subject;
import Resources;

// Forward declarations
class Vertex;
class Player;
enum class Colour : int;

/**
 * Tile
 * 
 * Represents a resource-producing hexagonal tile on the game board.
 * Inherits from Subject to support the Observer pattern.
 */
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
