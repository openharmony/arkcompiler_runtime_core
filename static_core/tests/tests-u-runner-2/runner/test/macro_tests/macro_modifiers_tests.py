#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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

from unittest import TestCase

from runner.options.macros_modifiers import ExpandedValueType, MacroModifier


class MacroModifiersTests(TestCase):
    def test_is_path(self) -> None:
        test_cases: list[tuple[ExpandedValueType, bool]] = [
            ("test.txt", True),
            ("example", True),
            ("/path/to/test.txt", True),
            (r"C:\folder\example.py", True),
            ("src/file.ets", True),
            ("a/b/c/d/final.abc", True),
            ("not a file at all", True),
            ("first\nsecond", False),
            (True, False),
            (1, False),
            (["hello"], False),
            ({"a": "b"}, False),
        ]

        for source, expected in test_cases:
            with self.subTest(source=source):
                actual = MacroModifier.is_path(source)
                self.assertEqual(expected, actual)

    def test_take_name(self) -> None:
        test_cases: list[tuple[str, str]] = [
            ("test.txt", "test.txt"),
            ("example", "example"),
            ("/path/to/test.txt", "test.txt"),
            (r"C:\folder\example.py", "example.py"),
            ("src/file.ets", "file.ets"),
            ("a/b/c/d/final.abc", "final.abc"),
            ("not a file at all", "not a file at all"),
        ]

        for source, expected in test_cases:
            with self.subTest(source=source):
                actual = MacroModifier.take_name(source)
                self.assertEqual(expected, actual)

    def test_take_stem(self) -> None:
        test_cases = [
            ("test.txt", "test"),
            ("example", "example"),
            ("/path/to/test.txt", "test"),
            ("C:\\folder\\example.py", "example"),
            ("src/file.ets", "file"),
            ("path/to/archive.tar.gz", "archive.tar"),
            ("R.E.A.D.M.E", "R.E.A.D.M"),
        ]

        for source, expected in test_cases:
            with self.subTest(source=source):
                actual = MacroModifier.take_stem(source)
                self.assertEqual(expected, actual)

    def test_take_parent(self) -> None:
        test_cases = [
            ("test.txt", "."),
            ("example", "."),
            ("/path/to/test.txt", "/path/to"),
            (r"C:\folder\example.py", "C:/folder"),
            ("src/file.ets", "src"),
            ("a/b/c/d/final.abc", "a/b/c/d"),
            ("/file.txt", "/"),
        ]

        for source, expected in test_cases:
            with self.subTest(source=source):
                actual = MacroModifier.take_parent(source)
                self.assertEqual(expected, actual)
