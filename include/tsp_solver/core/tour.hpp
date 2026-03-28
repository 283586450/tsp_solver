#pragma once

#include "tsp_solver/core/problem.hpp"
#include "tsp_solver/export.h"

#include <vector>

namespace tsp_solver {

struct Tour {
  std::vector<NodeId> order;
  Cost total_cost = 0;

  [[nodiscard]] bool empty() const noexcept { return order.empty(); }
};

TSP_SOLVER_API [[nodiscard]] Cost compute_tour_cost(const Problem& problem,
                                                    const Tour& tour);

} // namespace tsp_solver
