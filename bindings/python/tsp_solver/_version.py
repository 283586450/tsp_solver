from __future__ import annotations

from importlib.metadata import PackageNotFoundError, version
from pathlib import Path
import re


def package_version() -> str:
    try:
        return version("tsp-solver")
    except PackageNotFoundError:
        root = Path(__file__).resolve().parents[3]
        content = (root / "CMakeLists.txt").read_text(encoding="utf-8")
        match = re.search(
            r"project\(tsp_solver VERSION ([0-9]+\.[0-9]+\.[0-9]+)", content
        )
        if match is None:
            raise RuntimeError("Unable to determine tsp_solver package version")
        return match.group(1)
