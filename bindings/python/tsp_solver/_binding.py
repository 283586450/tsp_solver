from __future__ import annotations

import ctypes
import enum
from typing import Final

from ._errors import NativeCallError
from ._native import load_library


__all__ = [
    "Algorithm",
    "Model",
    "Options",
    "Result",
    "Status",
    "solve",
]


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


class _ErrorCode(enum.IntEnum):
    OK = 0
    INVALID_ARGUMENT = 1
    OUT_OF_RANGE = 2
    ALLOCATION_FAILED = 3
    INVALID_MODEL = 4
    INTERNAL_ERROR = 5


_UINT64_MAX: Final[int] = (1 << 64) - 1
_UINT32_MAX: Final[int] = (1 << 32) - 1
_INT64_MIN: Final[int] = -(1 << 63)
_INT64_MAX: Final[int] = (1 << 63) - 1

_library: ctypes.CDLL | None = None
_configured = False


def _load_library() -> ctypes.CDLL:
    global _library, _configured
    if _library is None:
        _library = load_library()
    if not _configured:
        _configure_library(_library)
        _configured = True
    return _library


def _configure_library(library: ctypes.CDLL) -> None:
    c_void_p_p = ctypes.POINTER(ctypes.c_void_p)
    c_int_p = ctypes.POINTER(ctypes.c_int)
    c_size_t_p = ctypes.POINTER(ctypes.c_size_t)
    c_uint32_p = ctypes.POINTER(ctypes.c_uint32)
    c_uint64 = ctypes.c_uint64
    c_int64 = ctypes.c_int64

    library.tsp_solver_model_create.argtypes = [c_void_p_p]
    library.tsp_solver_model_create.restype = ctypes.c_int
    library.tsp_solver_model_destroy.argtypes = [ctypes.c_void_p]
    library.tsp_solver_model_destroy.restype = None
    library.tsp_solver_model_add_node.argtypes = [ctypes.c_void_p, c_uint32_p]
    library.tsp_solver_model_add_node.restype = ctypes.c_int
    library.tsp_solver_model_set_distance.argtypes = [
        ctypes.c_void_p,
        ctypes.c_uint32,
        ctypes.c_uint32,
        c_int64,
    ]
    library.tsp_solver_model_set_distance.restype = ctypes.c_int
    library.tsp_solver_model_validate.argtypes = [ctypes.c_void_p]
    library.tsp_solver_model_validate.restype = ctypes.c_int

    library.tsp_solver_options_create.argtypes = [c_void_p_p]
    library.tsp_solver_options_create.restype = ctypes.c_int
    library.tsp_solver_options_destroy.argtypes = [ctypes.c_void_p]
    library.tsp_solver_options_destroy.restype = None
    library.tsp_solver_options_set_time_limit_ms.argtypes = [ctypes.c_void_p, c_uint64]
    library.tsp_solver_options_set_time_limit_ms.restype = ctypes.c_int
    library.tsp_solver_options_set_random_seed.argtypes = [ctypes.c_void_p, c_uint64]
    library.tsp_solver_options_set_random_seed.restype = ctypes.c_int
    library.tsp_solver_options_set_algorithm.argtypes = [ctypes.c_void_p, ctypes.c_int]
    library.tsp_solver_options_set_algorithm.restype = ctypes.c_int

    library.tsp_solver_solve.argtypes = [ctypes.c_void_p, ctypes.c_void_p, c_void_p_p]
    library.tsp_solver_solve.restype = ctypes.c_int

    library.tsp_solver_result_destroy.argtypes = [ctypes.c_void_p]
    library.tsp_solver_result_destroy.restype = None
    library.tsp_solver_result_get_status.argtypes = [ctypes.c_void_p, c_int_p]
    library.tsp_solver_result_get_status.restype = ctypes.c_int
    library.tsp_solver_result_get_objective.argtypes = [
        ctypes.c_void_p,
        ctypes.POINTER(ctypes.c_int64),
    ]
    library.tsp_solver_result_get_objective.restype = ctypes.c_int
    library.tsp_solver_result_get_tour_size.argtypes = [ctypes.c_void_p, c_size_t_p]
    library.tsp_solver_result_get_tour_size.restype = ctypes.c_int
    library.tsp_solver_result_get_tour.argtypes = [
        ctypes.c_void_p,
        c_uint32_p,
        ctypes.c_size_t,
        c_size_t_p,
    ]
    library.tsp_solver_result_get_tour.restype = ctypes.c_int


