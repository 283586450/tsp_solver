from __future__ import annotations


class SolverError(RuntimeError):
    """Base error for Python bindings."""


__all__ = ["SolverError", "LibraryLoadError", "NativeCallError"]


class LibraryLoadError(SolverError):
    """Raised when the native library cannot be located or loaded."""


class NativeCallError(SolverError):
    """Raised when a native C API call reports an error."""
