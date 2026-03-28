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
  [[nodiscard]] Tour solve(const Problem& problem) const override;
};

} // namespace tsp_solver
