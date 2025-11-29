# Watan Game - UML Class Diagram

## Class Diagram (PlantUML Format)

```plantuml
@startuml Watan_Game_Architecture

' ========== MAIN CONTROLLER ==========
class GameController {
  - board: GameBoard
  - currentPlayer: Colour
  - rolledThisTurn: bool
  - awaitingGeesePlacement: bool
  - quitRequested: bool
  __
  + GameController(board: GameBoard&&, startingPlayer: Colour)
  + run(in: istream&, out: ostream&): bool
  - setupInitialAssignments(in, out)
  - handleCommand(line, in, out)
  - cmdRoll(), cmdComplete(), cmdImprove()
  - cmdAchieve(), cmdSave(), cmdTrade()
  - checkForWinner(out)
}

' ========== GAME BOARD ==========
class GameBoard {
  - players: vector<unique_ptr<Player>>
  - tiles: vector<unique_ptr<Tile>>
  - vertices: vector<unique_ptr<Vertex>>
  - edges: vector<unique_ptr<Edge>>
  - view: unique_ptr<BoardView>
  - geeseTile: int
  __
  + GameBoard(enhance, values, resources)
  + {static} createRandom(enhance): GameBoard
  + placeInitialAssignment(playerColour, vertexId)
  + rollDice(activePlayer): int
  + completeVertex(playerColour, vertexId)
  + improveVertex(playerColour, vertexId)
  + achieveEdge(playerColour, edgeId)
  + moveGeese(activePlayer, tileId)
  + hasWinner(): bool
  + getWinner(): Colour
  + printBoard(out), printStatus(out)
}

' ========== PLAYER ==========
class Player {
  - colour: Colour
  - resources: map<Resources, int>
  - ownedVertices: vector<Vertex*>
  - ownedEdges: vector<Edge*>
  - dice: unique_ptr<Dice>
  __
  + Player(colour: Colour)
  + getColour(): Colour
  + rollDice(): int
  + useFairDice(), useLoadedDice()
  + addResources(resource, amount)
  + numResources(): int
  + getPoints(): int
  + resourcesCheck(type): bool
  + resourcesSpent(type)
  + loseResourcesToGeese(out)
  + addVertex(v), addEdge(e)
  + printStatus(out), printCriteria(out)
  + encodeForSave(): string
}

' ========== BOARD ELEMENTS ==========
class Vertex {
  - id: int
  - owner: Player*
  - assessment: Assessment
  - neighbours: vector<Vertex*>
  - incidentEdges: vector<Edge*>
  - adjacentTiles: vector<Tile*>
  - observer: BoardView*
  __
  + Vertex(id: int)
  + getId(): int
  + getOwner(): Player*
  + currentAssessment(): Assessment
  + canBeCompletedBy(colour): bool
  + canBeImprovedBy(colour): bool
  + complete(p: Player*)
  + improve()
  + addNeighbour(v), addIncidentEdge(e)
  + addAdjacentTile(t)
}

class Edge {
  - id: int
  - owner: Player*
  - endpoints: Vertex*[2]
  - neighbours: vector<Edge*>
  - observer: BoardView*
  __
  + Edge(id: int)
  + getId(): int
  + getOwner(): Player*
  + canBeAchievedBy(colour): bool
  + achieve(p: Player*)
  + setEndpoints(v1, v2)
  + addNeighbour(e)
}

class Tile {
  - id: int
  - value: int
  - resource: Resources
  - geeseHere: bool
  - vertices: vector<Vertex*>
  __
  + Tile(id, resource, value)
  + getId(), getValue(), getResource(): int
  + hasGeese(): bool
  + toggleGeese()
  + sendResources(): bool
  + playersToStealFrom(active): string
  + addVertex(v)
}

' ========== OBSERVER PATTERN ==========
abstract class Subject {
  # observers: vector<Observer*>
  __
  # notifyObservers()
  + attach(observer: Observer*)
  + detach(observer: Observer*)
}

interface Observer {
  + {abstract} notify(vertex: Vertex*)
  + {abstract} notify(edge: Edge*)
  + {abstract} notify(tile: Tile*)
}

' ========== VIEW ==========
class BoardView {
  - geeseAt: int
  - enhance: bool
  - criteriaString: vector<string>
  - goalsString: vector<string>
  - resourcesString: vector<string>
  - valuesString: vector<string>
  __
  + BoardView(enhance, values, resources)
  + notify(vertex: Vertex*)
  + notify(edge: Edge*)
  + notifyGeese(tileId: int)
  + render(out: ostream&)
}

' ========== DICE HIERARCHY ==========
abstract class Dice {
  + {abstract} ~Dice()
  + {abstract} setDie(num: int)
  + {abstract} roll(): int
}

class Fair {
  - dice1: int
  - dice2: int
  __
  + Fair()
  + setDie(num)
  + roll(): int
}

class Loaded {
  - value: int
  __
  + Loaded()
  + setDie(num)
  + roll(): int
}

' ========== SAVE/LOAD ==========
class SaveManager <<static>> {
  + {static} saveGame(board, currentPlayer, filename)
  + {static} loadGame(enhance, filename, currentPlayerOut): GameBoard
}

' ========== ENUMS ==========
enum Colour {
  Blue
  Red
  Orange
  Yellow
  Bank
  None
}

enum Resources {
  CAFFEINE
  LAB
  LECTURE
  STUDY
  TUTORIAL
  NETFLIX
}

enum Assessment {
  None
  Assignment
  Midterm
  Exam
}

' ========== RELATIONSHIPS ==========

' Controller owns GameBoard
GameController *-- GameBoard

' GameBoard owns Players, Tiles, Vertices, Edges, BoardView
GameBoard *-- "4" Player
GameBoard *-- "19" Tile
GameBoard *-- "54" Vertex
GameBoard *-- "72" Edge
GameBoard *-- BoardView

' Player owns Dice, references Vertices and Edges
Player *-- Dice
Player o-- "many" Vertex : owns >
Player o-- "many" Edge : owns >

' Vertex relationships
Vertex --> Player : owner
Vertex o-- "many" Vertex : neighbours
Vertex o-- "many" Edge : incident
Vertex o-- "many" Tile : adjacent
Vertex --> BoardView : observer

' Edge relationships
Edge --> Player : owner
Edge o-- "2" Vertex : endpoints
Edge o-- "many" Edge : neighbours
Edge --> BoardView : observer

' Tile relationships
Tile --> "many" Vertex
Tile --|> Subject

' Observer pattern
BoardView ..|> Observer
Subject o-- Observer
Tile --|> Subject

' Dice hierarchy
Fair --|> Dice
Loaded --|> Dice

' SaveManager uses GameBoard
SaveManager ..> GameBoard : uses

' Enums used by classes
Player --> Colour
Player --> Resources
Vertex --> Assessment
Tile --> Resources
GameBoard --> Colour

@enduml
```

