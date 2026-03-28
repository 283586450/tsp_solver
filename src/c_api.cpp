#include "tsp_solver/c_api.h"

#include "tsp_solver/algorithms/held_karp.hpp"
#include "tsp_solver/algorithms/local_search.hpp"
#include "tsp_solver/core/problem.hpp"
#include "tsp_solver/core/tour.hpp"

#include <algorithm>
#include <limits>
#include <new>
#include <optional>
#include <stdexcept>
#include <vector>

struct tsp_solver_model {
  std::vector<std::vector<std::optional<tsp_solver::Cost>>> distances;
};

struct tsp_solver_options {
  std::uint64_t time_limit_ms = UINT64_MAX;
  std::uint64_t random_seed = 0;
  tsp_solver_algorithm_t algorithm = TSP_SOLVER_ALGORITHM_DEFAULT;
};

struct tsp_solver_result {
  tsp_solver_status_t status = TSP_SOLVER_STATUS_NOT_SOLVED;
  tsp_solver_algorithm_t algorithm = TSP_SOLVER_ALGORITHM_DEFAULT;
  tsp_solver::Cost objective = 0;
  std::vector<tsp_solver::NodeId> tour;
};

namespace {

#define TSP_SOLVER_STRINGIFY_IMPL(value) #value
#define TSP_SOLVER_STRINGIFY(value) TSP_SOLVER_STRINGIFY_IMPL(value)

constexpr char kVersionString[] =
    TSP_SOLVER_STRINGIFY(TSP_SOLVER_VERSION_MAJOR) "." TSP_SOLVER_STRINGIFY(
        TSP_SOLVER_VERSION_MINOR) "." TSP_SOLVER_STRINGIFY(TSP_SOLVER_VERSION_PATCH);

[[nodiscard]] bool is_valid_algorithm(tsp_solver_algorithm_t algorithm) {
  return algorithm == TSP_SOLVER_ALGORITHM_DEFAULT ||
         algorithm == TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT ||
         algorithm == TSP_SOLVER_ALGORITHM_GREEDY_NEAREST_NEIGHBOR ||
         algorithm == TSP_SOLVER_ALGORITHM_GREEDY_CHEAPEST_INSERTION ||
         algorithm == TSP_SOLVER_ALGORITHM_HELD_KARP ||
         algorithm == TSP_SOLVER_ALGORITHM_METAHEURISTIC_ITERATED_LOCAL_SEARCH;
}

[[nodiscard]] tsp_solver_algorithm_t resolve_algorithm(tsp_solver_algorithm_t algorithm) {
  if (algorithm == TSP_SOLVER_ALGORITHM_DEFAULT) {
    return TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT;
  }
  return algorithm;
}

[[nodiscard]] tsp_solver::NodeId seed_start_node(const tsp_solver_model& model,
                                                 std::uint64_t random_seed) {
  const std::size_t node_count = model.distances.size();
  if (node_count == 0) {
    return 0;
  }

  return static_cast<tsp_solver::NodeId>(
      random_seed % static_cast<std::uint64_t>(node_count));
}

[[nodiscard]] bool is_square(const tsp_solver_model& model) {
  const std::size_t node_count = model.distances.size();
  for (const auto& row : model.distances) {
    if (row.size() != node_count) {
      return false;
    }
  }
  return true;
}

[[nodiscard]] bool is_complete(const tsp_solver_model& model) {
  if (!is_square(model)) {
    return false;
  }

  const std::size_t node_count = model.distances.size();
  for (std::size_t from = 0; from < node_count; ++from) {
    for (std::size_t to = 0; to < node_count; ++to) {
      if (!model.distances[from][to].has_value()) {
        return false;
      }
    }
  }
  return true;
}

[[nodiscard]] tsp_solver::Problem to_problem(const tsp_solver_model& model) {
  tsp_solver::Problem problem;
  problem.distances.resize(model.distances.size());

  for (std::size_t from = 0; from < model.distances.size(); ++from) {
    auto& row = problem.distances[from];
    row.resize(model.distances.size());
    for (std::size_t to = 0; to < model.distances.size(); ++to) {
      row[to] = model.distances[from][to].value_or(0);
    }
  }

  return problem;
}

[[nodiscard]] tsp_solver_error_code_t
validate_result_handle(const tsp_solver_result_t* result) {
  return result == nullptr ? TSP_SOLVER_ERROR_INVALID_ARGUMENT : TSP_SOLVER_ERROR_OK;
}

} // namespace

