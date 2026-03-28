from __future__ import annotations

import os
from pathlib import Path
import re
import shutil

from distutils.errors import DistutilsSetupError
from setuptools import setup
from setuptools.command.build_py import build_py as _build_py

try:
    from wheel.bdist_wheel import bdist_wheel as _bdist_wheel
except ImportError:  # pragma: no cover - wheel is present in build backends
    _bdist_wheel = None


ROOT = Path(__file__).resolve().parents[2]
PACKAGE_NAME = "tsp_solver"
ENV_NATIVE_LIBRARY = "TSP_SOLVER_PYTHON_NATIVE_LIBRARY"


def project_version() -> str:
    match = re.search(
        r"project\(tsp_solver VERSION ([0-9]+\.[0-9]+\.[0-9]+)",
        (ROOT / "CMakeLists.txt").read_text(encoding="utf-8"),
    )
    if match is None:
        raise DistutilsSetupError(
            "Unable to determine tsp_solver version from CMakeLists.txt"
        )
    return match.group(1)


class build_py(_build_py):
    def run(self) -> None:
        super().run()

        native_library = os.environ.get(ENV_NATIVE_LIBRARY)
        if not native_library:
            raise DistutilsSetupError(
                f"{ENV_NATIVE_LIBRARY} must point to the built native tsp_solver library"
            )

        source = Path(native_library)
        if not source.is_file():
            raise DistutilsSetupError(f"Native library not found: {source}")

        target = Path(self.build_lib) / PACKAGE_NAME / source.name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)


if _bdist_wheel is not None:

    class bdist_wheel(_bdist_wheel):
        def finalize_options(self) -> None:
            super().finalize_options()
            self.root_is_pure = False
else:  # pragma: no cover - only used if wheel is unavailable
    bdist_wheel = None


setup(
    version=project_version(),
    cmdclass={
        "build_py": build_py,
        **({"bdist_wheel": bdist_wheel} if bdist_wheel is not None else {}),
    },
    zip_safe=False,
)