def _error_code(code: int) -> _ErrorCode:
    try:
        return _ErrorCode(code)
    except ValueError as exc:
        raise NativeCallError(f"Native API returned unknown error code {code}") from exc


def _raise_for_error(code: int, context: str) -> None:
    error = _error_code(code)
    if error is _ErrorCode.OK:
        return
    if error is _ErrorCode.OUT_OF_RANGE:
        raise IndexError(context)
    if error in {_ErrorCode.INVALID_ARGUMENT, _ErrorCode.INVALID_MODEL}:
        raise ValueError(context)
    raise NativeCallError(context)


def _coerce_uint64(value: int, name: str) -> int:
    if not isinstance(value, int):
        raise TypeError(f"{name} must be an integer")
    if value < 0 or value > _UINT64_MAX:
        raise ValueError(f"{name} must be between 0 and {_UINT64_MAX}")
    return value


def _coerce_int64(value: int, name: str) -> int:
    if not isinstance(value, int):
        raise TypeError(f"{name} must be an integer")
    if value < _INT64_MIN or value > _INT64_MAX:
        raise ValueError(f"{name} must be between {_INT64_MIN} and {_INT64_MAX}")
    return value


def _coerce_node_id(value: int, name: str) -> int:
    if not isinstance(value, int):
        raise TypeError(f"{name} must be an integer")
    if value < 0 or value > _UINT32_MAX:
        raise OverflowError(f"{name} must be between 0 and {_UINT32_MAX}")
    return value


class _NativeResource:
    __slots__ = ("_handle",)

    def __init__(self, handle: ctypes.c_void_p | None) -> None:
        self._handle = handle

    @property
    def handle(self) -> ctypes.c_void_p | None:
        return self._handle

    @property
    def closed(self) -> bool:
        return self._handle is None

    def close(self) -> None:
        handle = self._handle
        if handle is None:
            return
        self._destroy(handle)
        self._handle = None

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc, tb):
        self.close()
        return False

    def _destroy(self, handle: ctypes.c_void_p) -> None:
        raise NotImplementedError

    def _require_handle(self) -> ctypes.c_void_p:
        handle = self._handle
        if handle is None:
            raise RuntimeError(f"{self.__class__.__name__} has been closed")
        return handle


class Model(_NativeResource):
    def __init__(self) -> None:
        library = _load_library()
        handle = ctypes.c_void_p()
        _raise_for_error(
            library.tsp_solver_model_create(ctypes.byref(handle)),
            "failed to create model",
        )
        super().__init__(handle)

    def _destroy(self, handle: ctypes.c_void_p) -> None:
        _load_library().tsp_solver_model_destroy(handle)

    def add_node(self) -> int:
        handle = self._require_handle()
        node_id = ctypes.c_uint32()
        _raise_for_error(
            _load_library().tsp_solver_model_add_node(handle, ctypes.byref(node_id)),
            "failed to add node",
        )
        return int(node_id.value)

    def set_distance(self, from_node: int, to_node: int, distance: int) -> None:
        handle = self._require_handle()
        from_node = _coerce_node_id(from_node, "from_node")
        to_node = _coerce_node_id(to_node, "to_node")
        distance = _coerce_int64(distance, "distance")
        _raise_for_error(
            _load_library().tsp_solver_model_set_distance(
                handle,
                ctypes.c_uint32(from_node),
                ctypes.c_uint32(to_node),
                ctypes.c_int64(distance),
            ),
            "failed to set distance",
        )

    def validate(self) -> None:
        _raise_for_error(
            _load_library().tsp_solver_model_validate(self._require_handle()),
            "model is incomplete",
        )

    def solve(self, options: Options | None = None) -> Result:
        return solve(self, options)


