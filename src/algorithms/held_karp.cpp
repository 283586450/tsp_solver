#include "tsp_solver/algorithms/held_karp.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace tsp_solver {
namespace {

constexpr std::size_t kHeldKarpCutoff = 18;

[[nodiscard]] NodeId normalize_start_node(const Problem& problem, NodeId start_node) {
  const std::size_t node_count = problem.size();
  if (node_count == 0) {
    return 0;
  }

  return static_cast<NodeId>(static_cast<std::uint64_t>(start_node) %
                             static_cast<std::uint64_t>(node_count));
}

[[nodiscard]] Cost edge_cost(const Problem& problem, NodeId from, NodeId to) {
  return problem.distances[from][to];
}

} // namespace

Tour held_karp_tour(const Problem& problem, NodeId start_node) {
  if (!problem.is_square()) {
    throw std::invalid_argument("problem distance matrix must be square");
  }

  const std::size_t node_count = problem.size();
  if (node_count == 0) {
    return {};
  }
  if (node_count > kHeldKarpCutoff) {
    throw std::out_of_range("problem size exceeds held-karp cutoff of 18 nodes");
  }

  const NodeId start = normalize_start_node(problem, start_node);
  if (node_count == 1) {
    Tour tour;
    tour.order = {start};
    tour.total_cost = edge_cost(problem, start, start);
    return tour;
  }

  std::vector<NodeId> nodes;
  nodes.reserve(node_count - 1);
  for (NodeId node = 0; node < static_cast<NodeId>(node_count); ++node) {
    if (node != start) {
      nodes.push_back(node);
    }
  }

  const std::size_t subset_count = std::size_t{1} << nodes.size();
  constexpr Cost kInf = std::numeric_limits<Cost>::max() / 4;

  std::vector<Cost> dp(subset_count * nodes.size(), kInf);
  std::vector<NodeId> parent(subset_count * nodes.size(),
                             std::numeric_limits<NodeId>::max());

  auto index = [node_count = nodes.size()](std::size_t mask, std::size_t last) {
    return mask * node_count + last;
  };

  for (std::size_t last = 0; last < nodes.size(); ++last) {
    const std::size_t mask = std::size_t{1} << last;
    dp[index(mask, last)] = edge_cost(problem, start, nodes[last]);
  }

  for (std::size_t mask = 1; mask < subset_count; ++mask) {
    for (std::size_t last = 0; last < nodes.size(); ++last) {
      const std::size_t last_bit = std::size_t{1} << last;
      if ((mask & last_bit) == 0) {
        continue;
      }

      const std::size_t previous_mask = mask ^ last_bit;
      if (previous_mask == 0) {
        continue;
      }

      Cost best_cost = dp[index(mask, last)];
      NodeId best_parent = parent[index(mask, last)];

      for (std::size_t previous = 0; previous < nodes.size(); ++previous) {
        const std::size_t previous_bit = std::size_t{1} << previous;
        if ((previous_mask & previous_bit) == 0) {
          continue;
        }

        const Cost previous_cost = dp[index(previous_mask, previous)];
        if (previous_cost == kInf) {
          continue;
        }

        const Cost candidate = previous_cost +
                               edge_cost(problem, nodes[previous], nodes[last]);
        if (candidate < best_cost ||
            (candidate == best_cost && nodes[previous] < best_parent)) {
          best_cost = candidate;
          best_parent = nodes[previous];
        }
      }

      dp[index(mask, last)] = best_cost;
      parent[index(mask, last)] = best_parent;
    }
  }

  const std::size_t full_mask = subset_count - 1;
  Cost best_total = kInf;
  std::size_t best_last = 0;

  for (std::size_t last = 0; last < nodes.size(); ++last) {
    const Cost path_cost = dp[index(full_mask, last)];
    if (path_cost == kInf) {
      continue;
    }

    const Cost candidate = path_cost + edge_cost(problem, nodes[last], start);
    if (candidate < best_total || (candidate == best_total && last < best_last)) {
      best_total = candidate;
      best_last = last;
    }
  }

  if (best_total == kInf) {
    throw std::out_of_range("held-karp solver could not construct a tour");
  }

  std::vector<NodeId> reversed_order;
  reversed_order.reserve(node_count);
  reversed_order.push_back(nodes[best_last]);

  std::size_t mask = full_mask;
  std::size_t last = best_last;
  while (true) {
    const std::size_t last_bit = std::size_t{1} << last;
    const std::size_t previous_mask = mask ^ last_bit;
    if (previous_mask == 0) {
      break;
    }

    const NodeId previous = parent[index(mask, last)];
    reversed_order.push_back(previous);
    last = static_cast<std::size_t>(previous < start ? previous : previous - 1);
    mask = previous_mask;
  }

  std::vector<NodeId> order;
  order.reserve(node_count);
  order.push_back(start);
  for (auto it = reversed_order.rbegin(); it != reversed_order.rend(); ++it) {
    order.push_back(*it);
  }

  Tour tour;
  tour.order = std::move(order);
  tour.total_cost = best_total;
  return tour;
}

} // namespace tsp_solver
