from __future__ import annotations

import unittest

import tsp_solver


class PackageSmokeTest(unittest.TestCase):
    def test_installed_wheel_solves_small_complete_graph(self) -> None:
        with tsp_solver.Model() as model, tsp_solver.Options() as options:
            options.algorithm = tsp_solver.Algorithm.LOCAL_SEARCH_2OPT

            for _ in range(4):
                model.add_node()

            distances = [
                [0, 2, 9, 10],
                [1, 0, 6, 4],
                [15, 7, 0, 8],
                [6, 3, 12, 0],
            ]

            for from_node, row in enumerate(distances):
                for to_node, distance in enumerate(row):
                    model.set_distance(from_node, to_node, distance)

            model.validate()
            result = model.solve(options)
            self.assertEqual(tsp_solver.Status.FEASIBLE, result.status)
            self.assertEqual(4, result.tour_size)
            self.assertEqual([0, 1, 2, 3], sorted(result.tour))
            self.assertGreaterEqual(result.objective, 0)


if __name__ == "__main__":
    unittest.main()
