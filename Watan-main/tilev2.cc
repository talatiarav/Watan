module Tile;

import <algorithm>;
import <sstream>;

import Tile;
import Vertex;
import Player;
import Assessment;
import Resources;
import Colour;

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
  // Notify observers that the tile state changed
  notifyObservers();
}

// Distribute this tile's resource to adjacent vertices' owners if conditions allow
// Returns true if at least one resource was sent to a player, false if none were distributed.
bool Tile::sendResources() {
  // If geese block this tile or if the tile produces no resources (e.g., Netflix), no distribution
  if (geeseHere || resource == Resources::Netflix) {
    return false;
  }
  bool distributed = false;
  // Give resources to each adjacent vertex owner based on their improvement level
  for (Vertex *v : vertices) {
    if (v == nullptr) continue;
    
    // Check if vertex has an owner
    Player *owner = v->getOwner();
    if (owner == nullptr) {
      continue;
    }
    
    // Determine the number of resources to give based on the assessment level
    Assessment level = v->currentAssessment();
    int reward = 0;
    switch (level) {
      case Assessment::Assignment: reward = 1; break;
      case Assessment::Midterm:    reward = 2; break;
      case Assessment::Exam:       reward = 3; break;
      default:                     reward = 0; break;
    }
    
    if (reward > 0) {
      owner->addResources(resource, reward);
      distributed = true;
    }
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
    
    Player *owner = v->getOwner();
    if (owner == nullptr) {
      continue;
    }
    
    Colour ownerColour = owner->getColour();
    
    // Skip if the owner is the active player
    if (ownerColour == active) {
      continue;
    }
    
    // Check that the owner has at least one resource card
    int totalRes = 0;
    totalRes += owner->getResourceCount(Resources::Caffeine);
    totalRes += owner->getResourceCount(Resources::Lab);
    totalRes += owner->getResourceCount(Resources::Lecture);
    totalRes += owner->getResourceCount(Resources::Study);
    totalRes += owner->getResourceCount(Resources::Tutorial);
    
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
    if (!eligible[i]) continue;
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
