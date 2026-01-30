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
from unittest import TestCase
from unittest.mock import patch

from runner.common_exceptions import InvalidConfiguration
from runner.options.options import IOptions
from runner.options.root_dir import RootDir
from runner.test.test_utils import test_environ


class TestRootTest(TestCase):
    options: IOptions = IOptions()

    def test_from_yaml(self) -> None:
        """
        Feature: load test root information from yaml
        Expected:
        - test root is correctly loaded
        """
        data = {
            'test-root': (Path.cwd() / "a").as_posix(),
        }
        actual = RootDir.get_test_root(data, self.options)
        self.assertEqual("a", actual().name)

    @patch.dict(os.environ, test_environ(), clear=True)
    def test_from_yaml_with_macro(self) -> None:
        """
        Feature: load test root with macro from yaml
        Expected:
        - test root is correctly loaded, macro is expanded
        """
        data = {
            'test-root': "${PANDA_BUILD}/a"
        }
        actual = RootDir.get_test_root(data, self.options)
        expected_path = (Path.cwd() / "a").as_posix()
        self.assertEqual(expected_path, actual().as_posix())

    def test_from_no_root(self) -> None:
        """
        Feature: no test root information is provided in the yaml
        Expected:
        - exception InvalidConfiguration
        """
        data: dict = {}
        with self.assertRaises(InvalidConfiguration) as ex:
            RootDir.get_test_root(data, self.options)
        self.assertIn("The key 'test-root' should be set.", str(ex.exception))
