#include "tsp_solver/algorithms/local_search.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace tsp_solver {
namespace {

[[nodiscard]] std::vector<NodeId> build_nearest_neighbor_seed(
    const Problem& problem) {
  const std::size_t n = problem.size();
  std::vector<NodeId> order;
  order.reserve(n);
  std::vector<std::uint8_t> visited(n, 0);

  NodeId current = 0;
  order.push_back(current);
  visited[current] = 1;

  for (std::size_t step = 1; step < n; ++step) {
    NodeId next = 0;
    Cost best_cost = std::numeric_limits<Cost>::max();

    for (NodeId candidate = 0; candidate < static_cast<NodeId>(n); ++candidate) {
      if (visited[candidate] != 0) {
        continue;
      }

      const Cost edge_cost = problem.distances[current][candidate];
      if (edge_cost < best_cost ||
          (edge_cost == best_cost && candidate < next)) {
        best_cost = edge_cost;
        next = candidate;
      }
    }

    order.push_back(next);
    visited[next] = 1;
    current = next;
  }

  return order;
}

[[nodiscard]] Cost route_cost(const Problem& problem,
                              const std::vector<NodeId>& order) {
  if (order.empty()) {
    return 0;
  }

  Cost total = 0;
  for (std::size_t i = 0; i < order.size(); ++i) {
    const std::size_t next = (i + 1) % order.size();
    total += problem.distances[order[i]][order[next]];
  }
  return total;
}

}  // namespace

bool Problem::is_square() const noexcept {
  const std::size_t n = distances.size();
  for (const auto& row : distances) {
    if (row.size() != n) {
      return false;
    }
  }
  return true;
}

Cost compute_tour_cost(const Problem& problem, const Tour& tour) {
  if (!problem.is_square()) {
    throw std::invalid_argument("problem distance matrix must be square");
  }
  if (tour.order.size() != problem.size()) {
    throw std::invalid_argument("tour size must match problem size");
  }
  return route_cost(problem, tour.order);
}

Tour TwoOptLocalSearch::solve(const Problem& problem) const {
  if (!problem.is_square()) {
    throw std::invalid_argument("problem distance matrix must be square");
  }

  const std::size_t n = problem.size();
  if (n == 0) {
    return {};
  }

  Tour best;
  best.order = build_nearest_neighbor_seed(problem);
  best.total_cost = route_cost(problem, best.order);

  bool improved = true;
  while (improved) {
    improved = false;
    for (std::size_t i = 1; i + 1 < n; ++i) {
      for (std::size_t k = i + 1; k < n; ++k) {
        std::vector<NodeId> candidate = best.order;
        std::reverse(candidate.begin() + static_cast<std::ptrdiff_t>(i),
                     candidate.begin() + static_cast<std::ptrdiff_t>(k) + 1);

        const Cost candidate_cost = route_cost(problem, candidate);
        if (candidate_cost < best.total_cost) {
          best.order = std::move(candidate);
          best.total_cost = candidate_cost;
          improved = true;
          break;
        }
      }
      if (improved) {
        break;
      }
    }
  }

  return best;
}

}  // namespace tsp_solver
