#include "uttt/ai.hpp"

#include <algorithm>
#include <limits>

namespace uttt {

namespace {

using Clock = std::chrono::steady_clock;

constexpr int kInf = std::numeric_limits<int>::max() / 2;
constexpr int kWinScore = 100000;

// Move ordering: centre > corners > edges of the small board. Trying likely good
// moves first tightens alpha/beta sooner, so more branches get pruned.
constexpr int kLocalPriority[9] = {2, 1, 2, 1, 4, 1, 2, 1, 2};

struct TimeUp {};

class Searcher {
   public:
    Searcher(char me, Clock::time_point deadline) : me_(me), deadline_(deadline) {}

    long long nodes = 0;

    std::vector<Move> ordered(const Board& board, Move first = {}) const {
        std::vector<Move> moves = board.legalMoves();
        std::stable_sort(moves.begin(), moves.end(), [&](Move a, Move b) {
            if (a == first) return b != first;
            if (b == first) return false;
            return kLocalPriority[localIndexOf(a.row, a.col)] >
                   kLocalPriority[localIndexOf(b.row, b.col)];
        });
        return moves;
    }

    // Score from `me_`'s point of view. `ply` = distance from the root.
    int minimax(const Board& board, int depth, int ply, int alpha, int beta, bool maximizing) {
        if ((++nodes & 1023) == 0 && Clock::now() >= deadline_) throw TimeUp{};

        const char w = board.winner();
        // Prefer quick wins and slow losses.
        if (w == me_) return kWinScore - ply;
        if (w == opponent(me_)) return -kWinScore + ply;
        if (w == DRAW) return 0;
        if (depth == 0) return board.evaluate(me_);

        const char toMove = maximizing ? me_ : opponent(me_);
        int best = maximizing ? -kInf : kInf;
        for (const Move& m : ordered(board)) {
            Board child = board;
            child.play(m, toMove);
            const int v = minimax(child, depth - 1, ply + 1, alpha, beta, !maximizing);
            if (maximizing) {
                best = std::max(best, v);
                alpha = std::max(alpha, v);
            } else {
                best = std::min(best, v);
                beta = std::min(beta, v);
            }
            if (beta <= alpha) break;
        }
        return best;
    }

   private:
    char me_;
    Clock::time_point deadline_;
};

}  // namespace

SearchResult findBestMove(const Board& board, char player, const SearchOptions& options) {
    SearchResult result;
    const auto& legal = board.legalMoves();
    if (legal.empty()) return result;
    result.move = legal.front();  // always return a legal move, even if time runs out at once
    if (legal.size() == 1) return result;

    Searcher searcher(player, Clock::now() + options.timeLimit);
    for (int depth = 1; depth <= options.maxDepth; ++depth) {
        try {
            Move bestMove = legal.front();
            int bestScore = -kInf;
            int alpha = -kInf;
            // Start with the previous iteration's best move for better pruning.
            for (const Move& m : searcher.ordered(board, result.move)) {
                Board child = board;
                child.play(m, player);
                const int v = searcher.minimax(child, depth - 1, 1, alpha, kInf, false);
                if (v > bestScore) {
                    bestScore = v;
                    bestMove = m;
                }
                alpha = std::max(alpha, v);
            }
            result.move = bestMove;
            result.score = bestScore;
            result.depthReached = depth;
            if (bestScore >= kWinScore - depth) break;  // forced win found, no need to go deeper
        } catch (const TimeUp&) {
            break;  // keep the result of the last completed depth
        }
    }
    result.nodes = searcher.nodes;
    return result;
}

}  // namespace uttt
