# Ultimate Tic-Tac-Toe AI

[![CI](https://github.com/srn8r/Ultimate-Tic-Tac-Toe-AI/actions/workflows/ci.yml/badge.svg)](https://github.com/srn8r/Ultimate-Tic-Tac-Toe-AI/actions/workflows/ci.yml)
![C++20](https://img.shields.io/badge/C%2B%2B-20-blue)

A C++ engine and AI for [Ultimate Tic-Tac-Toe](https://en.wikipedia.org/wiki/Ultimate_tic-tac-toe),
built for the ESILV 3rd-year (A3) AI tournament. The AI uses **minimax with alpha-beta pruning**
and **iterative deepening** under a per-move time budget.

## The game

The board is a 3×3 grid of small tic-tac-toe boards. The cell you play in a small board
decides which small board your opponent must play in next. Win a small board to claim that
square of the big (macro) board; three claimed squares in a row win the game. If you are
sent to a board that is already finished, you may play in any open board.

## Quick start

Requires CMake 3.16+ and a C++20 compiler (GCC, Clang or MSVC). No other dependency.

```bash
cmake -S . -B build
cmake --build build
./build/uttt-play                 # you (X) against the AI (O)
./build/uttt-play --ai-first      # the AI starts
./build/uttt-play --bench 50      # AI vs random player, 50 games
ctest --test-dir build            # unit tests
```

Options for `uttt-play` are `--time MS` (time budget per AI move, default 500) and
`--depth N` (maximum search depth, default 8).

## How the AI works

1. **Legal move generation** follows the "send" rule, so the branching factor stays small
   (often 9 or fewer, up to 81 when the player is free to play anywhere).
2. **Minimax with alpha-beta pruning** explores the game tree and cuts branches that cannot
   change the result.
3. **Move ordering** tries centre, then corners, then edges first, and puts the best move of the
   previous iteration first. Good moves first means tighter bounds and more pruning.
4. **Iterative deepening** searches depth 1, 2, 3… until the time budget runs out and keeps the
   best move of the last completed depth, so the AI always answers on time.
5. **Evaluation** of non-final positions scores won small boards by their strategic value on the
   macro board (centre 4, corner 3, edge 2) plus occupied cells in open small boards.
   Terminal scores favour quick wins and slow losses.

The board only re-evaluates the small board that received a move, instead of rebuilding the
whole macro board after every move.

## Project structure

```
include/uttt/board.hpp   game rules and state
include/uttt/ai.hpp      search API
src/board.cpp            rules, legal moves, evaluation
src/ai.cpp               minimax, alpha-beta, iterative deepening
apps/play.cpp            terminal game and benchmark
arena/                   entry point for the school tournament arena
tests/tests.cpp          unit tests (run with CTest)
```

GitHub Actions builds and tests the project on Linux, Windows and macOS for every push.

## School arena build

The tournament arena links against a library provided with the course (`libUTTTLib.a` and the
Allegro 5.2 DLLs, Windows / MinGW). Those files are not redistributed here. With them in a
local folder:

```bash
cmake -S . -B build -G "MinGW Makefiles" -DUTTT_COURSE_DIR=C:/path/to/course/files
cmake --build build --target uttt-arena
```

## Ideas for later

- Monte Carlo Tree Search (MCTS) with UCT, and comparison against minimax
- Make/unmake moves on a single board instead of copying it at each node
- Bitboard representation and a transposition table
- Tune evaluation weights through self-play

## Authors

Célia, Sharusan Rasakaran ([srn8r](https://github.com/srn8r)) and Mathys
([mathysEthical](https://github.com/mathysEthical)), ESILV.
