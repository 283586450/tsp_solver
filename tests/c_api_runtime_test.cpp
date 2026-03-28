#include "tsp_solver/c_api.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <vector>

static tsp_solver_cost_t tour_cost(
    const std::array<std::array<tsp_solver_cost_t, 4>, 4>& distances,
    const std::vector<tsp_solver_node_id_t>& tour) {
  if (tour.empty()) {
    return 0;
  }

  tsp_solver_cost_t total = 0;
  for (std::size_t index = 0; index < tour.size(); ++index) {
    const std::size_t next = (index + 1) % tour.size();
    total += distances[tour[index]][tour[next]];
  }
  return total;
}

int main() {
  const std::array<std::array<tsp_solver_cost_t, 4>, 4> distances{{
      {{0, 2, 9, 10}},
      {{1, 0, 6, 4}},
      {{15, 7, 0, 8}},
      {{6, 3, 12, 0}},
  }};

  tsp_solver_model_t* model = nullptr;
  tsp_solver_model_t* incomplete_model = nullptr;
  tsp_solver_model_t* empty_model = nullptr;
  tsp_solver_options_t* options = nullptr;
  tsp_solver_result_t* result = nullptr;
  tsp_solver_result_t* default_result = nullptr;
  tsp_solver_result_t* time_limit_result = nullptr;
  tsp_solver_result_t* empty_result = nullptr;

  assert(tsp_solver_model_create(&model) == TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_model_create(&incomplete_model) == TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_model_create(&empty_model) == TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_options_create(&options) == TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_options_set_time_limit_ms(options, 1000) == TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_options_set_random_seed(options, 42) == TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_options_set_algorithm(options,
         TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT) == TSP_SOLVER_ERROR_OK);

  tsp_solver_node_id_t incomplete_ids[2] = {};
  for (std::size_t index = 0; index < 2; ++index) {
    assert(tsp_solver_model_add_node(incomplete_model, &incomplete_ids[index]) ==
           TSP_SOLVER_ERROR_OK);
  }
  assert(tsp_solver_model_validate(incomplete_model) == TSP_SOLVER_ERROR_INVALID_MODEL);
  assert(tsp_solver_model_validate(empty_model) == TSP_SOLVER_ERROR_OK);

  assert(tsp_solver_options_set_time_limit_ms(options, 0) == TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_solve(model, options, &time_limit_result) == TSP_SOLVER_ERROR_OK);
  tsp_solver_status_t time_limit_status = TSP_SOLVER_STATUS_NOT_SOLVED;
  assert(tsp_solver_result_get_status(time_limit_result, &time_limit_status) ==
         TSP_SOLVER_ERROR_OK);
  assert(time_limit_status == TSP_SOLVER_STATUS_TIME_LIMIT);
  tsp_solver_result_destroy(time_limit_result);
  time_limit_result = nullptr;
  assert(tsp_solver_options_set_time_limit_ms(options, 1000) == TSP_SOLVER_ERROR_OK);

  tsp_solver_node_id_t ids[4] = {};
  for (std::size_t index = 0; index < 4; ++index) {
    assert(tsp_solver_model_add_node(model, &ids[index]) == TSP_SOLVER_ERROR_OK);
  }

  for (tsp_solver_node_id_t from = 0; from < 4; ++from) {
    for (tsp_solver_node_id_t to = 0; to < 4; ++to) {
      assert(tsp_solver_model_set_distance(model, from, to, distances[from][to]) ==
             TSP_SOLVER_ERROR_OK);
    }
  }

  assert(tsp_solver_model_validate(model) == TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_solve(model, options, &result) == TSP_SOLVER_ERROR_OK);

  tsp_solver_status_t status = TSP_SOLVER_STATUS_NOT_SOLVED;
  tsp_solver_cost_t objective = 0;
  std::size_t tour_size = 0;
  assert(tsp_solver_result_get_status(result, &status) == TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_result_get_objective(result, &objective) == TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_result_get_tour_size(result, &tour_size) == TSP_SOLVER_ERROR_OK);
  assert(status == TSP_SOLVER_STATUS_FEASIBLE);
  assert(tour_size == 4);

  std::vector<tsp_solver_node_id_t> tour(tour_size);
  std::size_t written = 0;
  assert(tsp_solver_result_get_tour(result, tour.data(), tour.size() - 1, &written) ==
         TSP_SOLVER_ERROR_INVALID_ARGUMENT);
  assert(tsp_solver_result_get_tour(result, tour.data(), tour.size(), &written) ==
         TSP_SOLVER_ERROR_OK);
  assert(written == tour.size());

  std::vector<tsp_solver_node_id_t> sorted_tour = tour;
  std::sort(sorted_tour.begin(), sorted_tour.end());
  assert((sorted_tour == std::vector<tsp_solver_node_id_t>{0, 1, 2, 3}));
  assert(objective == tour_cost(distances, tour));

  assert(tsp_solver_options_set_algorithm(options,
         TSP_SOLVER_ALGORITHM_DEFAULT) == TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_solve(model, options, &default_result) == TSP_SOLVER_ERROR_OK);

  tsp_solver_status_t default_status = TSP_SOLVER_STATUS_NOT_SOLVED;
  tsp_solver_cost_t default_objective = 0;
  assert(tsp_solver_result_get_status(default_result, &default_status) ==
         TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_result_get_objective(default_result, &default_objective) ==
         TSP_SOLVER_ERROR_OK);
  assert(default_status == TSP_SOLVER_STATUS_FEASIBLE);
  assert(default_objective == objective);

  assert(tsp_solver_solve(empty_model, options, &empty_result) == TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_result_get_status(empty_result, &default_status) ==
         TSP_SOLVER_ERROR_OK);
  assert(tsp_solver_result_get_tour_size(empty_result, &tour_size) ==
         TSP_SOLVER_ERROR_OK);
  assert(default_status == TSP_SOLVER_STATUS_FEASIBLE);
  assert(tour_size == 0);

  tsp_solver_result_destroy(result);
  tsp_solver_result_destroy(default_result);
  tsp_solver_result_destroy(empty_result);
  tsp_solver_result_destroy(time_limit_result);
  tsp_solver_options_destroy(options);
  tsp_solver_model_destroy(incomplete_model);
  tsp_solver_model_destroy(empty_model);
  tsp_solver_model_destroy(model);
  return 0;
}
