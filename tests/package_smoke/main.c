#include "tsp_solver/c_api.h"

#include <stddef.h>

int main(void) {
  return tsp_solver_version_string() == NULL;
}
