from __future__ import annotations

from importlib.metadata import PackageNotFoundError, version

from ._binding import Algorithm, Model, Options, Result, Status, solve
from ._errors import LibraryLoadError, NativeCallError, SolverError

try:
    __version__ = version("tsp-solver")
except PackageNotFoundError:
    __version__ = "0.1.0"

__all__ = [
    "Algorithm",
    "LibraryLoadError",
    "Model",
    "NativeCallError",
    "Options",
    "Result",
    "Status",
    "SolverError",
    "__version__",
    "solve",
]
