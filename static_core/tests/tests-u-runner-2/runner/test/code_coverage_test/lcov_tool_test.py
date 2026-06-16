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

import shutil
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from typing import cast
from unittest.mock import patch

from runner.code_coverage.cmd_executor import LinuxCmdExecutor
from runner.code_coverage.coverage_dir import CoverageDir
from runner.code_coverage.lcov_tool import LCOV_IGNORE_ERROR, LcovTool
from runner.options.options_general import GeneralOptions


class LcovToolTest(unittest.TestCase):
    def setUp(self) -> None:
        self.work_dir = Path(tempfile.mkdtemp())
        self.build_dir = self.work_dir / "build"
        general = SimpleNamespace(coverage=SimpleNamespace(
            coverage_html_report_dir=None,
            profdata_files_dir=None,
        ))
        self.coverage_dir = CoverageDir(cast(GeneralOptions, general), self.work_dir)
        self.tool = LcovTool(self.build_dir, self.coverage_dir, LinuxCmdExecutor(), gcov_tool=None)

    def tearDown(self) -> None:
        shutil.rmtree(self.work_dir, ignore_errors=True)

    def test_binaries_and_paths_resolved_in_init(self) -> None:
        self.assertEqual(self.tool.lcov_binary, "lcov")
        self.assertEqual(self.tool.genhtml_binary, "genhtml")
        self.assertEqual(self.tool.bin_dir, self.build_dir / "bin")
        self.assertEqual(self.tool.dot_info_files_dir, self.coverage_dir.coverage_work_dir / "dot_info_files_dir")
        self.assertEqual(self.tool.components, {})

    def test_ignore_errors_is_joined_list(self) -> None:
        self.assertEqual(self.tool.ignore_errors, ",".join(LCOV_IGNORE_ERROR))

    def test_get_gcov_prefix(self) -> None:
        result = self.tool.get_gcov_prefix("ets")
        expected_prefix = str(self.tool.dot_info_files_dir / "ets")
        expected_strip = str(len(self.build_dir.parts) - 1)
        self.assertEqual(result, [expected_prefix, expected_strip])
        self.assertIn("ets", self.tool.components)

    def test_clear_gcda_files_invokes_lcov_zerocounters(self) -> None:
        with patch.object(self.tool.cmd_executor, "run_command") as mock_run:
            self.tool.clear_gcda_files()
        mock_run.assert_called_once_with(
            ["lcov", "--zerocounters", "--directory", str(self.build_dir)])


if __name__ == "__main__":
    unittest.main()
