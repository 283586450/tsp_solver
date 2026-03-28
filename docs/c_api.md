# TSP Solver C API

The C API provides opaque handles for models, options, and results. Include `include/tsp_solver/c_api.h` and follow this flow:

1. Create a model.
2. Add nodes and set distances.
3. Create options and choose an algorithm.
4. Solve.
5. Copy the tour, objective, and selected algorithm from the result.

Available algorithms are `TSP_SOLVER_ALGORITHM_DEFAULT`,
`TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT`,
`TSP_SOLVER_ALGORITHM_GREEDY_NEAREST_NEIGHBOR`,
`TSP_SOLVER_ALGORITHM_GREEDY_CHEAPEST_INSERTION`, and
`TSP_SOLVER_ALGORITHM_HELD_KARP`.
The metaheuristic `TSP_SOLVER_ALGORITHM_METAHEURISTIC_ITERATED_LOCAL_SEARCH`
uses the configured random seed for deterministic perturbations.
`TSP_SOLVER_ALGORITHM_HELD_KARP` returns `TSP_SOLVER_ERROR_OUT_OF_RANGE` above
18 nodes.

```c
#include "tsp_solver/c_api.h"

tsp_solver_model_t* model = NULL;
tsp_solver_options_t* options = NULL;
tsp_solver_result_t* result = NULL;
tsp_solver_node_id_t node0 = 0;
tsp_solver_node_id_t node1 = 0;

if (tsp_solver_model_create(&model) != TSP_SOLVER_ERROR_OK) {
  return 1;
}
if (tsp_solver_options_create(&options) != TSP_SOLVER_ERROR_OK) {
  tsp_solver_model_destroy(model);
  return 1;
}

if (tsp_solver_model_add_node(model, &node0) != TSP_SOLVER_ERROR_OK) {
  return 1;
}
if (tsp_solver_model_add_node(model, &node1) != TSP_SOLVER_ERROR_OK) {
  return 1;
}
if (tsp_solver_model_set_distance(model, node0, node1, 10) != TSP_SOLVER_ERROR_OK) {
  return 1;
}
if (tsp_solver_model_set_distance(model, node1, node0, 12) != TSP_SOLVER_ERROR_OK) {
  return 1;
}
if (tsp_solver_model_validate(model) != TSP_SOLVER_ERROR_OK) {
  return 1;
}

if (tsp_solver_options_set_algorithm(options, TSP_SOLVER_ALGORITHM_DEFAULT) !=
    TSP_SOLVER_ERROR_OK) {
  return 1;
}
if (tsp_solver_options_set_time_limit_ms(options, 1000) != TSP_SOLVER_ERROR_OK) {
  return 1;
}

if (tsp_solver_solve(model, options, &result) == TSP_SOLVER_ERROR_OK) {
  tsp_solver_status_t status = TSP_SOLVER_STATUS_NOT_SOLVED;
  tsp_solver_algorithm_t algorithm = TSP_SOLVER_ALGORITHM_DEFAULT;
  tsp_solver_cost_t objective = 0;
  tsp_solver_node_id_t tour[2] = {0, 0};
  size_t tour_size = 0;
  size_t written = 0;

  tsp_solver_result_get_status(result, &status);
  tsp_solver_result_get_algorithm(result, &algorithm);
  tsp_solver_result_get_objective(result, &objective);
  tsp_solver_result_get_tour_size(result, &tour_size);
  tsp_solver_result_get_tour(result, tour, 2, &written);
}

tsp_solver_result_destroy(result);
tsp_solver_options_destroy(options);
tsp_solver_model_destroy(model);
```

Use `tsp_solver_result_get_tour_size` first, then allocate a buffer of that size and call `tsp_solver_result_get_tour` to copy out the node order.
