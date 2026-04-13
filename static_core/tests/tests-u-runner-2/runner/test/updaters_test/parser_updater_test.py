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

import tempfile
from pathlib import Path
from unittest import TestCase
from unittest.mock import MagicMock

from runner.extensions.updaters.parser.parser_updater import ParserUpdater
from runner.options.config import Config
from runner.options.options_step import StepKind
from runner.types.step_report import StepReport


class ParserUpdaterTest(TestCase):
    """Tests for ParserUpdater.process()."""

    @staticmethod
    def _make_config() -> MagicMock:
        return MagicMock(spec=Config)

    @staticmethod
    def _make_compiler_report(output: str) -> StepReport:
        return StepReport(
            name="es2panda",
            step_kind=StepKind.COMPILER,
            command_line="es2panda test.ets",
            cmd_output=output,
        )

    @staticmethod
    def _make_runtime_report(output: str) -> StepReport:
        return StepReport(
            name="ark",
            step_kind=StepKind.RUNTIME,
            command_line="ark test.ets",
            cmd_output=output,
        )

    # ------------------------------------------------------------------
    # Happy-path: expected file is written
    # ------------------------------------------------------------------

    def test_process_writes_expected_file(self) -> None:
        """
        When a COMPILER step report is present, process() must create
        <stem>-expected.txt next to the test file with the normalized output.
        """
        with tempfile.TemporaryDirectory() as tmp_dir:
            tmp = Path(tmp_dir)
            test_file = tmp / "mytest.ets"
            test_file.touch()

            compiler_output = "SyntaxError: unexpected token\n"
            report = self._make_compiler_report(compiler_output)

            updater = ParserUpdater(self._make_config())
            updater.process(test_file, [report])

            expected_path = tmp / "mytest-expected.txt"
            self.assertTrue(expected_path.exists(), "Expected file was not created")
            actual_content = expected_path.read_text(encoding="utf-8")
            self.assertEqual(compiler_output, actual_content)

    def test_process_normalizes_build_system_path(self) -> None:
        """
        Absolute paths in 'At File:' lines must be stripped to just the
        filename:line:col part before writing the expected file.
        """
        with tempfile.TemporaryDirectory() as tmp_dir:
            tmp = Path(tmp_dir)
            test_file = tmp / "diagnostic.ets"
            test_file.touch()

            raw_output = (
                "1 ERROR: Semantic error ESE0318\n"
                "Error Message: Type '\"string\"' cannot be assigned to type 'Double' "
                "At File: /home/user/project/ets2panda/test/diagnostic.ets:21:21"
            )
            expected_content = (
                "1 ERROR: Semantic error ESE0318\n"
                "Error Message: Type '\"string\"' cannot be assigned to type 'Double' "
                "At File: diagnostic.ets:21:21"
            )

            report = self._make_compiler_report(raw_output)
            updater = ParserUpdater(self._make_config())
            updater.process(test_file, [report])

            expected_path = tmp / "diagnostic-expected.txt"
            actual_content = expected_path.read_text(encoding="utf-8")
            self.assertEqual(expected_content, actual_content)

    def test_process_uses_first_compiler_report_when_multiple_present(self) -> None:
        """
        When multiple COMPILER step reports exist, only the first one is
        used to generate the expected file.
        """
        with tempfile.TemporaryDirectory() as tmp_dir:
            tmp = Path(tmp_dir)
            test_file = tmp / "multi.ets"
            test_file.touch()

            first_output = "first compiler output\n"
            second_output = "second compiler output\n"
            reports = [
                self._make_compiler_report(first_output),
                self._make_compiler_report(second_output),
            ]

            updater = ParserUpdater(self._make_config())
            updater.process(test_file, reports)

            expected_path = tmp / "multi-expected.txt"
            actual_content = expected_path.read_text(encoding="utf-8")
            self.assertEqual(first_output, actual_content)

    def test_process_overwrites_existing_expected_file(self) -> None:
        """
        If the expected file already exists, process() must overwrite it
        with the new content.
        """
        with tempfile.TemporaryDirectory() as tmp_dir:
            tmp = Path(tmp_dir)
            test_file = tmp / "overwrite.ets"
            test_file.touch()

            expected_path = tmp / "overwrite-expected.txt"
            expected_path.write_text("old content", encoding="utf-8")

            new_output = "new compiler output\n"
            report = self._make_compiler_report(new_output)

            updater = ParserUpdater(self._make_config())
            updater.process(test_file, [report])

            actual_content = expected_path.read_text(encoding="utf-8")
            self.assertEqual(new_output, actual_content)

    def test_process_writes_with_unix_line_endings(self) -> None:
        """
        The expected file must always be written with Unix line endings (\\n),
        regardless of the platform.
        """
        with tempfile.TemporaryDirectory() as tmp_dir:
            tmp = Path(tmp_dir)
            test_file = tmp / "lineend.ets"
            test_file.touch()

            output = "line1\nline2\nline3\n"
            report = self._make_compiler_report(output)

            updater = ParserUpdater(self._make_config())
            updater.process(test_file, [report])

            expected_path = tmp / "lineend-expected.txt"
            raw_bytes = expected_path.read_bytes()
            self.assertNotIn(b"\r\n", raw_bytes, "File must not contain Windows line endings")

    # ------------------------------------------------------------------
    # Edge cases: no COMPILER report
    # ------------------------------------------------------------------

    def test_process_does_nothing_when_no_reports(self) -> None:
        """
        When the data list is empty, process() must not create any file.
        """
        with tempfile.TemporaryDirectory() as tmp_dir:
            tmp = Path(tmp_dir)
            test_file = tmp / "empty.ets"
            test_file.touch()

            updater = ParserUpdater(self._make_config())
            updater.process(test_file, [])

            expected_path = tmp / "empty-expected.txt"
            self.assertFalse(expected_path.exists(), "No file should be created when data is empty")

    def test_process_does_nothing_when_only_runtime_reports(self) -> None:
        """
        When there are no COMPILER step reports (only RUNTIME), process()
        must not create any file.
        """
        with tempfile.TemporaryDirectory() as tmp_dir:
            tmp = Path(tmp_dir)
            test_file = tmp / "runtime_only.ets"
            test_file.touch()

            report = self._make_runtime_report("some runtime output")
            updater = ParserUpdater(self._make_config())
            updater.process(test_file, [report])

            expected_path = tmp / "runtime_only-expected.txt"
            self.assertFalse(expected_path.exists(), "No file should be created without a COMPILER report")

    def test_process_ignores_use_metadata_flag(self) -> None:
        """
        ParserUpdater ignores the third positional parameter (use_metadata);
        the expected file should be written regardless of its value.
        """
        with tempfile.TemporaryDirectory() as tmp_dir:
            tmp = Path(tmp_dir)
            test_file = tmp / "meta.ets"
            test_file.touch()

            output = "compiler output\n"
            report = self._make_compiler_report(output)

            updater = ParserUpdater(self._make_config())
            # ParserUpdater.process() uses `_` (unnamed) for the third param,
            # so it must be passed positionally, not as a keyword argument.
            updater.process(test_file, [report], True)

            expected_path = tmp / "meta-expected.txt"
            self.assertTrue(expected_path.exists())
            self.assertEqual(output, expected_path.read_text(encoding="utf-8"))

    def test_process_empty_compiler_output_writes_empty_file(self) -> None:
        """
        When the compiler output is an empty string, the expected file
        should be created and contain an empty string.
        """
        with tempfile.TemporaryDirectory() as tmp_dir:
            tmp = Path(tmp_dir)
            test_file = tmp / "empty_output.ets"
            test_file.touch()

            report = self._make_compiler_report("")
            updater = ParserUpdater(self._make_config())
            updater.process(test_file, [report])

            expected_path = tmp / "empty_output-expected.txt"
            self.assertTrue(expected_path.exists())
            self.assertEqual("", expected_path.read_text(encoding="utf-8"))
