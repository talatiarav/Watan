# Play Again Feature Implementation

## Overview
Implemented the "Would you like to play again?" functionality as per specification section 4.4.

## Changes Made

### 1. `gamecontroller.h` and `gamecontrollerv2.h`
- Changed `run()` return type from `void` to `bool`
- Returns `true` if player wants to play again, `false` otherwise

### 2. `gamecontroller.cc` and `gamecontrollerv2.cc`
- After detecting a winner, the game now prompts: "Would you like to play again?"
- Reads user response
- If response is "yes" (case-insensitive), returns `true` to restart
- If response is "no" or anything else, returns `false` to exit
- Game also returns `false` if quit command was used or EOF reached

### 3. `main.cc`
- Wrapped game initialization and execution in a `while (playAgain)` loop
- Creates a fresh `GameBoard` and `GameController` for each replay
- After first game, clears the load file so replays start fresh (not from save)
- Loop continues until player responds "no" to replay prompt

## Behavior

### When a player reaches 10 course criteria:
1. Game prints: "Student [Color] wins the game!"
2. Game prompts: "Would you like to play again?"
3. If user types "yes": Game restarts with a new random board
4. If user types "no" or anything else: Game exits

### When game ends by quit command:
- Game prints: "Game ended by user."
- Game exits without prompting for replay

## Testing

To test the feature:
```bash
# Create a test input that completes a game and says "yes" to replay
watan.exe -seed 42 < test_replay.txt

# Or manually play and test the prompt
watan.exe
```

## Input Handling
- Response is trimmed of whitespace
- Response is converted to lowercase
- Only "yes" triggers a replay
- Any other response (including "no", empty string, EOF) exits

## Compatibility
- Works with all command-line options: `-seed`, `-board`, `-load`
- After first game, subsequent replays ignore the `-load` option (start fresh)
- Seed remains the same across replays if specified
