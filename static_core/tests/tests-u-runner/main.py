#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

import os
import sys
from datetime import datetime
from typing import List

import pytz
from dotenv import load_dotenv

from runner.logger import Log
from runner.options.cli_options import get_args
from runner.options.config import Config
from runner.plugins_registry import PluginsRegistry
from runner.runner_base import Runner


def main() -> None:
    dotenv_path = os.path.join(os.path.dirname(__file__), '.env')
    if os.path.exists(dotenv_path):
        load_dotenv(dotenv_path)

    args = get_args()
    config = Config(args)
    logger = Log.setup(config.general.verbose, config.general.work_dir)
    config.log_warnings()
    Log.summary(logger, f"Loaded configuration: {config}")
    config.generate_config()

    registry = PluginsRegistry()
    config.custom.validate()
    runners: List[Runner] = []

    if config.general.processes == 1:
        Log.default(logger, "Attention: tests are going to take only 1 process. The execution can be slow. "
                            "You can set the option `--processes` to wished processes quantity "
                            "or use special value `all` to use all available cores.")
    start = datetime.now(pytz.UTC)
    for test_suite in config.test_suites:
        plugin = "ets" if test_suite.startswith("ets") or test_suite.startswith("sts") else test_suite
        runner_class = registry.get_runner(plugin)
        if runner_class is not None:
            runners.append(runner_class(config))
        else:
            Log.exception_and_raise(logger, f"Plugin {plugin} not registered")

    failed_tests = 0

    if not config.general.generate_only:
        for runner in runners:
            Log.all(logger, f"Runner {runner.name} started")
            runner.run()
            Log.all(logger, f"Runner {runner.name} finished")
            failed_tests += runner.summarize()
            Log.all(logger, f"Runner {runner.name}: {failed_tests} failed tests")
            if config.general.coverage.use_llvm_cov:
                runner.create_coverage_html()

    finish = datetime.now(pytz.UTC)
    Log.default(logger, f"Runner has been working for {round((finish-start).total_seconds())} sec")

    registry.cleanup()
    sys.exit(0 if failed_tests == 0 else 1)


if __name__ == "__main__":
    main()
