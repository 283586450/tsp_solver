#include "tsp_solver/algorithms/local_search.hpp"

#include <algorithm>
#include <cassert>

int main() {
  tsp_solver::Problem problem{{
      {0, 2, 9, 10},
      {1, 0, 6, 4},
      {15, 7, 0, 8},
      {6, 3, 12, 0},
  }};

  tsp_solver::TwoOptLocalSearch solver;
  const auto tour = solver.solve(problem);
  const auto nearest_neighbor = tsp_solver::nearest_neighbor_tour(problem);
  const auto iterated = tsp_solver::iterated_local_search_tour(problem, 42);

  assert(tour.order.size() == 4);
  assert(tour.total_cost > 0);
  assert(tsp_solver::compute_tour_cost(problem, tour) == tour.total_cost);
  assert(iterated.order.size() == 4);
  assert(std::is_permutation(iterated.order.begin(), iterated.order.end(),
                             nearest_neighbor.order.begin(),
                             nearest_neighbor.order.end()));
  assert(iterated.total_cost == tsp_solver::compute_tour_cost(problem, iterated));
  assert(iterated.total_cost <= nearest_neighbor.total_cost);
  return 0;
}
