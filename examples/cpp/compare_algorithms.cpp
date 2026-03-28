#include "tsp_solver/c_api.h"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <utility>

namespace {

struct Matrix {
  std::size_t size = 0;
  std::vector<std::vector<tsp_solver_cost_t>> values;
};

std::string algorithm_name(tsp_solver_algorithm_t algorithm) {
  switch (algorithm) {
  case TSP_SOLVER_ALGORITHM_DEFAULT:
    return "TSP_SOLVER_ALGORITHM_DEFAULT";
  case TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT:
    return "TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT";
  case TSP_SOLVER_ALGORITHM_GREEDY_NEAREST_NEIGHBOR:
    return "TSP_SOLVER_ALGORITHM_GREEDY_NEAREST_NEIGHBOR";
  case TSP_SOLVER_ALGORITHM_GREEDY_CHEAPEST_INSERTION:
    return "TSP_SOLVER_ALGORITHM_GREEDY_CHEAPEST_INSERTION";
  case TSP_SOLVER_ALGORITHM_HELD_KARP:
    return "TSP_SOLVER_ALGORITHM_HELD_KARP";
  case TSP_SOLVER_ALGORITHM_METAHEURISTIC_ITERATED_LOCAL_SEARCH:
    return "TSP_SOLVER_ALGORITHM_METAHEURISTIC_ITERATED_LOCAL_SEARCH";
  }

  return "TSP_SOLVER_ALGORITHM_UNKNOWN";
}

bool parse_matrix(const std::string& path, Matrix* matrix, std::string* error) {
  std::ifstream input(path);
  if (!input.is_open()) {
    *error = "failed to open matrix file: " + path;
    return false;
  }

  std::size_t size = 0;
  if (!(input >> size) || size == 0) {
    *error = "matrix header is invalid";
    return false;
  }

  Matrix parsed;
  parsed.size = size;
  parsed.values.assign(size, std::vector<tsp_solver_cost_t>(size));

  for (std::size_t row = 0; row < size; ++row) {
    for (std::size_t col = 0; col < size; ++col) {
      long long value = 0;
      if (!(input >> value)) {
        *error = "matrix is missing values";
        return false;
      }
      parsed.values[row][col] = static_cast<tsp_solver_cost_t>(value);
    }
  }

  long long extra = 0;
  if (input >> extra) {
    *error = "matrix file contains extra values";
    return false;
  }

  *matrix = std::move(parsed);
  return true;
}

bool build_model(const Matrix& matrix, tsp_solver_model_t** out_model,
                 std::vector<tsp_solver_node_id_t>* node_ids, std::string* error) {
  tsp_solver_model_t* model = nullptr;
  if (tsp_solver_model_create(&model) != TSP_SOLVER_ERROR_OK) {
    *error = "failed to create model";
    return false;
  }

  node_ids->clear();
  node_ids->reserve(matrix.size);
  for (std::size_t index = 0; index < matrix.size; ++index) {
    tsp_solver_node_id_t node_id = 0;
    if (tsp_solver_model_add_node(model, &node_id) != TSP_SOLVER_ERROR_OK) {
      *error = "failed to add node";
      tsp_solver_model_destroy(model);
      return false;
    }
    node_ids->push_back(node_id);
  }

  for (std::size_t row = 0; row < matrix.size; ++row) {
    for (std::size_t col = 0; col < matrix.size; ++col) {
      if (tsp_solver_model_set_distance(model, node_ids->at(row), node_ids->at(col),
                                        matrix.values[row][col]) != TSP_SOLVER_ERROR_OK) {
        *error = "failed to set distance";
        tsp_solver_model_destroy(model);
        return false;
      }
    }
  }

  if (tsp_solver_model_validate(model) != TSP_SOLVER_ERROR_OK) {
    *error = "model validation failed";
    tsp_solver_model_destroy(model);
    return false;
  }

  *out_model = model;
  return true;
}

bool print_solution(tsp_solver_algorithm_t requested_algorithm,
                    const tsp_solver_result_t* result, std::ostream& stream) {
  tsp_solver_status_t status = TSP_SOLVER_STATUS_NOT_SOLVED;
  tsp_solver_cost_t objective = 0;
  std::size_t tour_size = 0;

  if (tsp_solver_result_get_status(result, &status) != TSP_SOLVER_ERROR_OK ||
      tsp_solver_result_get_objective(result, &objective) != TSP_SOLVER_ERROR_OK ||
      tsp_solver_result_get_tour_size(result, &tour_size) != TSP_SOLVER_ERROR_OK) {
    return false;
  }

  std::vector<tsp_solver_node_id_t> tour(tour_size);
  std::size_t written = 0;
  if (tour_size > 0 &&
      tsp_solver_result_get_tour(result, tour.data(), tour.size(), &written) !=
          TSP_SOLVER_ERROR_OK) {
    return false;
  }

  stream << algorithm_name(requested_algorithm) << '\t' << objective << '\t';
  for (std::size_t index = 0; index < written; ++index) {
    if (index > 0) {
      stream << ' ';
    }
    stream << tour[index];
  }
  stream << '\n';

  return status == TSP_SOLVER_STATUS_FEASIBLE || status == TSP_SOLVER_STATUS_OPTIMAL;
}

bool run_algorithm(tsp_solver_model_t* model, tsp_solver_algorithm_t algorithm,
                   std::ostream& stream, std::string* error) {
  tsp_solver_options_t* options = nullptr;
  if (tsp_solver_options_create(&options) != TSP_SOLVER_ERROR_OK) {
    *error = "failed to create options";
    return false;
  }

  if (tsp_solver_options_set_algorithm(options, algorithm) != TSP_SOLVER_ERROR_OK) {
    *error = "failed to set algorithm";
    tsp_solver_options_destroy(options);
    return false;
  }

  tsp_solver_result_t* result = nullptr;
  bool ok = tsp_solver_solve(model, options, &result) == TSP_SOLVER_ERROR_OK;
  if (!ok) {
    *error = "failed to solve model";
  } else if (!print_solution(algorithm, result, stream)) {
    *error = "failed to read result";
    ok = false;
  }

  tsp_solver_result_destroy(result);
  tsp_solver_options_destroy(options);
  return ok;
}

} // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "usage: compare_algorithms <matrix_path>\n";
    return 1;
  }

  Matrix matrix;
  std::string error;
  if (!parse_matrix(argv[1], &matrix, &error)) {
    std::cerr << error << '\n';
    return 1;
  }

  tsp_solver_model_t* model = nullptr;
  [[maybe_unused]] std::vector<tsp_solver_node_id_t> node_ids;
  if (!build_model(matrix, &model, &node_ids, &error)) {
    std::cerr << error << '\n';
    return 1;
  }

  std::cout << "algorithm\tobjective\ttour\n";
  bool ok = true;
  ok = run_algorithm(model, TSP_SOLVER_ALGORITHM_DEFAULT, std::cout, &error) && ok;
  ok = run_algorithm(model, TSP_SOLVER_ALGORITHM_GREEDY_NEAREST_NEIGHBOR, std::cout,
                     &error) && ok;
  ok = run_algorithm(model, TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT, std::cout, &error) &&
       ok;

  tsp_solver_model_destroy(model);

  if (!ok) {
    std::cerr << error << '\n';
    return 1;
  }

  return 0;
}
