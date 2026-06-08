# Voidbound Arena

Voidbound Arena is a turn-based Lua arena game prototype.

The player fights through waves on a procedurally generated grid using action points, abilities, hazards and positioning. The code is split into small modules so the game loop, map, AI, abilities and entities can be changed independently.

## Features

- Deterministic map generation from a seed.
- Turn-based action-point combat.
- Player abilities: strike, dash, blast and shield.
- Enemy AI with pathing toward the player.
- Hazards, walls, healing cells and exit portal.
- Wave scaling.
- Small test runner with gameplay assertions.

## Run Visual Version

Install LOVE2D, then run this folder:

```bash
love .
```

Controls:

- `WASD`: move
- Arrow keys: strike
- `Shift + WASD`: dash
- `Space`: blast
- `E`: shield
- `Enter`: wait
- `R`: restart
- `Esc`: quit

## Run Terminal Version

You need Lua 5.4 or LuaJIT.

```bash
lua src/main.lua
```

Custom seed:

```bash
lua src/main.lua 12345
```

## Test

```bash
lua tests/run_tests.lua
```

## Code Map

- `src/main.lua`: command-line game loop.
- `src/game.lua`: turn resolution and game state.
- `src/map.lua`: procedural arena map.
- `src/entities.lua`: player/enemy creation and stats.
- `src/abilities.lua`: combat actions.
- `src/ai.lua`: enemy behavior.
- `tests/run_tests.lua`: simple no-dependency test runner.
