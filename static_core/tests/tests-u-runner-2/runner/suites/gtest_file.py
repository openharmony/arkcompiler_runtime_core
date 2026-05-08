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

import re
from dataclasses import dataclass
from pathlib import Path
from typing import Optional


@dataclass
class GTestFile:
    """
    Represents a parsed GTest path.

    This class parses and reconstructs GTest paths with the following format:
        test_file_name/[suite_name][/case_name][.test_name][/test_param]

    Components:
        - test_file_name: The source file name (alphanumeric + underscore)
        - suite_name: Test suite name (starts with uppercase, contains 'TestSuite')
        - case_name: Test case name (starts with uppercase)
        - test_name: Test name (alphanumeric + underscore + dot)
        - test_param: Parameterized test index (digits)

    Example paths:
        - "my_test_file"
        - "my_test_file/MyTestSuite"
        - "my_test_file/MyTestSuite/TestCase"
        - "my_test_file/MyTestSuite/TestCase.TestName"
        - "my_test_file/MyTestSuite/TestCase.TestName/0"

    Attributes:
        test_file_name (str): The base test file name.
        suite_name (str | None): Optional test suite name.
        case_name (str | None): Optional test case name.
        test_name (str | None): Optional test name.
        test_param (int | None): Optional parameterized test index.
    """

    test_file_name: str
    suite_name: str | None = None
    case_name: str | None = None
    test_name: str | None = None
    test_param: int | None = None

    # Regular expression for parsing gtest paths.
    # Format: test_file_name[/suite_name][/case_name][.test_name][/test_param]
    # where:
    #   test_file_name: alphanumeric + underscore
    #   suite_name: starts with uppercase, contains 'TestSuite'
    #   case_name: starts with uppercase
    #   test_name: alphanumeric + underscore + dot
    #   test_param: digits
    _TEST_FILE_RE = re.compile(
        r"^(?P<test_file_name>[A-Za-z0-9_]+)"
        r"(?:/(?P<suite_name>[A-Z][A-Za-z0-9_]*TestSuite[A-Za-z0-9_]*))?"
        r"(?:/(?P<case_name>[A-Z][A-Za-z0-9_]*))?"
        r"(?:\.(?P<test_name>[A-Za-z0-9_.]+))?"
        r"(?:/(?P<test_param>[0-9]+))?$"
    )

    def __str__(self) -> str:
        """
        Return a string representation of this GTestFile.

        Returns:
            str: A string in the format "GTestFile(<path>)" where <path> is
                 the reconstructed path from parsed components.
        """
        return f"GTestFile({self.get_path()})"

    @classmethod
    def parse_from_path(cls, path: str) -> Optional["GTestFile"]:
        """
        Parse a GTest path string and create a GTestFile instance.

        This method extracts the individual components (file name, suite, case,
        test name, and parameter) from a gtest path string using regex matching.

        Args:
            path (str): The gtest path string to parse. Expected format:
                        test_file_name[/suite_name][/case_name][.test_name][/test_param]

        Returns:
            GTestFile | None: A new GTestFile instance if parsing succeeds,
                              None if the path format is invalid.

        Example:
            >>> GTestFile.parse_from_path("my_test/MySuite/MyCase.Test/0")
            GTestFile(my_test/MySuite/MyCase.Test/0)
        """
        match = cls._TEST_FILE_RE.match(path)
        if not match:
            return None

        groups = match.groupdict()
        return cls(
            test_file_name=groups["test_file_name"],
            suite_name=groups.get("suite_name"),
            case_name=groups.get("case_name"),
            test_name=groups.get("test_name"),
            test_param=int(groups["test_param"]) if groups.get("test_param") else None,
        )

    def get_test_name(self) -> str:
        """
        Return the test name component, possibly including parameter.

        Returns the test name portion of the path, combining it with the
        parameter index if both are present.

        Returns:
            str: The test name with optional parameter suffix (e.g., "TestName"
                 or "TestName/0"). Returns empty string if no test name or
                 parameter is set.

        Example:
            >>> gtest = GTestFile.parse_from_path("test/Suite/Case.Test/5")
            >>> gtest.get_test_name()
            'Test/5'
        """
        if self.test_name is not None and self.test_param is not None:
            return f"{self.test_name}/{self.test_param}"
        if self.test_name is not None:
            return self.test_name
        if self.test_param is not None:
            return str(self.test_param)
        return ""

    def get_test_case_name(self) -> str:
        """
        Return the test case component (suite/case).

        Returns the suite and/or case portion of the path, combining them
        with a slash separator if both are present.

        Returns:
            str: The suite and case components (e.g., "Suite/Case", "Suite",
                 or "Case"). Returns empty string if neither is set.

        Example:
            >>> gtest = GTestFile.parse_from_path("test/MySuite/MyCase.Test")
            >>> gtest.get_test_case_name()
            'MySuite/MyCase'
        """
        if self.suite_name is not None and self.case_name is not None:
            return f"{self.suite_name}/{self.case_name}"
        if self.suite_name is not None:
            return self.suite_name
        if self.case_name is not None:
            return self.case_name
        return ""

    def get_path(self) -> Path:
        """
        Reconstruct the relative path from parsed components.

        Builds a complete path by combining the file name with optional
        test case and test name components in the correct order.

        Returns:
            Path: A pathlib.Path object representing the reconstructed
                  relative path (e.g., "test_file/Suite/Case.Test/0").

        Example:
            >>> gtest = GTestFile.parse_from_path("test/MySuite/MyCase.Test/0")
            >>> gtest.get_path()
            PosixPath('test/MySuite/MyCase.Test/0')
        """
        path = Path(self.test_file_name)
        test_case = self.get_test_case_name()
        if test_case:
            path = path.joinpath(test_case)
        test_name = self.get_test_name()
        if test_name:
            path = path.joinpath(test_name)
        return path
