#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

from dataclasses import dataclass
from pathlib import Path
import subprocess
import argparse
import os.path as ospath
from typing import List

from utils.file_structure import walk_test_subdirs
from utils.metainformation import InvalidMetaException, find_all_metas
from utils.exceptions import InvalidFileFormatException, InvalidFileStructureException
from utils.constants import JAR_EXTENSION, TEMPLATE_EXTENSION, NEGATIVE_PREFIX, NEGATIVE_EXECUTION_PREFIX


# =======================================================
# Test diagnostics


def load_test_meta(testpath: Path) -> dict:
    testpath = str(testpath)
    # NOTE: move to fsutils
    with open(testpath, "r") as f:
        test = f.read()

    try:
        metas = find_all_metas(test)
    except InvalidMetaException as e:
        raise InvalidFileFormatException(filepath=testpath, message=f"ERROR: {str(e)}")

    if len(metas) == 0:
        raise InvalidFileFormatException(filepath=testpath, message="No meta found in file")
    elif len(metas) > 1:
        raise InvalidFileFormatException(filepath=testpath, message="Multiple meta found in file")
    else:
        _, _, meta = metas[0]
        return meta    


def print_test_failure(filepath: Path):
    print("\nFAIL:", filepath)
    try:
        meta = load_test_meta(filepath)
        for key in meta.keys():
            print(f"\t{key}: {meta[key]}")
    except InvalidFileFormatException as e:
        print(f"\tERROR: Could not load test's meta: {str(e)}")


# =======================================================
# Run tests


@dataclass
class RunStats:
    """
    Test execution statistics 
    """
    failed: List[Path]
    passed: List[Path]

    def __init__(self):
        self.failed = []
        self.passed = [] 
    
    def record_failure(self, path: Path):
        self.failed.append(path)

    def record_success(self, path: Path):
        self.passed.append(path)

def test_should_pass(filepath: Path) -> bool:
    return not (filepath.name.startswith(NEGATIVE_PREFIX) or filepath.name.startswith(NEGATIVE_EXECUTION_PREFIX))

def run_test(filepath: Path, runnerpath: Path) -> bool:
    """
    Runs the tests and returns True if it passed.
    Takes into account the test's type (positive or negative)
    """
    # NOTE: Make a separate entity describes process of running the test.
    # NOTE: Each instance or children (depends on design)
    # NOTE: have to the method 'run' with (status, message, payload) as result
    code = subprocess.run(
        ["java", "-jar", str(runnerpath), "-T", str(filepath)],
        stderr=subprocess.DEVNULL,
        stdout=subprocess.DEVNULL
    ).returncode

    passed = (code == 0)

    return passed == test_should_pass(filepath)


def run_tests(root: Path, runnerpath: Path) -> bool:
    """
    Runs all tests in a test-suite 
    """
    stats = RunStats()

    for directory in list(walk_test_subdirs(root)):
        for filepath in directory.iter_files():
            if ospath.isdir(filepath):
                continue
            if run_test(filepath, runnerpath):
                stats.record_success(filepath)
            else:
                stats.record_failure(filepath)
                print_test_failure(filepath)
    
    return stats


# =======================================================
# Entry 


parser = argparse.ArgumentParser(description='Run CTS tests.')
parser.add_argument('tests_dir', help='Path to directory that contains the tests')
parser.add_argument('runner_tool_path',
                     help='Path to runner tool. Available runners: 1) migration tool, specify path to jar')


def main():
    args = parser.parse_args()

    testpath = Path(args.tests_dir)
    runnerpath = Path(args.runner_tool_path)

    # Verify that CTS dir exists  
    if not testpath.is_dir():
        print(f"ERROR: Path '{testpath}' must be a directory")
        exit(1)
    
    # Verify runner tool exists
    path = Path.cwd() / runnerpath
    if not path.is_file() or path.suffix != JAR_EXTENSION:
        print(f"ERROR: Path '{runnerpath}' must point to a runner, e.g. migration tool")
        exit(1)

    try:
        stats = run_tests(testpath, runnerpath)
    except (InvalidFileFormatException, InvalidFileStructureException) as e:
        print("ERROR:\n", e.message)
        exit(1)

    print("\n=====")
    print(f"TESTS {len(stats.failed) + len(stats.passed)} | SUCCESS {len(stats.passed)} | FAILED {len(stats.failed)}\n")

    if len(stats.failed) > 0:
        print("FAIL!")
        exit(1)
    else:
        print("SUCCESS!")


if __name__ == "__main__":
    main()      
