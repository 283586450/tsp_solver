#include "tsp_solver/c_api.h"

#include <stddef.h>

int main(void) {
  _Static_assert(TSP_SOLVER_VERSION_MAJOR == 0, "major version");
  _Static_assert(TSP_SOLVER_VERSION_MINOR == 1, "minor version");
  _Static_assert(TSP_SOLVER_VERSION_PATCH == 0, "patch version");
  _Static_assert(sizeof(tsp_solver_cost_t) == 8, "cost width");

  tsp_solver_model_t* model = NULL;
  tsp_solver_options_t* options = NULL;
  tsp_solver_result_t* result = NULL;
  (void)model;
  (void)options;
  (void)result;
  return 0;
}
