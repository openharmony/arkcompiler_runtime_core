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
import json
import sys
import tempfile
from pathlib import Path
from unittest import TestCase
from unittest.mock import patch

from runner.extensions.validators.srcdumper.ast_comparator import main


def _write_ast_dump(tmp_dir: Path, name: str, ast: dict) -> Path:
    p = tmp_dir / name
    p.write_text("ParserPhase\n" + json.dumps(ast), encoding="utf-8")
    return p


class TestMainCli(TestCase):
    """Tests for the CLI entry point main()."""

    def test_main_returns_0_on_equal_dumps(self) -> None:
        """CLI main() returns 0 when both dump files contain equal ASTs."""
        ast = {"type": "Program", "statements": []}
        with tempfile.TemporaryDirectory() as tmp:
            tmp_path = Path(tmp)
            dump1 = _write_ast_dump(tmp_path, "dump1.json", ast)
            dump2 = _write_ast_dump(tmp_path, "dump2.json", ast)
            with patch.object(sys, "argv", ["ast_comparator.py", dump1.as_posix(), dump2.as_posix()]):
                result = main()
        self.assertEqual(result, 0)

    def test_main_returns_1_on_different_dumps(self) -> None:
        """CLI main() returns 1 when dump files contain different ASTs."""

        ast1 = {"type": "Program", "value": 1}
        ast2 = {"type": "Program", "value": 2}
        with tempfile.TemporaryDirectory() as tmp:
            tmp_path = Path(tmp)
            dump1 = _write_ast_dump(tmp_path, "dump1.json", ast1)
            dump2 = _write_ast_dump(tmp_path, "dump2.json", ast2)
            with patch.object(sys, "argv", ["ast_comparator.py", dump1.as_posix(), dump2.as_posix()]):
                with patch('sys.stderr', new=io.StringIO()) as mock_stderr:
                    result = main()
                    stderr_output = mock_stderr.getvalue()
                    self.assertIn("AST comparison failed!", stderr_output)
        self.assertEqual(result, 1)
