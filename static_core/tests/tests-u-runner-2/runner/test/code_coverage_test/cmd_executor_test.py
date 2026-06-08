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

import subprocess
import unittest
from unittest.mock import MagicMock, patch

from runner.code_coverage.cmd_executor import LinuxCmdExecutor


class LinuxCmdExecutorGetBinaryTest(unittest.TestCase):
    def setUp(self) -> None:
        self.executor = LinuxCmdExecutor()

    def test_get_binary_without_version(self) -> None:
        self.assertEqual(self.executor.get_binary("lcov"), "lcov")

    def test_get_binary_with_none_version(self) -> None:
        self.assertEqual(self.executor.get_binary("genhtml", None), "genhtml")

    def test_get_binary_with_version(self) -> None:
        self.assertEqual(self.executor.get_binary("lcov", "14"), "lcov-14")


class LinuxCmdExecutorRunCommandTest(unittest.TestCase):
    def setUp(self) -> None:
        self.executor = LinuxCmdExecutor()

    @patch("runner.code_coverage.cmd_executor.subprocess.run")
    def test_run_command_returns_completed_process(self, mock_run: MagicMock) -> None:
        sentinel: subprocess.CompletedProcess[str] = subprocess.CompletedProcess(args=["echo"], returncode=0)
        mock_run.return_value = sentinel
        result = self.executor.run_command(["echo", "hi"])
        self.assertIs(result, sentinel)
        # check=True and text=True are always passed
        _, kwargs = mock_run.call_args
        self.assertTrue(kwargs["check"])
        self.assertTrue(kwargs["text"])

    @patch("runner.code_coverage.cmd_executor.subprocess.run")
    def test_run_command_returns_none_on_called_process_error(self, mock_run: MagicMock) -> None:
        mock_run.side_effect = subprocess.CalledProcessError(returncode=1, cmd=["bad"], stderr="boom")
        self.assertIsNone(self.executor.run_command(["bad"]))


if __name__ == "__main__":
    unittest.main()
