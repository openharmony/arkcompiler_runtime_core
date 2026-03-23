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

import json
import tempfile
from pathlib import Path
from unittest import TestCase

from runner.extensions.validators.srcdumper.ast_comparator import AstComparator, AstComparatorException


class TestLoadAst(TestCase):
    """Tests for AstComparator.load_ast()"""

    def test_load_ast_skips_first_line(self) -> None:
        """First line (phase name) must be stripped before JSON parsing."""
        ast = {"type": "Program", "statements": []}
        with tempfile.NamedTemporaryFile(mode="w", suffix=".json", delete=False, encoding="utf-8") as f:
            f.write("ParserPhase\n")
            f.write(json.dumps(ast))
            tmp = Path(f.name)
        try:
            result = AstComparator.load_ast(tmp)
            # After normalization the dict may be modified, but the type key must survive
            self.assertEqual(result.get("type"), "Program")
        finally:
            tmp.unlink()

    def test_load_ast_strips_trailing_errors(self) -> None:
        """Lines after the last closing '}' (e.g. error messages) must be removed.

        The load_ast algorithm scans lines in reverse to find the last standalone '}'
        line. Any lines that appear after it (i.e. have a smaller reverse index) are
        stripped before JSON parsing.  The trailing error lines must therefore come
        AFTER the closing '}' of the JSON object.
        """
        # Use pretty-printed JSON so the closing '}' is on its own line.
        ast_pretty = json.dumps({"type": "Program", "statements": []}, indent=2)
        trailing = "Error: something went wrong\nAnother error line"
        content = "ParserPhase\n" + ast_pretty + "\n" + trailing
        with tempfile.NamedTemporaryFile(mode="w", suffix=".json", delete=False, encoding="utf-8") as f:
            f.write(content)
            tmp = Path(f.name)
        try:
            result = AstComparator.load_ast(tmp)
            self.assertEqual(result.get("type"), "Program")
        finally:
            tmp.unlink()

    def test_load_ast_invalid_json_raises(self) -> None:
        """Malformed JSON after stripping the first line must raise AstComparatorException."""
        content = "ParserPhase\nthis is not json at all"
        with tempfile.NamedTemporaryFile(mode="w", suffix=".json", delete=False, encoding="utf-8") as f:
            f.write(content)
            tmp = Path(f.name)
        try:
            with self.assertRaises(AstComparatorException):
                AstComparator.load_ast(tmp)
        finally:
            tmp.unlink()
