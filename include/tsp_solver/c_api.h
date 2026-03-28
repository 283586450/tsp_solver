#pragma once

#include "tsp_solver/export.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TSP_SOLVER_VERSION_MAJOR 0
#define TSP_SOLVER_VERSION_MINOR 2
#define TSP_SOLVER_VERSION_PATCH 0
#define TSP_SOLVER_ABI_VERSION 1

typedef struct tsp_solver_model tsp_solver_model_t;
typedef struct tsp_solver_options tsp_solver_options_t;
typedef struct tsp_solver_result tsp_solver_result_t;

typedef uint32_t tsp_solver_node_id_t;
typedef int64_t tsp_solver_cost_t;

typedef enum tsp_solver_error_code {
  TSP_SOLVER_ERROR_OK = 0,
  TSP_SOLVER_ERROR_INVALID_ARGUMENT,
  TSP_SOLVER_ERROR_OUT_OF_RANGE,
  TSP_SOLVER_ERROR_ALLOCATION_FAILED,
  TSP_SOLVER_ERROR_INVALID_MODEL,
  TSP_SOLVER_ERROR_INTERNAL_ERROR,
} tsp_solver_error_code_t;

typedef enum tsp_solver_status {
  TSP_SOLVER_STATUS_NOT_SOLVED = 0,
  TSP_SOLVER_STATUS_FEASIBLE,
  TSP_SOLVER_STATUS_OPTIMAL,
  TSP_SOLVER_STATUS_INFEASIBLE,
  TSP_SOLVER_STATUS_TIME_LIMIT,
  TSP_SOLVER_STATUS_INVALID_MODEL,
  TSP_SOLVER_STATUS_INTERNAL_ERROR,
} tsp_solver_status_t;

typedef enum tsp_solver_algorithm {
  TSP_SOLVER_ALGORITHM_DEFAULT = 0,
  TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT,
  TSP_SOLVER_ALGORITHM_GREEDY_NEAREST_NEIGHBOR,
  TSP_SOLVER_ALGORITHM_GREEDY_CHEAPEST_INSERTION,
  TSP_SOLVER_ALGORITHM_HELD_KARP,
  TSP_SOLVER_ALGORITHM_METAHEURISTIC_ITERATED_LOCAL_SEARCH,
} tsp_solver_algorithm_t;

TSP_SOLVER_API const char* tsp_solver_version_string(void);

TSP_SOLVER_API tsp_solver_error_code_t
tsp_solver_model_create(tsp_solver_model_t** out_model);
TSP_SOLVER_API void tsp_solver_model_destroy(tsp_solver_model_t* model);
TSP_SOLVER_API tsp_solver_error_code_t
tsp_solver_model_add_node(tsp_solver_model_t* model, tsp_solver_node_id_t* out_node_id);
TSP_SOLVER_API tsp_solver_error_code_t
tsp_solver_model_set_distance(tsp_solver_model_t* model, tsp_solver_node_id_t from,
                              tsp_solver_node_id_t to, tsp_solver_cost_t distance);
TSP_SOLVER_API tsp_solver_error_code_t
tsp_solver_model_validate(const tsp_solver_model_t* model);

TSP_SOLVER_API tsp_solver_error_code_t
tsp_solver_options_create(tsp_solver_options_t** out_options);
TSP_SOLVER_API void tsp_solver_options_destroy(tsp_solver_options_t* options);
TSP_SOLVER_API tsp_solver_error_code_t tsp_solver_options_set_time_limit_ms(
    tsp_solver_options_t* options, uint64_t time_limit_ms);
TSP_SOLVER_API tsp_solver_error_code_t
tsp_solver_options_set_random_seed(tsp_solver_options_t* options, uint64_t random_seed);
TSP_SOLVER_API tsp_solver_error_code_t tsp_solver_options_set_algorithm(
    tsp_solver_options_t* options, tsp_solver_algorithm_t algorithm);

TSP_SOLVER_API tsp_solver_error_code_t
tsp_solver_solve(const tsp_solver_model_t* model, const tsp_solver_options_t* options,
                 tsp_solver_result_t** out_result);

TSP_SOLVER_API void tsp_solver_result_destroy(tsp_solver_result_t* result);
TSP_SOLVER_API tsp_solver_error_code_t tsp_solver_result_get_status(
    const tsp_solver_result_t* result, tsp_solver_status_t* out_status);
TSP_SOLVER_API tsp_solver_error_code_t tsp_solver_result_get_algorithm(
    const tsp_solver_result_t* result, tsp_solver_algorithm_t* out_algorithm);
TSP_SOLVER_API tsp_solver_error_code_t tsp_solver_result_get_objective(
    const tsp_solver_result_t* result, tsp_solver_cost_t* out_objective);
TSP_SOLVER_API tsp_solver_error_code_t
tsp_solver_result_get_tour_size(const tsp_solver_result_t* result, size_t* out_size);
TSP_SOLVER_API tsp_solver_error_code_t tsp_solver_result_get_tour(
    const tsp_solver_result_t* result, tsp_solver_node_id_t* out_nodes, size_t capacity,
    size_t* out_written);

#ifdef __cplusplus
} // extern "C"
#endif
