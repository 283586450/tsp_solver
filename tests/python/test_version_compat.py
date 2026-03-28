from __future__ import annotations

import sys
from pathlib import Path
import unittest


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
            r"Python package version 0\.1\.0 does not match native library version 9\.9\.9",
        ):
            _native._ensure_version_match("0.1.0", "9.9.9")


if __name__ == "__main__":
    unittest.main()
