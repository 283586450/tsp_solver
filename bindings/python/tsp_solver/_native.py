from __future__ import annotations

import ctypes
from ctypes.util import find_library
import enum
import os
from pathlib import Path
from typing import Final

from ._errors import LibraryLoadError, NativeCallError


__all__ = [
    "Algorithm",
    "Model",
    "Options",
    "Result",
    "Status",
    "solve",
]


_LIBRARY_NAME: Final[str] = "tsp_solver"
_library: ctypes.CDLL | None = None


class Algorithm(enum.IntEnum):
    DEFAULT = 0
    LOCAL_SEARCH_2OPT = 1


class Status(enum.IntEnum):
    NOT_SOLVED = 0
    FEASIBLE = 1
    OPTIMAL = 2
    INFEASIBLE = 3
    TIME_LIMIT = 4
    INVALID_MODEL = 5
    INTERNAL_ERROR = 6


class _OpaqueHandle:
    __slots__ = ("_handle",)

    def __init__(self, handle: ctypes.c_void_p | None = None) -> None:
        self._handle = handle

    @property
    def handle(self) -> ctypes.c_void_p | None:
        return self._handle

    def __repr__(self) -> str:
        return f"{self.__class__.__name__}(handle={self._handle!r})"


class Model(_OpaqueHandle):
    """Opaque native model handle."""


class Options(_OpaqueHandle):
    """Opaque native options handle."""


class Result(_OpaqueHandle):
    """Opaque native result handle."""


def load_library() -> ctypes.CDLL:
    """Load and cache the native tsp_solver shared library."""

    global _library
    if _library is not None:
        return _library

    attempts: list[str] = []
    local_error: OSError | None = None

    env_path = os.environ.get("TSP_SOLVER_LIBRARY_PATH")
    if env_path:
        attempts.append(f"TSP_SOLVER_LIBRARY_PATH={env_path!r}")
        try:
            _library = ctypes.CDLL(env_path)
            return _library
        except OSError as exc:
            env_error = exc
        else:
            env_error = None
    else:
        env_error = None

    local_library = _load_local_library()
    if local_library is not None:
        attempts.append("package-local library")
        _library = local_library
        return _library

    local_error = _local_library_error

    found = find_library(_LIBRARY_NAME)
    if found:
        attempts.append(f"ctypes.util.find_library({_LIBRARY_NAME!r}) -> {found!r}")
        try:
            _library = ctypes.CDLL(found)
            return _library
        except OSError as exc:
            find_error = exc
        else:
            find_error = None
    else:
        find_error = None

    details: list[str] = []
    if env_error is not None:
        details.append(f"env load failed: {env_error}")
    if local_error is not None:
        details.append(f"package-local load failed: {local_error}")
    if find_error is not None:
        details.append(f"find_library load failed: {find_error}")

    message = f"Unable to load the {_LIBRARY_NAME} native library."
    if attempts:
        message += " Attempts: " + "; ".join(attempts) + "."
    if details:
        message += " " + " ".join(details)
    raise LibraryLoadError(message)


_local_library_error: OSError | None = None


def _local_library_candidates() -> list[Path]:
    module_dir = Path(__file__).resolve().parent
    search_dirs: list[Path] = [module_dir, module_dir.parent]
    filenames = [_LIBRARY_NAME]
    if os.name == "nt":
        filenames.append(f"{_LIBRARY_NAME}.dll")
    else:
        filenames.extend([f"lib{_LIBRARY_NAME}.so", f"lib{_LIBRARY_NAME}.dylib"])

    candidates: list[Path] = []
    seen: set[Path] = set()
    for directory in search_dirs:
        for filename in filenames:
            candidate = directory / filename
            if candidate not in seen:
                seen.add(candidate)
                candidates.append(candidate)
    return candidates


def _load_local_library() -> ctypes.CDLL | None:
    global _local_library_error
    _local_library_error = None

    dll_directory_handles = []
    if os.name == "nt" and hasattr(os, "add_dll_directory"):
        seen_directories: set[Path] = set()
        for candidate in _local_library_candidates():
            directory = candidate.parent
            if directory in seen_directories:
                continue
            seen_directories.add(directory)
            try:
                handle = os.add_dll_directory(str(directory))
            except OSError:
                continue
            dll_directory_handles.append(handle)

    try:
        for candidate in _local_library_candidates():
            if candidate.exists():
                try:
                    return ctypes.CDLL(str(candidate))
                except OSError as exc:
                    _local_library_error = exc
    finally:
        for handle in dll_directory_handles:
            handle.close()

    return None


def solve(model: Model, options: Options) -> Result:
    """Solve a TSP instance using the native library."""

    _ = model
    _ = options
    load_library()
    raise NativeCallError(
        "Native solve wrapper is not wired up yet; the library loader is available."
    )
