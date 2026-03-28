# TSP Solver C API

C API 为模型、选项和结果提供不透明句柄。包含 `include/tsp_solver/c_api.h` 后，按以下流程使用：

1. 创建模型。
2. 添加节点并设置距离。
3. 创建选项并选择算法。
4. 求解。
5. 从结果中复制 tour、目标值和所选算法。

可用算法包括 `TSP_SOLVER_ALGORITHM_DEFAULT`、
`TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT`、
`TSP_SOLVER_ALGORITHM_GREEDY_NEAREST_NEIGHBOR`、
`TSP_SOLVER_ALGORITHM_GREEDY_CHEAPEST_INSERTION`、
`TSP_SOLVER_ALGORITHM_HELD_KARP` 和
`TSP_SOLVER_ALGORITHM_METAHEURISTIC_ITERATED_LOCAL_SEARCH`。
元启发式 `TSP_SOLVER_ALGORITHM_METAHEURISTIC_ITERATED_LOCAL_SEARCH`
会使用配置的随机种子来生成确定性的扰动。
`TSP_SOLVER_ALGORITHM_HELD_KARP` 在超过 18 个节点时会返回 `TSP_SOLVER_ERROR_OUT_OF_RANGE`。

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

请先调用 `tsp_solver_result_get_tour_size`，再分配同等大小的缓冲区，然后调用
`tsp_solver_result_get_tour` 来拷贝节点顺序。
