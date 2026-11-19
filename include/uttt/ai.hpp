#pragma once

#include <chrono>

#include "uttt/board.hpp"

namespace uttt {

struct SearchOptions {
    int maxDepth = 8;                          // hard depth cap
    std::chrono::milliseconds timeLimit{500};  // budget per move
};

struct SearchResult {
    Move move;
    int score = 0;
    int depthReached = 0;
    long long nodes = 0;
};

// Minimax with alpha-beta pruning and iterative deepening.
// Searches depth 1, 2, 3... until the time budget or maxDepth is reached and
// returns the best move of the last fully completed depth.
SearchResult findBestMove(const Board& board, char player, const SearchOptions& options = {});

}  // namespace uttt
