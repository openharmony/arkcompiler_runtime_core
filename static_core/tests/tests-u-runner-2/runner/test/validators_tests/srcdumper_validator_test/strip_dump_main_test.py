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
import sys
import tempfile
from pathlib import Path
from unittest import TestCase
from unittest.mock import patch

from runner.extensions.validators.srcdumper.strip_dump import main


class TestStripDumpMain(TestCase):
    """Tests for the CLI entry point main()."""

    def test_main_returns_0_on_success(self) -> None:
        """main() returns 0 when the file exists and is processed successfully."""
        content = "ParserPhase\nsome content\n"
        with tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False, encoding="utf-8") as f:
            f.write(content)
            tmp = Path(f.name)
        try:
            with patch.object(sys, "argv", ["strip_dump.py", tmp.as_posix()]):
                with patch('sys.stdout', new=io.StringIO()):
                    result = main()
                    self.assertEqual(result, 0)
        finally:
            tmp.unlink(missing_ok=True)

    def test_main_returns_1_on_missing_file(self) -> None:
        """main() returns 1 when the specified file does not exist."""
        non_existent = "/tmp/this_file_does_not_exist_12345.txt"
        with patch.object(sys, "argv", ["strip_dump.py", non_existent]):
            with patch('sys.stderr', new=io.StringIO()) as mock_stderr:
                result = main()
                stderr_output = mock_stderr.getvalue()
                self.assertIn("Cannot read/write a file", stderr_output)
        self.assertEqual(result, 1)
