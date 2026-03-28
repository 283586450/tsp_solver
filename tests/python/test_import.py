from __future__ import annotations

import sys
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[2]
PYTHON_BINDINGS = ROOT / "bindings" / "python"
if str(PYTHON_BINDINGS) not in sys.path:
    sys.path.insert(0, str(PYTHON_BINDINGS))


class ImportTest(unittest.TestCase):
    def test_public_api_exports_exist(self) -> None:
        import tsp_solver
        from tsp_solver import Algorithm, Model, Options, Result, Status, solve

        expected_exports = {
            "Model",
            "Options",
            "Result",
            "Algorithm",
            "Status",
            "solve",
        }
        self.assertTrue(expected_exports.issubset(set(tsp_solver.__all__)))

        self.assertIs(tsp_solver.Model, Model)
        self.assertIs(tsp_solver.Options, Options)
        self.assertIs(tsp_solver.Result, Result)
        self.assertIs(tsp_solver.Algorithm, Algorithm)
        self.assertIs(tsp_solver.Status, Status)
        self.assertIs(tsp_solver.solve, solve)


if __name__ == "__main__":
    unittest.main()
