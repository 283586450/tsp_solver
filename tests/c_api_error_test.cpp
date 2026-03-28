#include "tsp_solver/c_api.h"

#include <cstddef>

int main() {
  tsp_solver_model_t* model = nullptr;
  tsp_solver_options_t* options = nullptr;
  tsp_solver_result_t* result = nullptr;
  tsp_solver_node_id_t node_id = 0;

  if (tsp_solver_model_add_node(nullptr, &node_id) != TSP_SOLVER_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  if (tsp_solver_options_set_time_limit_ms(nullptr, 1) != TSP_SOLVER_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  if (tsp_solver_result_get_tour(nullptr, nullptr, 0, nullptr) !=
      TSP_SOLVER_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  if (tsp_solver_model_create(&model) != TSP_SOLVER_ERROR_OK) {
    return 1;
  }
  if (tsp_solver_options_create(&options) != TSP_SOLVER_ERROR_OK) {
    return 1;
  }
  if (tsp_solver_solve(nullptr, options, &result) != TSP_SOLVER_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  if (tsp_solver_model_add_node(model, &node_id) != TSP_SOLVER_ERROR_OK) {
    return 1;
  }
  if (tsp_solver_model_validate(model) != TSP_SOLVER_ERROR_OK) {
    return 1;
  }

  if (tsp_solver_model_set_distance(model, 1, 0, 5) != TSP_SOLVER_ERROR_OUT_OF_RANGE) {
    return 1;
  }

  tsp_solver_model_t* incomplete_model = nullptr;
  if (tsp_solver_model_create(&incomplete_model) != TSP_SOLVER_ERROR_OK) {
    return 1;
  }
  if (tsp_solver_model_add_node(incomplete_model, &node_id) != TSP_SOLVER_ERROR_OK) {
    return 1;
  }
  if (tsp_solver_model_add_node(incomplete_model, &node_id) != TSP_SOLVER_ERROR_OK) {
    return 1;
  }
  if (tsp_solver_model_validate(incomplete_model) != TSP_SOLVER_ERROR_INVALID_MODEL) {
    return 1;
  }

  if (tsp_solver_options_set_time_limit_ms(options, 0) != TSP_SOLVER_ERROR_OK) {
    return 1;
  }
  if (tsp_solver_solve(model, options, &result) != TSP_SOLVER_ERROR_OK) {
    return 1;
  }

  tsp_solver_status_t status = TSP_SOLVER_STATUS_NOT_SOLVED;
  if (tsp_solver_result_get_status(result, &status) != TSP_SOLVER_ERROR_OK) {
    return 1;
  }
  if (status != TSP_SOLVER_STATUS_TIME_LIMIT) {
    return 1;
  }

  tsp_solver_result_destroy(result);
  result = nullptr;

  if (tsp_solver_options_set_time_limit_ms(options, 1) != TSP_SOLVER_ERROR_OK) {
    return 1;
  }
  if (tsp_solver_solve(model, options, &result) != TSP_SOLVER_ERROR_OK) {
    return 1;
  }

  tsp_solver_node_id_t nodes[1] = {};
  std::size_t written = 0;
  if (tsp_solver_result_get_tour(result, nodes, 0, &written) !=
      TSP_SOLVER_ERROR_INVALID_ARGUMENT) {
    return 1;
  }
  if (tsp_solver_result_get_tour(result, nodes, 1, &written) != TSP_SOLVER_ERROR_OK) {
    return 1;
  }
  if (written != 1) {
    return 1;
  }

  tsp_solver_result_destroy(result);
  tsp_solver_options_destroy(options);
  tsp_solver_model_destroy(incomplete_model);
  tsp_solver_model_destroy(model);
  return 0;
}
