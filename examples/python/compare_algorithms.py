from __future__ import annotations

import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
PYTHON_BINDINGS = REPO_ROOT / "bindings" / "python"
if str(PYTHON_BINDINGS) not in sys.path:
    sys.path.insert(0, str(PYTHON_BINDINGS))

import tsp_solver


ALGORITHMS: list[tuple[str, tsp_solver.Algorithm]] = [
    ("TSP_SOLVER_ALGORITHM_DEFAULT", tsp_solver.Algorithm.DEFAULT),
    (
        "TSP_SOLVER_ALGORITHM_GREEDY_NEAREST_NEIGHBOR",
        tsp_solver.Algorithm.GREEDY_NEAREST_NEIGHBOR,
    ),
    ("TSP_SOLVER_ALGORITHM_LOCAL_SEARCH_2OPT", tsp_solver.Algorithm.LOCAL_SEARCH_2OPT),
]


def load_matrix(path: Path) -> list[list[int]]:
    content = path.read_text(encoding="utf-8").split()
    if not content:
        raise ValueError(f"matrix file is empty: {path}")

    size = int(content[0])
    if size <= 0:
        raise ValueError(f"matrix size must be positive: {path}")
    expected = 1 + size * size
    if len(content) != expected:
        raise ValueError(
            f"matrix file must contain {size}x{size} values, found {len(content) - 1}"
        )

    values = [int(value) for value in content[1:]]
    return [values[index : index + size] for index in range(0, len(values), size)]


def build_model(matrix: list[list[int]]) -> tsp_solver.Model:
    model = tsp_solver.Model()
    try:
        for _ in range(len(matrix)):
            model.add_node()
        for from_node, row in enumerate(matrix):
            for to_node, distance in enumerate(row):
                model.set_distance(from_node, to_node, distance)
        model.validate()
        return model
    except Exception:
        model.close()
        raise


def run(path: Path) -> int:
    matrix = load_matrix(path)
    with build_model(matrix) as model, tsp_solver.Options() as options:
        print("algorithm\tobjective\ttour")
        for label, algorithm in ALGORITHMS:
            options.algorithm = algorithm
            with model.solve(options) as result:
                print(
                    f"{label}\t{result.objective}\t{' '.join(str(node) for node in result.tour)}"
                )
        return 0


def main(argv: list[str] | None = None) -> int:
    args = sys.argv[1:] if argv is None else argv
    if len(args) != 1:
        print("usage: compare_algorithms.py <matrix_path>", file=sys.stderr)
        return 1

    try:
        return run(Path(args[0]))
    except (OSError, ValueError, tsp_solver.SolverError) as exc:
        print(str(exc), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
