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
import shutil
from collections.abc import Callable
from pathlib import Path
from typing import ClassVar
from unittest import TestCase
from unittest.mock import patch

from runner.common_exceptions import InvalidConfiguration
from runner.extensions.generators.ets_cts.ets_templates_generator import EtsTemplatesGenerator
from runner.options.config import Config
from runner.test import test_utils
from runner.test.generators_test.ets_data_tests import data_test_suite0


class EtsGeneratorTest(TestCase):
    args = data_test_suite0.args
    get_instance_id: ClassVar[Callable[[], str]] = lambda: test_utils.create_runner_test_id(__file__)
    config: Config

    def load_config(self) -> None:
        self.config = Config(self.args)

    def generate_tests(self, test_source_path: Path, test_gen_path: Path) -> list[str]:
        ets_test_generator = EtsTemplatesGenerator(test_source_path, test_gen_path, self.config)
        ets_test_generator.generate()
        generated_tests = sorted(test.stem for test in test_gen_path.glob("*"))
        return generated_tests

    @patch.dict(os.environ, {
        'ARKCOMPILER_RUNTIME_CORE_PATH': ".",
        'ARKCOMPILER_ETS_FRONTEND_PATH': ".",
        'PANDA_BUILD': ".",
        'WORK_DIR': f"work-{test_utils.random_suffix()}"
    }, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    def test_disable_overwrite(self) -> None:

        test_source_path: Path = Path(__file__).with_name("ets_data_tests")
        test_gen_path: Path = Path(__file__).with_name("gen")
        shutil.rmtree(test_gen_path, ignore_errors=True)
        self.load_config()

        ets_test_generator = EtsTemplatesGenerator(test_source_path, test_gen_path, self.config)
        self.assertRaises(InvalidConfiguration, ets_test_generator.generate)

        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)
