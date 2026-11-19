#include "uttt/board.hpp"

#include <sstream>

namespace uttt {

namespace {

// Weight of a square by the number of winning lines it belongs to:
// centre = 4, corners = 3, edges = 2.
constexpr int kSquareWeight[3][3] = {{3, 2, 3}, {2, 4, 2}, {3, 2, 3}};

constexpr int kWinScore = 100000;

}  // namespace

bool hasLine(const Grid3& g, char p) {
    for (int i = 0; i < 3; ++i) {
        if (g[i][0] == p && g[i][1] == p && g[i][2] == p) return true;
        if (g[0][i] == p && g[1][i] == p && g[2][i] == p) return true;
    }
    return (g[0][0] == p && g[1][1] == p && g[2][2] == p) ||
           (g[0][2] == p && g[1][1] == p && g[2][0] == p);
}

Board::Board() {
    for (auto& row : cells_) row.fill(EMPTY);
    for (auto& row : macro_) row.fill(EMPTY);
    computeLegalMoves();
}

int Board::forcedSmallBoard() const {
    if (!lastMove_.isValid()) return -1;  // first move of the game
    const int target = localIndexOf(lastMove_.row, lastMove_.col);
    return macro(target) == EMPTY ? target : -1;  // finished target board: play anywhere
}

bool Board::isLegal(Move move) const {
    for (const Move& m : legalMoves_)
        if (m == move) return true;
    return false;
}

void Board::play(Move move, char player) {
    cells_[move.row][move.col] = player;
    lastMove_ = move;
    // Only the small board that received the move can change state.
    updateSmallBoard(smallBoardIndexOf(move.row, move.col));
    computeLegalMoves();
}

void Board::updateSmallBoard(int index) {
    const int r0 = (index / 3) * 3;
    const int c0 = (index % 3) * 3;
    Grid3 small{};
    bool full = true;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) {
            small[i][j] = cells_[r0 + i][c0 + j];
            if (small[i][j] == EMPTY) full = false;
        }

    char state = EMPTY;
    if (hasLine(small, X))
        state = X;
    else if (hasLine(small, O))
        state = O;
    else if (full)
        state = DRAW;
    macro_[index / 3][index % 3] = state;
}

void Board::computeLegalMoves() {
    legalMoves_.clear();
    if (hasLine(macro_, X) || hasLine(macro_, O)) return;  // game already won

    const int forced = forcedSmallBoard();
    for (int i = 0; i < 9; ++i)
        for (int j = 0; j < 9; ++j) {
            if (cells_[i][j] != EMPTY) continue;
            const int sb = smallBoardIndexOf(i, j);
            if (macro(sb) != EMPTY) continue;  // cannot play in a finished small board
            if (forced != -1 && sb != forced) continue;
            legalMoves_.push_back({i, j});
        }
}

char Board::winner() const {
    if (hasLine(macro_, X)) return X;
    if (hasLine(macro_, O)) return O;
    return legalMoves_.empty() ? DRAW : EMPTY;
}

int Board::evaluate(char player) const {
    const char other = opponent(player);
    if (hasLine(macro_, player)) return kWinScore;
    if (hasLine(macro_, other)) return -kWinScore;

    int score = 0;
    // (1) Won small boards, weighted by their strategic value on the macro board.
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) {
            if (macro_[i][j] == player)
                score += 10 * kSquareWeight[i][j];
            else if (macro_[i][j] == other)
                score -= 10 * kSquareWeight[i][j];
        }
    // (2) Occupied cells in open small boards, weighted by their local position.
    for (int i = 0; i < 9; ++i)
        for (int j = 0; j < 9; ++j) {
            if (macro(smallBoardIndexOf(i, j)) != EMPTY) continue;
            const int w = kSquareWeight[i % 3][j % 3] - 1;  // centre 3, corner 2, edge 1
            if (cells_[i][j] == player)
                score += w;
            else if (cells_[i][j] == other)
                score -= w;
        }
    return score;
}

std::string Board::toString() const {
    std::ostringstream out;
    out << "    0 1 2   3 4 5   6 7 8\n";
    for (int i = 0; i < 9; ++i) {
        if (i % 3 == 0) out << "  +-------+-------+-------+\n";
        out << i << " ";
        for (int j = 0; j < 9; ++j) {
            if (j % 3 == 0) out << "| ";
            out << cells_[i][j] << ' ';
        }
        out << "|\n";
    }
    out << "  +-------+-------+-------+\n";
    return out.str();
}

}  // namespace uttt
