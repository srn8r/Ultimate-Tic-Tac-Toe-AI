#pragma once

#include <array>
#include <string>
#include <vector>

namespace uttt {

// Cell / small-board states.
constexpr char EMPTY = '.';
constexpr char X = 'X';
constexpr char O = 'O';
constexpr char DRAW = 'D';

constexpr char opponent(char player) { return player == X ? O : X; }

struct Move {
    int row = -1;  // 0..8 on the 9x9 board
    int col = -1;  // 0..8 on the 9x9 board

    bool isValid() const { return row >= 0 && row < 9 && col >= 0 && col < 9; }
    bool operator==(const Move& other) const = default;
};

// A classic 3x3 tic-tac-toe grid, used both for small boards and for the macro board.
using Grid3 = std::array<std::array<char, 3>, 3>;

// True if `player` has three in a row (row, column or diagonal) on a 3x3 grid.
bool hasLine(const Grid3& grid, char player);

// Index (0-8, row-major) of the small board containing the cell (row, col).
constexpr int smallBoardIndexOf(int row, int col) { return (row / 3) * 3 + (col / 3); }

// Position (0-8, row-major) of the cell (row, col) inside its own small board.
// In Ultimate Tic-Tac-Toe this is the small board the opponent is sent to.
constexpr int localIndexOf(int row, int col) { return (row % 3) * 3 + (col % 3); }

// Full Ultimate Tic-Tac-Toe state: a 9x9 board made of nine 3x3 small boards,
// plus the macro board that records which small boards are won ('X', 'O') or drawn ('D').
class Board {
   public:
    Board();

    char cell(int row, int col) const { return cells_[row][col]; }
    char macro(int smallBoardIndex) const {
        return macro_[smallBoardIndex / 3][smallBoardIndex % 3];
    }
    const Grid3& macroBoard() const { return macro_; }
    Move lastMove() const { return lastMove_; }

    // Small board the next player must play in, or -1 if they may play anywhere.
    int forcedSmallBoard() const;

    // Moves allowed by the rules in the current position.
    const std::vector<Move>& legalMoves() const { return legalMoves_; }
    bool isLegal(Move move) const;

    // Plays `move` for `player`. The caller is responsible for legality (see isLegal).
    void play(Move move, char player);

    // 'X' or 'O' if someone won the macro board, 'D' for a draw, EMPTY while in progress.
    char winner() const;
    bool isOver() const { return winner() != EMPTY; }

    // Static evaluation from `player`'s point of view (> 0 is good for `player`).
    int evaluate(char player) const;

    std::string toString() const;

   private:
    void updateSmallBoard(int smallBoardIndex);
    void computeLegalMoves();

    std::array<std::array<char, 9>, 9> cells_{};
    Grid3 macro_{};
    Move lastMove_{};
    std::vector<Move> legalMoves_;
};

}  // namespace uttt
