#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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

import io
import tempfile
from pathlib import Path
from typing import cast
from unittest import TestCase
from unittest.mock import patch

from runner.extensions.validators.srcdumper.strip_dump import strip_dump


class TestStripDump(TestCase):
    """Tests for strip_dump() function."""

    @staticmethod
    def call_strip_dump(content: str) -> str | None:
        """Write *content* to a temporary file, run :func:`strip_dump` on it, and return the result.

        This helper creates two temporary files: one for the input dump content
        and one for the stripped output.  After calling :func:`strip_dump` with
        both paths, it reads and returns the content of the output file.  Both
        temporary files are deleted in a ``finally`` block regardless of
        whether the call succeeds.

        Args:
            content: The raw text to write to the source dump file before
                stripping.

        Returns:
            The text content of the output file after stripping, or
            ``None`` if an error prevents the output from being read
        """
        with tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False, encoding="utf-8") as o_file:
            o_file.write(content)
            source = Path(o_file.name)
        with tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False, encoding="utf-8") as t_file:
            target = Path(t_file.name)
        try:
            strip_dump(source, target)
            return target.read_text(encoding="utf-8")
        except OSError:
            return None
        finally:
            source.unlink()
            target.unlink()

    def test_strip_dump_removes_first_line(self) -> None:
        """The first line is always removed regardless of its content."""
        content = "ParserPhase\nline2\nline3\n"
        result = self.call_strip_dump(content)
        self.assertIsNotNone(result)
        result = cast(str, result)
        self.assertNotIn("ParserPhase", result)
        self.assertIn("line2", result)
        self.assertIn("line3", result)

    def test_strip_dump_removes_fatal_error_lines(self) -> None:
        """Lines containing 'Fatal error' at the end of the file are removed."""
        content = (
            "ParserPhase\n"
            '{"type": "Program"}\n'
            "Fatal error: something bad happened\n"
        )
        result = self.call_strip_dump(content)
        self.assertIsNotNone(result)
        result = cast(str, result)
        self.assertNotIn("Fatal error", result)
        self.assertIn('"type": "Program"', result)

    def test_strip_dump_no_fatal_errors(self) -> None:
        """When there are no fatal errors, only the first line is removed."""
        content = "ParserPhase\nline2\nline3\n"
        result = self.call_strip_dump(content)
        self.assertIsNotNone(result)
        result = cast(str, result)
        self.assertIn("line2", result)
        self.assertIn("line3", result)
        self.assertNotIn("ParserPhase", result)

    def test_strip_dump_outputs_to_stdout_when_no_target(self) -> None:
        """When target is None, stripped content is written to stdout."""
        content = "ParserPhase\nline2\nline3\n"
        with tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False, encoding="utf-8") as f:
            f.write(content)
            tmp = Path(f.name)
        try:
            with patch("sys.stdout", new_callable=io.StringIO) as mock_stdout:
                strip_dump(tmp)
                result = mock_stdout.getvalue()
            self.assertIn("line2", result)
            self.assertIn("line3", result)
            self.assertNotIn("ParserPhase", result)
            self.assertTrue(result.endswith("\n"))
        finally:
            tmp.unlink()

    def test_strip_dump_writes_newline_at_end(self) -> None:
        """The output file always ends with a newline character."""
        content = "ParserPhase\nsome content\n"
        result = self.call_strip_dump(content)
        self.assertIsNotNone(result)
        result = cast(str, result)
        self.assertTrue(result.endswith("\n"))
