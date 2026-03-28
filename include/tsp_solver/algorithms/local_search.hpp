#pragma once

#include "tsp_solver/core/problem.hpp"
#include "tsp_solver/core/tour.hpp"

namespace tsp_solver {

class LocalSearchSolver {
public:
  virtual ~LocalSearchSolver() = default;
  [[nodiscard]] virtual Tour solve(const Problem& problem) const = 0;
};

class TwoOptLocalSearch final : public LocalSearchSolver {
public:
  [[nodiscard]] Tour solve(const Problem& problem) const override;
};

} // namespace tsp_solver
