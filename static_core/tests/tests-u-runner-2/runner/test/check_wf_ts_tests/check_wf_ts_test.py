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
import unittest
from io import StringIO
from pathlib import Path
from unittest.mock import patch

from runner.map_test_suites import check_mapping_ts_vs_wf
from runner.test.test_utils import random_suffix


def env() -> dict[str, str]:
    return {
        # The test supposes to use the real path, not the faked one
        'ARKCOMPILER_RUNTIME_CORE_PATH': Path(__file__).parent.parent.parent.parent.parent.parent.parent.as_posix(),
        'ARKCOMPILER_ETS_FRONTEND_PATH': Path.cwd().as_posix(),
        'PANDA_BUILD': Path.cwd().as_posix(),
        'WORK_DIR': (Path.cwd() / f"work-{random_suffix()}").as_posix(),
        'CFG_PATH': (Path(__file__).parent.parent.parent.parent / "cfg").as_posix()
    }


class CheckWfTsTest(unittest.TestCase):
    """Test cases for the workflow vs test suite compatibility validation."""

    @patch.dict(os.environ, env(), clear=True)
    def test_deprecated_workflow(self) -> None:
        """A workflow listed in deprecated_workflows is rejected with a deprecation message."""
        args = {
            "test-suite": "ets-cts",
            "workflow": "arkjs-int"
        }
        with patch('sys.stderr', new=StringIO()) as mock_stderr:
            result = check_mapping_ts_vs_wf(args)
            self.assertFalse(result)
            stderr_output = mock_stderr.getvalue()
            self.assertIn("workflow is not supported for the", stderr_output)
            self.assertIn("test suite use one of following workflows", stderr_output)
            self.assertIn("workflow there is no acceptable test suites", stderr_output)

    @patch.dict(os.environ, env(), clear=True)
    def test_deprecated_test_suite(self) -> None:
        """A test suite with an empty workflow list (deprecated/unsupported) is rejected."""
        args = {
            "test-suite": "hermes",
            "workflow": "panda-int"
        }
        with patch('sys.stderr', new=StringIO()) as mock_stderr:
            result = check_mapping_ts_vs_wf(args)
            self.assertFalse(result)
            stderr_output = mock_stderr.getvalue()
            self.assertIn("workflow is not supported for the", stderr_output)
            self.assertIn("test suite there is no acceptable workflows", stderr_output)
            self.assertIn("The suite is either deprecated or not supported", stderr_output)

    @patch.dict(os.environ, env(), clear=True)
    def test_acceptable_test_suite(self) -> None:
        """A valid workflow and test suite pair is accepted and returns True."""
        args = {
            "test-suite": "ets-cts",
            "workflow": "panda-int"
        }
        result = check_mapping_ts_vs_wf(args)
        self.assertTrue(result)

    @patch.dict(os.environ, env(), clear=True)
    def test_incorrect_workflow(self) -> None:
        """A known workflow used with an incompatible test suite is rejected with suggestions."""
        args = {
            "test-suite": "astchecker",
            "workflow": "panda-int"
        }
        with patch('sys.stderr', new=StringIO()) as mock_stderr:
            result = check_mapping_ts_vs_wf(args)
            self.assertFalse(result)
            stderr_output = mock_stderr.getvalue()
            self.assertIn("workflow is not supported for the", stderr_output)
            self.assertIn("test suite use one of following workflows", stderr_output)
            self.assertIn("workflow use one of following test suites", stderr_output)

    @patch.dict(os.environ, env(), clear=True)
    def test_dependent_workflow(self) -> None:
        """A non-standalone workflow used without a parent workflow is allowed with a warning message."""
        args = {
            "test-suite": "ets-cts",
            "workflow": "ark"
        }
        with patch('sys.stderr', new=StringIO()) as mock_stderr:
            result = check_mapping_ts_vs_wf(args)
            self.assertTrue(result)
            stderr_output = mock_stderr.getvalue()
            self.assertIn("that might require additional actions before. See its description", stderr_output)
            self.assertIn("Make sure that the required ABC files are prepared", stderr_output)

    @patch.dict(os.environ, env(), clear=True)
    def test_metadata_parity_workflow_accepted(self) -> None:
        """panda-metadata-parity is listed in the ets-cts-ch24-metadata-parity suite's workflows."""
        args = {
            "test-suite": "ets-cts-ch24-metadata-parity",
            "workflow": "panda-metadata-parity"
        }
        result = check_mapping_ts_vs_wf(args)
        self.assertTrue(result)

    @patch.dict(os.environ, env(), clear=True)
    def test_metadata_parity_workflow_rejected_on_main_ets_cts(self) -> None:
        """panda-metadata-parity must NOT be silently usable against the main ets-cts suite —
        running parity logic on tests outside 24.build_system/metadata would either fail
        (missing binary-metadata counterparts) or pass vacuously. The dedicated suite
        ets-cts-ch24-metadata-parity is the only valid pairing."""
        args = {
            "test-suite": "ets-cts",
            "workflow": "panda-metadata-parity"
        }
        with patch('sys.stderr', new=StringIO()) as mock_stderr:
            result = check_mapping_ts_vs_wf(args)
            self.assertFalse(result)
            stderr_output = mock_stderr.getvalue()
            self.assertIn("workflow is not supported for the", stderr_output)
            self.assertIn("ets-cts-ch24-metadata-parity", stderr_output)

    @patch.dict(os.environ, env(), clear=True)
    def test_metadata_parity_suite_rejects_unrelated_workflow(self) -> None:
        """The ets-cts-ch24-metadata-parity suite lists only panda-metadata-parity —
        passing a runtime workflow like panda-int must be rejected so that a typo
        does not silently run a 0-step compile/runtime cycle on these test files."""
        args = {
            "test-suite": "ets-cts-ch24-metadata-parity",
            "workflow": "panda-int"
        }
        with patch('sys.stderr', new=StringIO()) as mock_stderr:
            result = check_mapping_ts_vs_wf(args)
            self.assertFalse(result)
            stderr_output = mock_stderr.getvalue()
            self.assertIn("workflow is not supported for the", stderr_output)
            self.assertIn("panda-metadata-parity", stderr_output)