class Options(_NativeResource):
    def __init__(self) -> None:
        library = _load_library()
        handle = ctypes.c_void_p()
        _raise_for_error(
            library.tsp_solver_options_create(ctypes.byref(handle)),
            "failed to create options",
        )
        super().__init__(handle)
        self._time_limit_ms = _UINT64_MAX
        self._random_seed = 0
        self._algorithm = Algorithm.DEFAULT

    def _destroy(self, handle: ctypes.c_void_p) -> None:
        _load_library().tsp_solver_options_destroy(handle)

    @property
    def time_limit_ms(self) -> int:
        return self._time_limit_ms

    @time_limit_ms.setter
    def time_limit_ms(self, value: int) -> None:
        handle = self._require_handle()
        value = _coerce_uint64(value, "time_limit_ms")
        _raise_for_error(
            _load_library().tsp_solver_options_set_time_limit_ms(
                handle, ctypes.c_uint64(value)
            ),
            "failed to set time limit",
        )
        self._time_limit_ms = value

    @property
    def random_seed(self) -> int:
        return self._random_seed

    @random_seed.setter
    def random_seed(self, value: int) -> None:
        handle = self._require_handle()
        value = _coerce_uint64(value, "random_seed")
        _raise_for_error(
            _load_library().tsp_solver_options_set_random_seed(
                handle, ctypes.c_uint64(value)
            ),
            "failed to set random seed",
        )
        self._random_seed = value

    @property
    def algorithm(self) -> Algorithm:
        return self._algorithm

    @algorithm.setter
    def algorithm(self, value: Algorithm | int) -> None:
        handle = self._require_handle()
        algorithm = Algorithm(value)
        _raise_for_error(
            _load_library().tsp_solver_options_set_algorithm(
                handle, ctypes.c_int(int(algorithm))
            ),
            "failed to set algorithm",
        )
        self._algorithm = algorithm


class Result(_NativeResource):
    def __init__(self, handle: ctypes.c_void_p) -> None:
        super().__init__(handle)

    @classmethod
    def _from_handle(cls, handle: ctypes.c_void_p) -> Result:
        return cls(handle)

    def _destroy(self, handle: ctypes.c_void_p) -> None:
        _load_library().tsp_solver_result_destroy(handle)

    @property
    def status(self) -> Status:
        handle = self._require_handle()
        status = ctypes.c_int()
        _raise_for_error(
            _load_library().tsp_solver_result_get_status(handle, ctypes.byref(status)),
            "failed to get result status",
        )
        return Status(status.value)

    @property
    def objective(self) -> int:
        handle = self._require_handle()
        objective = ctypes.c_int64()
        _raise_for_error(
            _load_library().tsp_solver_result_get_objective(
                handle, ctypes.byref(objective)
            ),
            "failed to get result objective",
        )
        return int(objective.value)

    @property
    def tour_size(self) -> int:
        handle = self._require_handle()
        size = ctypes.c_size_t()
        _raise_for_error(
            _load_library().tsp_solver_result_get_tour_size(handle, ctypes.byref(size)),
            "failed to get result tour size",
        )
        return int(size.value)

    @property
    def tour(self) -> tuple[int, ...]:
        handle = self._require_handle()
        size = self.tour_size
        if size == 0:
            return ()
        nodes = (ctypes.c_uint32 * size)()
        written = ctypes.c_size_t()
        _raise_for_error(
            _load_library().tsp_solver_result_get_tour(
                handle,
                nodes,
                ctypes.c_size_t(size),
                ctypes.byref(written),
            ),
            "failed to get result tour",
        )
        return tuple(int(nodes[index]) for index in range(int(written.value)))


def solve(model: Model, options: Options | None = None) -> Result:
    if not isinstance(model, Model):
        raise TypeError("model must be a Model instance")
    if options is not None and not isinstance(options, Options):
        raise TypeError("options must be an Options instance or None")

    library = _load_library()
    owned_options = None
    if options is None:
        owned_options = Options()
        options = owned_options

    result_handle = ctypes.c_void_p()
    try:
        _raise_for_error(
            library.tsp_solver_solve(
                model._require_handle(),
                options._require_handle(),
                ctypes.byref(result_handle),
            ),
            "failed to solve model",
        )
        return Result._from_handle(result_handle)
    finally:
        if owned_options is not None:
            owned_options.close()
