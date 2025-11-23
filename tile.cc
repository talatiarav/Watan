// tile.cc
#include "tile.h"
#include "vertex.h"
#include "player.h"
#include <algorithm>
#include <sstream>

// Constructor: initialize tile with given id, resource type, and dice value
Tile::Tile(int id, Resources resource, int value)
  : id{id}, value{value}, resource{resource}, geeseHere{false}, vertices{} {}

// Get the tile's unique identifier
int Tile::getId() const {
  return id;
}

// Get the dice value associated with this tile
int Tile::getValue() const {
  return value;
}

// Get the resource type produced by this tile
Resources Tile::getResource() const {
  return resource;
}

// Check if geese are currently on this tile
bool Tile::hasGeese() const {
  return geeseHere;
}

// Toggle the presence of geese on this tile and notify observers of the change
void Tile::toggleGeese() {
  geeseHere = !geeseHere;
  // Notify BoardView observers that geese moved (or were removed) at this tile
  for (Observer *obs : observers) {
    // The BoardView observer has a special handler for geese movement
    obs->notifyGeese(id);
  }
}

// Distribute this tile's resource to adjacent vertices' owners if conditions allow
// Returns true if at least one resource was sent to a player, false if none were distributed.
bool Tile::sendResources() {
  // If geese block this tile or if the tile produces no resources (e.g., NETFLIX), no distribution
  if (geeseHere || resource == Resources::NETFLIX) {
    return false;
  }
  bool distributed = false;
  // Give resources to each adjacent vertex owner based on their improvement level
  for (Vertex *v : vertices) {
    if (v == nullptr) continue;
    // Get the state of the vertex (owner and assessment level)
    State vState = v->getState();
    Colour ownerColour = vState.getColour();
    // If no one owns this vertex (colour is invalid or no improvement), skip
    if (ownerColour == Colour::None) {  // assuming Colour::None indicates no owner
      continue;
    }
    // Determine the number of resources to give based on the improvement (Assignment=1, Midterm=2, Exam=3)
    int reward = static_cast<int>(vState.getAssessment());
    if (reward <= 0) {
      continue;
    }
    // Find the Player object for this owner colour and add the resources
    // (Assumes a function or mapping exists to get Player* from Colour)
    Player *player = /* get Player by ownerColour */ nullptr;
    if (player) {
      player->addResource(resource, reward);
    }
    distributed = true;
  }
  return distributed;
}

// List the players (by colour name) on this tile that the active player can steal from
// Only players who have a completed criterion on this tile, who are not the active player, 
// and who have at least one resource are included.
std::string Tile::playersToStealFrom(Colour active) const {
  // Track which player colours are eligible
  bool eligible[4] = {false, false, false, false};
  // Check each adjacent vertex for an eligible player
  for (Vertex *v : vertices) {
    if (v == nullptr) continue;
    State vState = v->getState();
    Colour ownerColour = vState.getColour();
    // Skip if no owner or if the owner is the active player
    if (ownerColour == Colour::None || ownerColour == active) {
      continue;
    }
    // Check that the owner has at least one resource card
    Player *ownerPlayer = /* get Player by ownerColour */ nullptr;
    if (!ownerPlayer) {
      continue;
    }
    int totalRes = 0;
    // Sum all resource counts for this player
    totalRes += ownerPlayer->getResourceCount(Resources::CAFFEINE);
    totalRes += ownerPlayer->getResourceCount(Resources::LAB);
    totalRes += ownerPlayer->getResourceCount(Resources::LECTURE);
    totalRes += ownerPlayer->getResourceCount(Resources::STUDY);
    totalRes += ownerPlayer->getResourceCount(Resources::TUTORIAL);
    if (totalRes <= 0) {
      continue;
    }
    // Mark this colour as eligible
    int idx = static_cast<int>(ownerColour);
    if (idx >= 0 && idx < 4) {
      eligible[idx] = true;
    }
  }
  // Build comma-separated list of eligible player colours in ascending player order
  static const std::string colourNames[4] = {"Blue", "Red", "Orange", "Yellow"};
  std::string result;
  for (int i = 0; i < 4; ++i) {
    if (!eligible[i] || static_cast<Colour>(i) == active) continue;
    if (!result.empty()) {
      result += ", ";
    }
    result += colourNames[i];
  }
  return result;
}

// Attach a vertex to this tile (called during board initialization to link vertices)
void Tile::addVertex(Vertex *v) {
  if (v != nullptr) {
    vertices.push_back(v);
  }
}
