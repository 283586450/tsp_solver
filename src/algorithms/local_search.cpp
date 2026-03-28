#include "tsp_solver/algorithms/local_search.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

namespace tsp_solver {
namespace {

constexpr std::uint64_t kPerturbationMix = 0x9e3779b97f4a7c15ULL;
constexpr std::uint64_t kSplitMix64Shift1 = 30;
constexpr std::uint64_t kSplitMix64Shift2 = 27;
constexpr std::uint64_t kSplitMix64Shift3 = 31;
constexpr std::uint64_t kSplitMix64Mul1 = 0xbf58476d1ce4e5b9ULL;
constexpr std::uint64_t kSplitMix64Mul2 = 0x94d049bb133111ebULL;

[[nodiscard]] NodeId normalize_start_node(const Problem& problem, NodeId start_node) {
  const std::size_t node_count = problem.size();
  if (node_count == 0) {
    return 0;
  }

  return static_cast<NodeId>(static_cast<std::uint64_t>(start_node) %
                             static_cast<std::uint64_t>(node_count));
}

[[nodiscard]] std::vector<NodeId> build_nearest_neighbor_seed(const Problem& problem,
                                                              NodeId start_node) {
  const std::size_t node_count = problem.size();
  std::vector<NodeId> order;
  order.reserve(node_count);
  std::vector<std::uint8_t> visited(node_count, 0);

  NodeId current = normalize_start_node(problem, start_node);
  order.push_back(current);
  visited[current] = 1;

  for (std::size_t step = 1; step < node_count; ++step) {
    NodeId next = 0;
    Cost best_cost = std::numeric_limits<Cost>::max();

    for (NodeId candidate = 0; candidate < static_cast<NodeId>(node_count);
         ++candidate) {
      if (visited[candidate] != 0) {
        continue;
      }

      const Cost edge_cost = problem.distances[current][candidate];
      if (edge_cost < best_cost || (edge_cost == best_cost && candidate < next)) {
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

[[nodiscard]] std::uint64_t splitmix64(std::uint64_t& state) {
  state += kPerturbationMix;
  std::uint64_t value = state;
  value = (value ^ (value >> kSplitMix64Shift1)) * kSplitMix64Mul1;
  value = (value ^ (value >> kSplitMix64Shift2)) * kSplitMix64Mul2;
  return value ^ (value >> kSplitMix64Shift3);
}

[[nodiscard]] std::vector<NodeId> perturb_tour(std::vector<NodeId> order,
                                               std::uint64_t& state) {
  const std::size_t node_count = order.size();
  if (node_count <= 3) {
    return order;
  }

  const std::size_t range = node_count - 1;
  std::size_t first = 1 + static_cast<std::size_t>(splitmix64(state) % range);
  std::size_t second = 1 + static_cast<std::size_t>(splitmix64(state) % range);
  while (second == first) {
    second = 1 + static_cast<std::size_t>(splitmix64(state) % range);
  }
  if (first > second) {
    std::swap(first, second);
  }

  std::swap(order[first], order[second]);
  return order;
}

[[nodiscard]] Tour two_opt_refine(const Problem& problem, std::vector<NodeId> order) {
  Tour best;
  best.order = std::move(order);
  best.total_cost = route_cost(problem, best.order);

  const std::size_t node_count = best.order.size();
  bool improved = true;
  while (improved) {
    improved = false;
    for (std::size_t i = 1; i + 1 < node_count; ++i) {
      for (std::size_t k = i + 1; k < node_count; ++k) {
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

[[nodiscard]] std::vector<NodeId> build_cheapest_insertion_seed(const Problem& problem,
                                                                NodeId start_node) {
  const std::size_t node_count = problem.size();
  std::vector<NodeId> order;
  order.reserve(node_count);
  if (node_count == 0) {
    return order;
  }

  const NodeId first = normalize_start_node(problem, start_node);
  order.push_back(first);
  if (node_count == 1) {
    return order;
  }

  std::vector<std::uint8_t> visited(node_count, 0);
  visited[first] = 1;

  NodeId second = first;
  Cost best_cost = std::numeric_limits<Cost>::max();
  for (NodeId candidate = 0; candidate < static_cast<NodeId>(node_count); ++candidate) {
    if (visited[candidate] != 0) {
      continue;
    }

    const Cost cycle_cost =
        problem.distances[first][candidate] + problem.distances[candidate][first];
    if (cycle_cost < best_cost || (cycle_cost == best_cost && candidate < second)) {
      best_cost = cycle_cost;
      second = candidate;
    }
  }

  order.push_back(second);
  visited[second] = 1;

  while (order.size() < node_count) {
    NodeId best_node = 0;
    std::size_t best_position = 0;
    Cost best_delta = std::numeric_limits<Cost>::max();
    bool found = false;

    for (NodeId candidate = 0; candidate < static_cast<NodeId>(node_count);
         ++candidate) {
      if (visited[candidate] != 0) {
        continue;
      }

      for (std::size_t position = 0; position < order.size(); ++position) {
        const std::size_t next = (position + 1) % order.size();
        const NodeId from = order[position];
        const NodeId target_node = order[next];
        const Cost delta = problem.distances[from][candidate] +
                           problem.distances[candidate][target_node] -
                           problem.distances[from][target_node];

        if (!found || delta < best_delta ||
            (delta == best_delta && candidate < best_node) ||
            (delta == best_delta && candidate == best_node &&
             position < best_position)) {
          best_delta = delta;
          best_node = candidate;
          best_position = position + 1;
          found = true;
        }
      }
    }

    order.insert(order.begin() + static_cast<std::ptrdiff_t>(best_position), best_node);
    visited[best_node] = 1;
  }

  return order;
}

} // namespace

TwoOptLocalSearch::TwoOptLocalSearch(NodeId start_node) noexcept
    : start_node_(start_node) {}

bool Problem::is_square() const noexcept {
  const std::size_t node_count = distances.size();
  return std::ranges::all_of(
      distances, [node_count](const auto& row) { return row.size() == node_count; });
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

Tour nearest_neighbor_tour(const Problem& problem, NodeId start_node) {
  if (!problem.is_square()) {
    throw std::invalid_argument("problem distance matrix must be square");
  }

  const std::size_t node_count = problem.size();
  if (node_count == 0) {
    return {};
  }

  Tour tour;
  tour.order = build_nearest_neighbor_seed(problem, start_node);
  tour.total_cost = route_cost(problem, tour.order);
  return tour;
}

Tour cheapest_insertion_tour(const Problem& problem, NodeId start_node) {
  if (!problem.is_square()) {
    throw std::invalid_argument("problem distance matrix must be square");
  }

  const std::size_t node_count = problem.size();
  if (node_count == 0) {
    return {};
  }

  Tour tour;
  tour.order = build_cheapest_insertion_seed(problem, start_node);
  tour.total_cost = route_cost(problem, tour.order);
  return tour;
}

Tour TwoOptLocalSearch::solve(const Problem& problem) const {
  if (!problem.is_square()) {
    throw std::invalid_argument("problem distance matrix must be square");
  }

  const std::size_t node_count = problem.size();
  if (node_count == 0) {
    return {};
  }

  return two_opt_refine(problem, build_nearest_neighbor_seed(problem, start_node_));
}

Tour iterated_local_search_tour(const Problem& problem, NodeId start_node,
                                std::uint64_t random_seed) {
  if (!problem.is_square()) {
    throw std::invalid_argument("problem distance matrix must be square");
  }

  const std::size_t node_count = problem.size();
  if (node_count == 0) {
    return {};
  }

  Tour best = two_opt_refine(problem, build_nearest_neighbor_seed(problem, start_node));
  std::uint64_t state =
      random_seed ^
      (static_cast<std::uint64_t>(normalize_start_node(problem, start_node)) << 1) ^
      kPerturbationMix;
  const std::size_t iterations = std::max<std::size_t>(1, node_count);

  for (std::size_t iteration = 0; iteration < iterations; ++iteration) {
    std::vector<NodeId> candidate_order = perturb_tour(best.order, state);
    Tour candidate = two_opt_refine(problem, std::move(candidate_order));
    if (candidate.total_cost < best.total_cost) {
      best = std::move(candidate);
    }
  }

  return best;
}

} // namespace tsp_solver
