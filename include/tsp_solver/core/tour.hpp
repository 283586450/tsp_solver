#pragma once

#include <vector>

#include "tsp_solver/core/problem.hpp"

namespace tsp_solver {

struct Tour {
  std::vector<NodeId> order;
  Cost total_cost = 0;

  [[nodiscard]] bool empty() const noexcept {
    return order.empty();
  }
};

[[nodiscard]] Cost compute_tour_cost(const Problem& problem,
                                     const Tour& tour);

}  // namespace tsp_solver
