#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2025 Huawei Device Co., Ltd.
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
from pathlib import Path

from runner.options.options import IOptions
from runner.options.options_general import GeneralOptions
from runner.options.options_test_suite import TestSuiteOptions
from runner.options.options_workflow import WorkflowOptions
from runner.options.step import StepKind
from runner.test.config_test import data_3


class WorkflowArkJsConfigTest(unittest.TestCase):
    current_folder = os.path.dirname(__file__)
    args = data_3.args

    def setUp(self) -> None:
        os.environ["ARKCOMPILER_RUNTIME_CORE_PATH"] = "."
        os.environ["ARKCOMPILER_ETS_FRONTEND_PATH"] = "."
        os.environ["WORK_DIR"] = "."
        os.environ["PANDA_BUILD"] = "."
        self.general = GeneralOptions(self.args, IOptions())
        self.test_suite = TestSuiteOptions(self.args, self.general)
        self.workflow = WorkflowOptions(cfg_content=self.args, parent_test_suite=self.test_suite)

    def test_loaded_steps(self) -> None:
        self.assertEqual(len(self.workflow.steps), 2)
        self.assertSetEqual(
            {step.name for step in self.workflow.steps},
            {'ohos_es2abc', 'ark_js_napi_cli'}
        )

    def test_ohos_es2abc_step(self) -> None:
        echo = [step for step in self.workflow.steps if step.name == 'ohos_es2abc']
        self.assertEqual(len(echo), 1, "Step 'es2panda' not found")
        step = echo[0]
        self.assertEqual(str(step.executable_path), "/usr/bin/echo")
        self.assertEqual(step.step_kind, StepKind.OTHER)
        self.assertEqual(step.timeout, 30)
        self.assertSetEqual(set(step.args), {
            "--merge-abc",
            "--extension=ts",
            "--module",
            "--output=./intermediate/${test-id}.abc",
            "./gen/${test-id}"
        })

    def test_ark_js_napi_cli_step(self) -> None:
        echo = [step for step in self.workflow.steps if step.name == 'ark_js_napi_cli']
        self.assertEqual(len(echo), 1, "Step 'es2panda' not found")
        step = echo[0]
        self.assertEqual(str(step.executable_path),
                         "runner/extensions/generators/ets_arkjs_xgc/ark_js_napi_cli_runner.sh")
        self.assertEqual(step.step_kind, StepKind.OTHER)
        self.assertEqual(step.timeout, 60)
        self.assertSetEqual(set(step.args), {
            "--work-dir",
            "./intermediate",
            "--build-dir .",
            "--stub-file ./gen/arkcompiler/ets_runtime/stub.an",
            "--enable-force-gc=false",
            "--open-ark-tools=true",
            "--entry-point",
            "${test-id}",
            "./intermediate/${test-id}.abc"
        })
        self.assertSetEqual(set(step.env), {
            "LD_LIBRARY_PATH"
        })
        self.assertEqual(step.env['LD_LIBRARY_PATH'], [
            "./arkcompiler/runtime_core:"]
        )

    def test_test_suite(self) -> None:
        self.assertEqual(self.test_suite.suite_name, "test_suite1")
        self.assertEqual(self.test_suite.test_root, Path.cwd().resolve())
        self.assertEqual(self.test_suite.list_root, Path.cwd().resolve())
        self.assertEqual(self.test_suite.extension(), "sts")
        self.assertEqual(self.test_suite.load_runtimes(), "ets")
        self.assertEqual(self.test_suite.work_dir, ".")
