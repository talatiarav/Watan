# "Would You Like to Play Again?" - Implementation Summary

## ✅ IMPLEMENTATION COMPLETE

### What Was Implemented

The game now properly implements the **End of Game** functionality as specified in section 4.4:

1. **Win Detection** ✅
   - Game detects when a student reaches 10 completed course criteria
   - Prints: "Student [Color] wins the game!"

2. **Play Again Prompt** ✅ (NEW)
   - After winner is announced, prompts: "Would you like to play again?"
   - Waits for user input

3. **Replay Functionality** ✅ (NEW)
   - If response is "yes": Game restarts from the beginning with a fresh board
   - If response is "no": Game exits

### Files Modified

1. **gamecontroller.h** - Changed `run()` signature to return `bool`
2. **gamecontroller.cc** - Added replay prompt logic
3. **gamecontrollerv2.h** - Changed `run()` signature to return `bool`
4. **gamecontrollerv2.cc** - Added replay prompt logic
5. **main.cc** - Added game loop to support multiple plays

### Code Locations

**Replay Prompt:**
- `gamecontroller.cc` line 157
- `gamecontrollerv2.cc` line 169

**Response Handling:**
- `gamecontroller.cc` lines 158-177
- `gamecontrollerv2.cc` lines 170-189

**Game Loop:**
- `main.cc` lines 89-120

### Example Flow

```
[Game plays...]
Student Blue wins the game!
Would you like to play again?
> yes
[New game starts with fresh board]
[Game plays...]
Student Red wins the game!
Would you like to play again?
> no
[Program exits]
```

### Technical Details

- Response handling is case-insensitive ("yes", "Yes", "YES" all work)
- Whitespace is automatically trimmed
- Only exact "yes" triggers replay; anything else exits
- Each replay gets a fresh random board (unless -board option used)
- Seed remains consistent across replays if -seed was specified
- Load files are only used for the first game, not replays

### Testing

Compile and run:
```bash
make
./watan -seed 42
```

Then play until someone wins, and you'll see the prompt!

### Specification Compliance

✅ Game detects 10 completed course criteria
✅ Announces winner correctly
✅ Prompts "Would you like to play again?"
✅ Accepts "yes" to restart
✅ Accepts "no" to exit
✅ Starts fresh game from beginning
