#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

"""
check-venv.py - Virtual environment dependency checker.

This script verifies that all Python packages listed in a requirements file
are installed in the current environment with versions satisfying the
specified constraints.

Usage:
    python check-venv.py [-v] <requirements_file>

Arguments:
    requirements_file   Path to a pip-style requirements file to validate.
    -v, --verbose       Print each satisfied dependency explicitly.

Exit codes:
    0 - All dependencies are satisfied.
    1 - One or more dependencies are missing or have a version conflict.
"""

import argparse
import sys
from importlib.metadata import PackageNotFoundError
from importlib.metadata import version as pkg_version
from pathlib import Path

from packaging.requirements import Requirement

RED = '\033[31m'
YELLOW = '\033[33m'
RESET = "\033[0m"


def check_req_file(req_file: str, verbose: bool) -> bool:
    """Check that all dependencies listed in a requirements file are satisfied.

    Reads each non-empty, non-comment line from ``req_file`` as a PEP 508
    dependency specifier, then verifies that the package is installed and
    its version matches any version constraints declared in the specifier.

    Args:
        req_file: Path to the requirements file to check.
        verbose:  When ``True``, print a confirmation line for every
                  dependency that is found and satisfies its constraints.

    Returns:
        ``True`` if every listed dependency is present and satisfies the
        declared version specifier;
        ``False`` otherwise.
    """
    with open(req_file, encoding="utf-8") as f:
        dependencies = [line.strip() for line in f if line.strip() and not line.startswith("#")]

    if verbose:
        print(f"Checking {len(dependencies)} dependencies from '{req_file}'...")

    all_ok = True
    for dep in dependencies:
        req = Requirement(dep)
        try:
            installed = pkg_version(req.name)
            if req.specifier and not req.specifier.contains(installed, prereleases=True):
                print(
                    f"ERROR: {YELLOW}Version conflict{RESET} - {req.name} {installed} does not satisfy {req.specifier}",
                    file=sys.stderr,
                )
                all_ok = False
            elif verbose:
                print(f"  OK: {req.name} {installed}")
        except PackageNotFoundError:
            print(f"ERROR: {RED}Missing package{RESET} - {req.name}", file=sys.stderr)
            all_ok = False

    if all_ok:
        print("All dependencies are satisfied.")
    else:
        curr_dir = Path(__file__).parent.parent / "install-deps-ubuntu"
        print("To install missing packages, run:\n"
              f"  sudo {curr_dir.resolve().as_posix()} -i=test")
    return all_ok


def get_args() -> argparse.Namespace:
    """Parse and return command-line arguments.

    Defines two arguments:
    * ``-v`` / ``--verbose`` (optional flag) - enable verbose output.
    * ``requirements_file`` (positional, required) - path to the
      requirements file to validate.

    Returns:
        A :class:`argparse.Namespace` object with attributes
        ``verbose`` (bool) and ``requirements_file`` (str).
    """
    parser = argparse.ArgumentParser(
        description="Check if all required Python packages are installed with correct versions."
    )
    parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Enable verbose output"
    )
    parser.add_argument(
        "requirements_file",
        help="Path to the requirements file to check"
    )
    return parser.parse_args()


def main() -> None:
    """Entry point of the script.

    Retrieves parsed CLI arguments via :func:`get_args`,
    delegates the actual dependency checking to :func:`check_req_file`,
    and exits with code ``0`` on success or ``1`` if any dependency check fails.
    """
    args = get_args()
    success = check_req_file(args.requirements_file, args.verbose)
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
