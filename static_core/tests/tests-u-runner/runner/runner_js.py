#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
# This file does only contain a selection of the most common options. For a
# full list see the documentation:
# http://www.sphinx-doc.org/en/master/config

import logging
from os import path

from runner.enum_types.configuration_kind import ConfigurationKind
from runner.logger import Log
from runner.options.config import Config
from runner.runner_file_based import RunnerFileBased
from runner.utils import get_platform_binary_name

_LOGGER = logging.getLogger("runner.runner_js")


class RunnerJS(RunnerFileBased):
    def __init__(self, config: Config, name: str) -> None:
        RunnerFileBased.__init__(self, config, name)
        ecmastdlib_abc: str = f"{self.build_dir}/pandastdlib/arkstdlib.abc"
        if not path.exists(ecmastdlib_abc):
            ecmastdlib_abc = f"{self.build_dir}/gen/plugins/ecmascript/ecmastdlib.abc"  # stdlib path for GN build

        self.quick_args = []
        if self.conf_kind == ConfigurationKind.QUICK:
            ecmastdlib_abc = self.generate_quick_stdlib(ecmastdlib_abc)

        if self.conf_kind == ConfigurationKind.QUICK:
            self.ark_quick = path.join(self.config.general.build, 'bin', get_platform_binary_name('arkquick'))
            if not path.isfile(self.runtime):
                Log.exception_and_raise(_LOGGER, f"Cannot find arkquick binary: {self.runtime}", FileNotFoundError)
            ecmastdlib_abc = self.generate_quick_stdlib(ecmastdlib_abc)

        self.runtime_args.extend([
            f'--boot-panda-files={ecmastdlib_abc}',
            '--load-runtimes=ecmascript',
        ])

        if self.conf_kind in [ConfigurationKind.AOT, ConfigurationKind.AOT_FULL]:
            self.aot_args.extend([
                f'--boot-panda-files={ecmastdlib_abc}',
                '--load-runtimes=ecmascript',
            ])
