#pragma once

#include "tsp_solver/core/problem.hpp"
#include "tsp_solver/core/tour.hpp"
#include "tsp_solver/export.h"

namespace tsp_solver {

TSP_SOLVER_API Tour held_karp_tour(const Problem& problem, NodeId start_node = 0);

} // namespace tsp_solver
