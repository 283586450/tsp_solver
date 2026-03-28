from __future__ import annotations

import subprocess
import sys
from pathlib import Path
import unittest


class ExamplesPythonSmokeTest(unittest.TestCase):
    def test_compare_algorithms_output(self) -> None:
        repo_root = Path(__file__).resolve().parents[1]
        output = subprocess.check_output(
            [
                sys.executable,
                str(repo_root / "examples" / "python" / "compare_algorithms.py"),
                str(repo_root / "examples" / "data" / "20_city_distance_matrix.txt"),
            ],
            text=True,
        )

        self.assertIn("TSP_SOLVER_ALGORITHM_DEFAULT", output)
        self.assertIn("TSP_SOLVER_ALGORITHM_GREEDY_NEAREST_NEIGHBOR", output)
        self.assertIn("TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT", output)


if __name__ == "__main__":
    unittest.main()
