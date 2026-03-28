from __future__ import annotations

from ._binding import Algorithm, Model, Options, Result, Status, solve
from ._errors import (
    LibraryLoadError,
    NativeCallError,
    SolverError,
    VersionMismatchError,
)
from ._version import package_version

__version__ = package_version()

__all__ = [
    "Algorithm",
    "LibraryLoadError",
    "Model",
    "NativeCallError",
    "Options",
    "Result",
    "Status",
    "SolverError",
    "VersionMismatchError",
    "__version__",
    "solve",
]
