// tile.h
#ifndef TILE_H
#define TILE_H

#include <vector>
#include <string>
#include "subject.h"   // Tile inherits from Subject

// Forward declarations for types used
class Vertex;
class Player;
enum class Colour : int;
enum class Resources : int;

class Tile : public Subject {
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
  void toggleGeese();               // flip geese flag and notify observers
  bool sendResources();             // distribute resources to adjacent vertices' owners if possible
  std::string playersToStealFrom(Colour active) const;  // list of stealable players on this tile
  void addVertex(Vertex* v);        // attach a vertex to this tile (called during board setup)
};

#endif