## Architecture Overview

### **Core Game Flow**
1. **GameController** orchestrates the game loop
2. **GameBoard** manages game state and enforces rules
3. **Player** tracks resources, owned properties, and dice
4. **Vertex, Edge, Tile** represent the board graph

### **Design Patterns Used**

#### 1. **Observer Pattern**
- `Tile` inherits from `Subject`
- `BoardView` implements `Observer`
- Changes to tiles notify the view for real-time updates

#### 2. **Strategy Pattern**
- `Dice` interface with `Fair` and `Loaded` implementations
- Players can switch dice types at runtime

#### 3. **Factory Pattern**
- `GameBoard::createRandom()` creates randomized boards
- `GameBoard(values, resources)` for custom boards

#### 4. **Singleton-like Pattern**
- `SaveManager` provides static save/load methods

### **Memory Management**
- ✅ **Smart Pointers**: `unique_ptr` for ownership (Players, Tiles, Vertices, Edges, Dice, BoardView)
- ✅ **Raw Pointers**: Only for non-ownership references (adjacency, observers)
- ✅ **No Manual Memory**: Zero `new`/`delete` statements
- ✅ **RAII Compliance**: Full resource management via STL containers

### **Key Relationships**

| Relationship | Type | Description |
|--------------|------|-------------|
| GameBoard → Player | Composition | Board owns 4 players |
| GameBoard → Tile/Vertex/Edge | Composition | Board owns all graph elements |
| Player → Dice | Composition | Player owns their dice |
| Player → Vertex/Edge | Association | Player references owned properties |
| Tile → Subject | Inheritance | Tile is observable |
| BoardView → Observer | Implementation | View observes tile changes |
| Fair/Loaded → Dice | Inheritance | Dice implementations |

### **Bonus Features Implemented**
1. **Play Again** (GameController::run returns bool)
2. **Memory Management** (4 marks) - All smart pointers
3. **Color Enhancement** (2 marks) - BoardView enhance mode

---

## To Visualize
Copy the PlantUML code to:
- https://www.plantuml.com/plantuml/uml/
- VS Code with PlantUML extension
- Any PlantUML renderer

