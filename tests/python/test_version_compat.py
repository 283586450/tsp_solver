from __future__ import annotations

import os
import sys
from pathlib import Path
import unittest
from unittest import mock


ROOT = Path(__file__).resolve().parents[2]
PYTHON_BINDINGS = ROOT / "bindings" / "python"
if str(PYTHON_BINDINGS) not in sys.path:
    sys.path.insert(0, str(PYTHON_BINDINGS))


from tsp_solver import _native
from tsp_solver._errors import VersionMismatchError


class VersionCompatibilityTest(unittest.TestCase):
    def test_python_package_and_native_versions_must_match(self) -> None:
        with self.assertRaisesRegex(
            VersionMismatchError,
            r"Python package version 0\.1\.0 does not match native library version 9\.9\.9.*same tsp_solver release",
        ):
            _native._ensure_version_match("0.1.0", "9.9.9")

    def test_load_library_rejects_runtime_version_mismatch(self) -> None:
        if "TSP_SOLVER_LIBRARY_PATH" not in os.environ:
            self.skipTest("runtime library path is not configured")

        previous_library = _native._library
        try:
            _native._library = None
            with mock.patch.object(_native, "package_version", return_value="9.9.9"):
                with self.assertRaisesRegex(
                    VersionMismatchError,
                    r"Python package version 9\.9\.9 does not match native library version 0\.1\.0.*same tsp_solver release",
                ):
                    _native.load_library()
        finally:
            _native._library = previous_library


if __name__ == "__main__":
    unittest.main()
