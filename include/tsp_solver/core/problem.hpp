#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace tsp_solver {

using NodeId = std::uint32_t;
using Cost = std::int64_t;

struct Problem {
  std::vector<std::vector<Cost>> distances;

  [[nodiscard]] std::size_t size() const noexcept {
    return distances.size();
  }

  [[nodiscard]] bool is_square() const noexcept;
};

}  // namespace tsp_solver
