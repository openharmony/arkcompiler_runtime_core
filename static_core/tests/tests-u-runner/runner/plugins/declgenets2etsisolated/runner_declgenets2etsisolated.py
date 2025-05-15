#!/usr/bin/env python3
# -- coding: utf-8 --
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

import logging
from pathlib import Path
from typing import List

from runner.logger import Log
from runner.options.config import Config
from runner.runner_js import RunnerJS
from runner.plugins.declgenets2etsisolated.test_declgenets2etsisolated import (
    TestDeclgenIsolated,
)
from runner.runner_base import get_test_id
from runner.enum_types.test_directory import TestDirectory

_LOGGER = logging.getLogger(
    "runner.plugins.declgenets2etsisolated.runner_declgenets2etsisolated"
)


class RunnerDeclgenEts2EtsIsolated(RunnerJS):
    def __init__(self, config: Config) -> None:
        super().__init__(config, "declgen-ets2ets-isolated")
        es2panda_test = Path(config.general.static_core_root).parent.parent.joinpath(
            "ets_frontend", "ets2panda", "test", "isolated_declgen"
        )
        self.test_root = es2panda_test
        self.list_root = Path(config.general.static_core_root).joinpath(
            "plugins", "ets", "tests", "test-lists", "declgenets2etsisolated"
        )

        Log.summary(_LOGGER, f"TEST_ROOT set to {self.test_root}")

        self.collect_excluded_test_lists()
        self.collect_ignored_test_lists()

        self.explicit_list = self.recalculate_explicit_list(
            config.test_lists.explicit_list
        )
        test_dirs: List[TestDirectory] = [
            TestDirectory(
                "",
                "ets",
                flags=["--extension=ets", f"--arktsconfig={self.arktsconfig}"],
            )
        ]
        self.add_directories(test_dirs)

    @property
    def default_work_dir_root(self) -> Path:
        return Path("/tmp") / "declgen_ets2ets_isolated"

    def create_test(
        self, test_file: str, flags: List[str], is_ignored: bool
    ) -> TestDeclgenIsolated:
        test = TestDeclgenIsolated(
            self.test_env, test_file, flags, get_test_id(test_file, self.test_root)
        )
        test.ignored = is_ignored
        return test
