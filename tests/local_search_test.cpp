#include <cassert>

#include "tsp_solver/algorithms/local_search.hpp"

int main() {
  tsp_solver::Problem problem{{
      {0, 2, 9, 10},
      {1, 0, 6, 4},
      {15, 7, 0, 8},
      {6, 3, 12, 0},
  }};

  tsp_solver::TwoOptLocalSearch solver;
  const auto tour = solver.solve(problem);

  assert(tour.order.size() == 4);
  assert(tour.total_cost > 0);
  assert(tsp_solver::compute_tour_cost(problem, tour) == tour.total_cost);
  return 0;
}
