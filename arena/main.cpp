// Entry point for the ESILV tournament arena.
// Links against the course library (libUTTTLib.a + Allegro), which is not
// distributed in this repository. See the README, "School arena build".

#include <iostream>

#include "game_interface.hpp"
#include "uttt/ai.hpp"
#include "uttt/board.hpp"

int main() {
    using namespace uttt;

    // The arena opponent plays X and moves first (alwaysPlayFirst = false), we play O.
    game.initialize(1, Level::MEDIUM_1, Mode::ARENA, false, "NotreSuperIA");

    SearchOptions opts;
    opts.timeLimit = std::chrono::milliseconds(800);

    while (!game.isAllGameFinish()) {
        Board board;
        while (!game.isFinish()) {
            GameMove theirs{};
            if (game.getMove(theirs)) {
                std::cerr << "Opponent move " << theirs.row << ' ' << theirs.col << '\n';
                board.play({theirs.row, theirs.col}, X);
            }
            if (board.isOver()) break;

            const SearchResult r = findBestMove(board, O, opts);
            std::cerr << "Send move " << r.move.row << ' ' << r.move.col << " (depth "
                      << r.depthReached << ")\n";
            game.setMove({r.move.row, r.move.col});
            board.play(r.move, O);
        }
        std::cerr << board.toString();
    }
    return 0;
}
