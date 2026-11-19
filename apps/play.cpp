// Terminal front-end: play against the AI, or benchmark it against a random player.
//
//   uttt-play                     you (X) vs AI (O), you move first
//   uttt-play --ai-first          AI moves first
//   uttt-play --bench 50          AI vs random player over 50 games
//   uttt-play --time 1000         AI time budget per move, in ms (default 500)

#include <cstdlib>
#include <iostream>
#include <random>
#include <string>

#include "uttt/ai.hpp"
#include "uttt/board.hpp"

using namespace uttt;

namespace {

Move askHumanMove(const Board& board) {
    while (true) {
        const int forced = board.forcedSmallBoard();
        std::cout << (forced == -1 ? "Play anywhere"
                                   : "Play in small board " + std::to_string(forced))
                  << ". Enter row and column (0-8): " << std::flush;
        Move m;
        if (!(std::cin >> m.row >> m.col)) std::exit(0);
        if (board.isLegal(m)) return m;
        std::cout << "Illegal move, try again.\n";
    }
}

int playInteractive(bool aiFirst, const SearchOptions& opts) {
    Board board;
    const char human = aiFirst ? O : X;
    const char ai = opponent(human);
    char toMove = X;

    while (!board.isOver()) {
        std::cout << '\n' << board.toString();
        if (toMove == human) {
            board.play(askHumanMove(board), human);
        } else {
            const SearchResult r = findBestMove(board, ai, opts);
            std::cout << "AI plays " << r.move.row << ' ' << r.move.col << "  (depth "
                      << r.depthReached << ", " << r.nodes << " nodes)\n";
            board.play(r.move, ai);
        }
        toMove = opponent(toMove);
    }
    std::cout << '\n' << board.toString();
    const char w = board.winner();
    std::cout << (w == DRAW ? "Draw." : w == human ? "You win!" : "AI wins.") << '\n';
    return 0;
}

int benchmark(int games, const SearchOptions& opts) {
    std::mt19937 rng(42);
    int wins = 0, losses = 0, draws = 0;
    for (int g = 0; g < games; ++g) {
        Board board;
        const char ai = (g % 2 == 0) ? X : O;  // alternate who starts
        char toMove = X;
        while (!board.isOver()) {
            Move m;
            if (toMove == ai) {
                m = findBestMove(board, ai, opts).move;
            } else {
                const auto& legal = board.legalMoves();
                m = legal[std::uniform_int_distribution<size_t>(0, legal.size() - 1)(rng)];
            }
            board.play(m, toMove);
            toMove = opponent(toMove);
        }
        const char w = board.winner();
        (w == ai ? wins : w == DRAW ? draws : losses)++;
    }
    std::cout << "AI vs random over " << games << " games: " << wins << " wins, " << draws
              << " draws, " << losses << " losses\n";
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    SearchOptions opts;
    bool aiFirst = false;
    int benchGames = 0;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--ai-first") {
            aiFirst = true;
        } else if (arg == "--time" && i + 1 < argc) {
            opts.timeLimit = std::chrono::milliseconds(std::atoi(argv[++i]));
        } else if (arg == "--depth" && i + 1 < argc) {
            opts.maxDepth = std::atoi(argv[++i]);
        } else if (arg == "--bench" && i + 1 < argc) {
            benchGames = std::atoi(argv[++i]);
        } else {
            std::cerr << "Usage: uttt-play [--ai-first] [--time MS] [--depth N] [--bench GAMES]\n";
            return arg == "--help" || arg == "-h" ? 0 : 1;
        }
    }
    return benchGames > 0 ? benchmark(benchGames, opts) : playInteractive(aiFirst, opts);
}
