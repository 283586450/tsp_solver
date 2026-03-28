#pragma once

#include "tsp_solver/core/problem.hpp"
#include "tsp_solver/core/tour.hpp"
#include "tsp_solver/export.h"

namespace tsp_solver {

class TSP_SOLVER_API LocalSearchSolver {
public:
  virtual ~LocalSearchSolver() = default;
  [[nodiscard]] virtual Tour solve(const Problem& problem) const = 0;
};

class TSP_SOLVER_API TwoOptLocalSearch final : public LocalSearchSolver {
public:
  explicit TwoOptLocalSearch(NodeId start_node = 0) noexcept;
  [[nodiscard]] Tour solve(const Problem& problem) const override;

private:
  NodeId start_node_ = 0;
};

TSP_SOLVER_API Tour nearest_neighbor_tour(const Problem& problem,
                                          NodeId start_node = 0);
TSP_SOLVER_API Tour cheapest_insertion_tour(const Problem& problem,
                                            NodeId start_node = 0);
TSP_SOLVER_API Tour iterated_local_search_tour(const Problem& problem,
                                               NodeId start_node = 0,
                                               std::uint64_t random_seed = 0);

} // namespace tsp_solver
