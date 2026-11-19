// Minimal self-contained test runner (no external dependency), run through CTest.

#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "uttt/ai.hpp"
#include "uttt/board.hpp"

using namespace uttt;

namespace {

int failures = 0;
std::vector<std::pair<std::string, std::function<void()>>>& registry() {
    static std::vector<std::pair<std::string, std::function<void()>>> r;
    return r;
}
struct Register {
    Register(const char* name, std::function<void()> fn) { registry().emplace_back(name, fn); }
};

#define TEST(name)                           \
    void name();                             \
    static Register reg_##name(#name, name); \
    void name()

#define CHECK(cond)                                                                     \
    do {                                                                                \
        if (!(cond)) {                                                                  \
            std::cerr << "  FAILED " << __FILE__ << ':' << __LINE__ << "  " #cond "\n"; \
            ++failures;                                                                 \
        }                                                                               \
    } while (0)

// Plays a list of moves alternating X, O, X...
Board playAll(std::initializer_list<Move> moves) {
    Board b;
    char p = X;
    for (Move m : moves) {
        b.play(m, p);
        p = opponent(p);
    }
    return b;
}

}  // namespace

TEST(empty_board_allows_all_81_cells) {
    Board b;
    CHECK(b.legalMoves().size() == 81);
    CHECK(b.forcedSmallBoard() == -1);
    CHECK(b.winner() == EMPTY);
}

TEST(index_helpers) {
    CHECK(smallBoardIndexOf(4, 7) == 5);
    CHECK(localIndexOf(4, 7) == 4);
    CHECK(smallBoardIndexOf(8, 8) == 8 && localIndexOf(8, 8) == 8);
}

TEST(move_sends_opponent_to_matching_small_board) {
    Board b = playAll({{4, 7}});  // local position 4 = centre
    CHECK(b.forcedSmallBoard() == 4);
    CHECK(b.legalMoves().size() == 9);
    for (Move m : b.legalMoves()) CHECK(smallBoardIndexOf(m.row, m.col) == 4);
}

TEST(has_line_detects_rows_columns_diagonals) {
    Grid3 g{{{X, X, X}, {EMPTY, EMPTY, EMPTY}, {EMPTY, EMPTY, EMPTY}}};
    CHECK(hasLine(g, X) && !hasLine(g, O));
    Grid3 col{{{O, EMPTY, EMPTY}, {O, EMPTY, EMPTY}, {O, EMPTY, EMPTY}}};
    CHECK(hasLine(col, O));
    Grid3 diag{{{EMPTY, EMPTY, X}, {EMPTY, X, EMPTY}, {X, EMPTY, EMPTY}}};
    CHECK(hasLine(diag, X));
}

TEST(winning_a_small_board_updates_macro_and_frees_next_player) {
    Board b;
    // X takes the top row of small board 0; O's replies stay in other boards.
    b.play({0, 0}, X);
    b.play({0, 3}, O);
    b.play({0, 1}, X);
    b.play({0, 4}, O);
    b.play({0, 2}, X);
    CHECK(b.macro(0) == X);
    // X's last move (0,2) has local index 2 -> O must play in small board 2.
    CHECK(b.forcedSmallBoard() == 2);
    // Sending a player to a finished board lets them play anywhere still open.
    Board c = b;
    c.play({0, 6}, O);  // local index 0 -> board 0, which is won
    CHECK(c.forcedSmallBoard() == -1);
    for (Move m : c.legalMoves()) CHECK(smallBoardIndexOf(m.row, m.col) != 0);
}

TEST(full_small_board_without_line_is_a_draw) {
    Board b;
    // X O X / X O O / O X X in small board 0, no line.
    const char layout[3][3] = {{X, O, X}, {X, O, O}, {O, X, X}};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) b.play({i, j}, layout[i][j]);
    CHECK(b.macro(0) == DRAW);
}

TEST(macro_line_wins_the_game) {
    Board b;
    // Give X small boards 0, 1 and 2 (top row of the macro board).
    for (int sb = 0; sb < 3; ++sb)
        for (int k = 0; k < 3; ++k) b.play({0, sb * 3 + k}, X);
    CHECK(b.winner() == X);
    CHECK(b.isOver());
    CHECK(b.legalMoves().empty());
}

TEST(ai_returns_a_legal_move) {
    Board b = playAll({{4, 4}, {3, 3}, {0, 0}});
    SearchOptions o;
    o.timeLimit = std::chrono::milliseconds(100);
    const SearchResult r = findBestMove(b, O, o);
    CHECK(b.isLegal(r.move));
    CHECK(r.depthReached >= 1);
}

TEST(ai_takes_an_immediate_game_win) {
    Board b;
    // X owns small boards 0 and 1, and has two in a row in board 2 (cells (0,6) and (0,7)).
    for (int sb = 0; sb < 2; ++sb)
        for (int k = 0; k < 3; ++k) b.play({0, sb * 3 + k}, X);
    b.play({0, 6}, X);
    b.play({0, 7}, X);
    b.play({6, 2}, O);  // local index 2 -> X must play in small board 2
    CHECK(b.forcedSmallBoard() == 2);
    SearchOptions o;
    o.timeLimit = std::chrono::milliseconds(200);
    const SearchResult r = findBestMove(b, X, o);
    CHECK((r.move == Move{0, 8}));
}

TEST(ai_beats_random_player) {
    // Smoke test: a shallow AI should never lose to a fixed sequence of first-legal moves.
    SearchOptions o;
    o.maxDepth = 3;
    o.timeLimit = std::chrono::milliseconds(50);
    for (char ai : {X, O}) {
        Board b;
        char toMove = X;
        while (!b.isOver()) {
            Move m = toMove == ai ? findBestMove(b, ai, o).move : b.legalMoves().front();
            b.play(m, toMove);
            toMove = opponent(toMove);
        }
        CHECK(b.winner() == ai);
    }
}

int main() {
    for (auto& [name, fn] : registry()) {
        const int before = failures;
        fn();
        std::cout << (failures == before ? "[ OK ] " : "[FAIL] ") << name << '\n';
    }
    std::cout << registry().size() << " tests, " << failures << " failed checks\n";
    return failures == 0 ? 0 : 1;
}
