from __future__ import annotations

import sys
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[2]
PYTHON_BINDINGS = ROOT / "bindings" / "python"
if str(PYTHON_BINDINGS) not in sys.path:
    sys.path.insert(0, str(PYTHON_BINDINGS))


import tsp_solver


class ModelBindingTest(unittest.TestCase):
    def test_complete_graph_solve(self) -> None:
        model = tsp_solver.Model()
        options = tsp_solver.Options()
        options.algorithm = tsp_solver.Algorithm.LOCAL_SEARCH_2OPT

        try:
            for _ in range(4):
                model.add_node()

            distances = (
                (0, 2, 9, 10),
                (1, 0, 6, 4),
                (15, 7, 0, 8),
                (6, 3, 12, 0),
            )
            for from_id, row in enumerate(distances):
                for to_id, distance in enumerate(row):
                    model.set_distance(from_id, to_id, distance)

            model.validate()
            result = model.solve(options)
            try:
                self.assertEqual(result.status, tsp_solver.Status.FEASIBLE)
                self.assertEqual(result.tour_size, 4)
                self.assertEqual(sorted(result.tour), [0, 1, 2, 3])
                self.assertIsInstance(result.objective, int)
                self.assertGreaterEqual(result.objective, 0)
            finally:
                result.close()
        finally:
            options.close()
            model.close()

    def test_validation_errors(self) -> None:
        model = tsp_solver.Model()
        try:
            model.add_node()
            model.add_node()

            with self.assertRaises(ValueError):
                model.validate()

            with self.assertRaises(IndexError):
                model.set_distance(2, 0, 5)

            with self.assertRaises(OverflowError):
                model.set_distance(2**32, 0, 5)
        finally:
            model.close()

    def test_context_manager_and_close_are_idempotent(self) -> None:
        with tsp_solver.Model() as model:
            self.assertEqual(model.add_node(), 0)

        model.close()
        model.close()


if __name__ == "__main__":
    unittest.main()
