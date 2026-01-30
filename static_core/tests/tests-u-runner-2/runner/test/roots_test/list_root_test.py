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
import os
from pathlib import Path
from typing import ClassVar
from unittest import TestCase
from unittest.mock import patch

from runner.common_exceptions import InvalidConfiguration
from runner.options.options import IOptions
from runner.options.root_dir import RootDir
from runner.test.test_utils import test_environ


class ListRootTest(TestCase):
    default_name = "default_name"
    options: ClassVar[IOptions] = IOptions()
    default_list_roots: ClassVar[list[RootDir]] = [RootDir(Path.cwd() / "default", default_name, options)]

    def test_from_list(self) -> None:
        """
        Feature: load test list information from yaml list
        Expected:
        - 2 list roots, the first root with custom name, the second one with default name
        """
        data = {
            'test-root': Path.cwd().as_posix(),
            'list-roots': [
                {'root': (Path.cwd() / "a").as_posix(), 'name': 'new'},
                (Path.cwd() / "b").as_posix()
            ]
        }
        actual = RootDir.get_list_roots(data, self.options, self.default_name, self.default_list_roots)
        self.assertIsInstance(actual, list, f"Expected list of roots. Got {actual}")
        self.assertEqual(2, len(actual), f"Expected 2 list roots. Got {len(actual)}")
        first: RootDir = actual[0]
        self.assertEqual("a", first.root_dir.name)
        self.assertEqual("new", first.name)
        second: RootDir = actual[1]
        self.assertEqual("b", second.root_dir.name)
        self.assertEqual(self.default_name, second.name)

    def test_from_str(self) -> None:
        """
        Feature: load test list information from yaml str
        Expected:
        - 1 list root with default name
        """
        data = {
            'test-root': Path.cwd().as_posix(),
            'list-root': (Path.cwd() / "b").as_posix()
        }
        actual = RootDir.get_list_roots(data, self.options, self.default_name, self.default_list_roots)
        self.assertIsInstance(actual, list, f"Expected list of roots. Got {actual}")
        self.assertEqual(1, len(actual), f"Expected 1 list root. Got {len(actual)}")
        first: RootDir = actual[0]
        self.assertEqual("b", first.root_dir.name)
        self.assertEqual(self.default_name, first.name)

    def test_from_one_obj(self) -> None:
        """
        Feature: load test list information from yaml one dict item
        Expected:
        - 1 list root with custom name
        """
        data = {
            'test-root': Path.cwd().as_posix(),
            'list-root': {'root': (Path.cwd() / "b").as_posix(), 'name': 'new'}
        }
        actual = RootDir.get_list_roots(data, self.options, self.default_name, self.default_list_roots)
        self.assertIsInstance(actual, list, f"Expected list of roots. Got {actual}")
        self.assertEqual(1, len(actual), f"Expected 1 list root. Got {len(actual)}")
        first: RootDir = actual[0]
        self.assertEqual("b", first.root_dir.name)
        self.assertEqual('new', first.name)

    def test_from_no_name(self) -> None:
        """
        Feature: load test list information from yaml one dict item
        Expected:
        - 1 list root with custom name
        """
        data = {
            'test-root': Path.cwd().as_posix(),
            'list-root': {'root': (Path.cwd() / "b").as_posix()}
        }
        with self.assertRaises(InvalidConfiguration) as ex:
            RootDir.get_list_roots(data, self.options, self.default_name, self.default_list_roots)
        self.assertIn('Required field is absent.', str(ex.exception))

    @patch.dict(os.environ, test_environ(), clear=True)
    def test_from_yaml_with_macro(self) -> None:
        """
        Feature: load list roots with macro from yaml
        Expected:
        - list roots are correctly loaded, macros are expanded
        """
        data = {
            'list-roots': [
                "${PANDA_BUILD}/a",
                {'root': "${PANDA_BUILD}/b", 'name': "new"}
            ]
        }
        actual = RootDir.get_list_roots(data, self.options, self.default_name, self.default_list_roots)
        self.assertIsInstance(actual, list, f"Expected list of roots. Got {actual}")
        self.assertEqual(2, len(actual), f"Expected 2 list roots. Got {len(actual)}")

        first: RootDir = actual[0]
        expected_first = (Path.cwd() / "a").as_posix()
        self.assertEqual(expected_first, first.root_dir.as_posix())
        self.assertEqual(self.default_name, first.name)

        second: RootDir = actual[1]
        expected_second = (Path.cwd() / "b").as_posix()
        self.assertEqual(expected_second, second.root_dir.as_posix())
        self.assertEqual("new", second.name)