extern "C" {

const char* tsp_solver_version_string(void) {
  return kVersionString;
}

tsp_solver_error_code_t tsp_solver_model_create(tsp_solver_model_t** out_model) {
  if (out_model == nullptr) {
    return TSP_SOLVER_ERROR_INVALID_ARGUMENT;
  }

  *out_model = nullptr;
  try {
    auto* model = new (std::nothrow) tsp_solver_model{};
    if (model == nullptr) {
      return TSP_SOLVER_ERROR_ALLOCATION_FAILED;
    }

    *out_model = model;
    return TSP_SOLVER_ERROR_OK;
  } catch (...) {
    return TSP_SOLVER_ERROR_INTERNAL_ERROR;
  }
}

void tsp_solver_model_destroy(tsp_solver_model_t* model) {
  delete model;
}

tsp_solver_error_code_t tsp_solver_model_add_node(tsp_solver_model_t* model,
                                                  tsp_solver_node_id_t* out_node_id) {
  if (model == nullptr || out_node_id == nullptr) {
    return TSP_SOLVER_ERROR_INVALID_ARGUMENT;
  }

  try {
    if (model->distances.size() > std::numeric_limits<tsp_solver_node_id_t>::max()) {
      return TSP_SOLVER_ERROR_OUT_OF_RANGE;
    }

    const std::size_t node_id = model->distances.size();
    for (auto& row : model->distances) {
      row.push_back(std::nullopt);
    }

    model->distances.emplace_back(node_id + 1, std::nullopt);
    model->distances[node_id][node_id] = 0;

    *out_node_id = static_cast<tsp_solver_node_id_t>(node_id);
    return TSP_SOLVER_ERROR_OK;
  } catch (const std::bad_alloc&) {
    return TSP_SOLVER_ERROR_ALLOCATION_FAILED;
  } catch (...) {
    return TSP_SOLVER_ERROR_INTERNAL_ERROR;
  }
}

tsp_solver_error_code_t tsp_solver_model_set_distance(tsp_solver_model_t* model,
                                                      tsp_solver_node_id_t from,
                                                      tsp_solver_node_id_t to,
                                                      tsp_solver_cost_t distance) {
  if (model == nullptr) {
    return TSP_SOLVER_ERROR_INVALID_ARGUMENT;
  }
  if (from >= model->distances.size() || to >= model->distances.size()) {
    return TSP_SOLVER_ERROR_OUT_OF_RANGE;
  }

  model->distances[from][to] = (from == to) ? 0 : distance;
  return TSP_SOLVER_ERROR_OK;
}

tsp_solver_error_code_t tsp_solver_model_validate(const tsp_solver_model_t* model) {
  if (model == nullptr) {
    return TSP_SOLVER_ERROR_INVALID_ARGUMENT;
  }
  if (!is_complete(*model)) {
    return TSP_SOLVER_ERROR_INVALID_MODEL;
  }
  return TSP_SOLVER_ERROR_OK;
}

tsp_solver_error_code_t tsp_solver_options_create(tsp_solver_options_t** out_options) {
  if (out_options == nullptr) {
    return TSP_SOLVER_ERROR_INVALID_ARGUMENT;
  }

  *out_options = nullptr;
  try {
    auto* options = new (std::nothrow) tsp_solver_options{};
    if (options == nullptr) {
      return TSP_SOLVER_ERROR_ALLOCATION_FAILED;
    }

    *out_options = options;
    return TSP_SOLVER_ERROR_OK;
  } catch (...) {
    return TSP_SOLVER_ERROR_INTERNAL_ERROR;
  }
}

void tsp_solver_options_destroy(tsp_solver_options_t* options) {
  delete options;
}

tsp_solver_error_code_t
tsp_solver_options_set_time_limit_ms(tsp_solver_options_t* options,
                                     std::uint64_t time_limit_ms) {
  if (options == nullptr) {
    return TSP_SOLVER_ERROR_INVALID_ARGUMENT;
  }

  options->time_limit_ms = time_limit_ms;
  return TSP_SOLVER_ERROR_OK;
}

tsp_solver_error_code_t
tsp_solver_options_set_random_seed(tsp_solver_options_t* options,
                                   std::uint64_t random_seed) {
  if (options == nullptr) {
    return TSP_SOLVER_ERROR_INVALID_ARGUMENT;
  }

  options->random_seed = random_seed;
  return TSP_SOLVER_ERROR_OK;
}

tsp_solver_error_code_t
tsp_solver_options_set_algorithm(tsp_solver_options_t* options,
                                 tsp_solver_algorithm_t algorithm) {
  if (options == nullptr) {
    return TSP_SOLVER_ERROR_INVALID_ARGUMENT;
  }
  if (!is_valid_algorithm(algorithm)) {
    return TSP_SOLVER_ERROR_INVALID_ARGUMENT;
  }

  options->algorithm = algorithm;
  return TSP_SOLVER_ERROR_OK;
}

tsp_solver_error_code_t tsp_solver_solve(const tsp_solver_model_t* model,
                                         const tsp_solver_options_t* options,
                                         tsp_solver_result_t** out_result) {
  if (model == nullptr || options == nullptr || out_result == nullptr) {
    return TSP_SOLVER_ERROR_INVALID_ARGUMENT;
  }

  *out_result = nullptr;
  if (!is_complete(*model)) {
    return TSP_SOLVER_ERROR_INVALID_MODEL;
  }
  if (!is_valid_algorithm(options->algorithm)) {
    return TSP_SOLVER_ERROR_INVALID_ARGUMENT;
  }

  try {
    const tsp_solver::Problem problem = to_problem(*model);
    const tsp_solver_algorithm_t resolved_algorithm = resolve_algorithm(options->algorithm);
    const tsp_solver::NodeId start_node = seed_start_node(*model, options->random_seed);

    if (options->time_limit_ms == 0) {
      auto* result = new (std::nothrow) tsp_solver_result{};
      if (result == nullptr) {
        return TSP_SOLVER_ERROR_ALLOCATION_FAILED;
      }

      result->status = TSP_SOLVER_STATUS_TIME_LIMIT;
      result->algorithm = resolved_algorithm;
      *out_result = result;
      return TSP_SOLVER_ERROR_OK;
    }

    tsp_solver::Tour tour;
    switch (resolved_algorithm) {
    case TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT: {
      tsp_solver::TwoOptLocalSearch solver(start_node);
      tour = solver.solve(problem);
      break;
    }
    case TSP_SOLVER_ALGORITHM_GREEDY_NEAREST_NEIGHBOR:
      tour = tsp_solver::nearest_neighbor_tour(problem, start_node);
      break;
    case TSP_SOLVER_ALGORITHM_GREEDY_CHEAPEST_INSERTION:
      tour = tsp_solver::cheapest_insertion_tour(problem, start_node);
      break;
    case TSP_SOLVER_ALGORITHM_HELD_KARP:
      tour = tsp_solver::held_karp_tour(problem, start_node);
      break;
    case TSP_SOLVER_ALGORITHM_METAHEURISTIC_ITERATED_LOCAL_SEARCH:
      tour = tsp_solver::iterated_local_search_tour(problem, start_node,
                                                    options->random_seed);
      break;
    case TSP_SOLVER_ALGORITHM_DEFAULT:
      return TSP_SOLVER_ERROR_INVALID_ARGUMENT;
    }

    auto* result = new (std::nothrow) tsp_solver_result{};
    if (result == nullptr) {
      return TSP_SOLVER_ERROR_ALLOCATION_FAILED;
    }

    result->status = resolved_algorithm == TSP_SOLVER_ALGORITHM_HELD_KARP
                         ? TSP_SOLVER_STATUS_OPTIMAL
                         : TSP_SOLVER_STATUS_FEASIBLE;
    result->algorithm = resolved_algorithm;
    result->objective = tour.total_cost;
    result->tour = tour.order;

    *out_result = result;
    return TSP_SOLVER_ERROR_OK;
  } catch (const std::bad_alloc&) {
    return TSP_SOLVER_ERROR_ALLOCATION_FAILED;
  } catch (const std::out_of_range&) {
    return TSP_SOLVER_ERROR_OUT_OF_RANGE;
  } catch (...) {
    return TSP_SOLVER_ERROR_INTERNAL_ERROR;
  }
}

void tsp_solver_result_destroy(tsp_solver_result_t* result) {
  delete result;
}

tsp_solver_error_code_t tsp_solver_result_get_status(const tsp_solver_result_t* result,
                                                      tsp_solver_status_t* out_status) {
  if (validate_result_handle(result) != TSP_SOLVER_ERROR_OK || out_status == nullptr) {
    return TSP_SOLVER_ERROR_INVALID_ARGUMENT;
  }

  *out_status = result->status;
  return TSP_SOLVER_ERROR_OK;
}

tsp_solver_error_code_t tsp_solver_result_get_algorithm(const tsp_solver_result_t* result,
                                                        tsp_solver_algorithm_t* out_algorithm) {
  if (validate_result_handle(result) != TSP_SOLVER_ERROR_OK || out_algorithm == nullptr) {
    return TSP_SOLVER_ERROR_INVALID_ARGUMENT;
  }

  *out_algorithm = result->algorithm;
  return TSP_SOLVER_ERROR_OK;
}

tsp_solver_error_code_t
tsp_solver_result_get_objective(const tsp_solver_result_t* result,
                                tsp_solver_cost_t* out_objective) {
  if (validate_result_handle(result) != TSP_SOLVER_ERROR_OK ||
      out_objective == nullptr) {
    return TSP_SOLVER_ERROR_INVALID_ARGUMENT;
  }

  *out_objective = result->objective;
  return TSP_SOLVER_ERROR_OK;
}

tsp_solver_error_code_t
tsp_solver_result_get_tour_size(const tsp_solver_result_t* result,
                                std::size_t* out_size) {
  if (validate_result_handle(result) != TSP_SOLVER_ERROR_OK || out_size == nullptr) {
    return TSP_SOLVER_ERROR_INVALID_ARGUMENT;
  }

  *out_size = result->tour.size();
  return TSP_SOLVER_ERROR_OK;
}

tsp_solver_error_code_t tsp_solver_result_get_tour(const tsp_solver_result_t* result,
                                                   tsp_solver_node_id_t* out_nodes,
                                                   std::size_t capacity,
                                                   std::size_t* out_written) {
  if (validate_result_handle(result) != TSP_SOLVER_ERROR_OK || out_nodes == nullptr ||
      out_written == nullptr) {
    return TSP_SOLVER_ERROR_INVALID_ARGUMENT;
  }
  if (capacity < result->tour.size()) {
    return TSP_SOLVER_ERROR_INVALID_ARGUMENT;
  }

  for (std::size_t index = 0; index < result->tour.size(); ++index) {
    out_nodes[index] = result->tour[index];
  }
  *out_written = result->tour.size();
  return TSP_SOLVER_ERROR_OK;
}

} // extern "C"
