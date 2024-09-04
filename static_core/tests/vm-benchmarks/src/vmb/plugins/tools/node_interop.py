#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024 Huawei Device Co., Ltd.
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

from pathlib import Path

from vmb.tool import ToolBase
from vmb.unit import BenchUnit

NODE_RUNNER = Path(__file__).parent.parent.parent\
    .joinpath('templates', 'InteropRunner.js')


class Tool(ToolBase):

    def __init__(self, *args):
        super().__init__(*args)
        panda_build = self.ensure_dir_env('PANDA_BUILD')
        self.runner_script = self.ensure_file(NODE_RUNNER)
        # assuming PANDA_BUILD is PANDA_ROOT/build
        modules = self.ensure_dir(panda_build, 'lib', 'module')
        self.node = self.ensure_file(
            panda_build, '..', 'third_party',
            'nodejs', 'node-v18.13.0-linux-x64', 'bin', 'node')
        etsstdlib = self.ensure_file(
            panda_build, 'plugins', 'ets', 'etsstdlib.abc')
        self.cmd = f'MODULE_PATH={modules} ' \
                   f'ARK_ETS_STDLIB_PATH={etsstdlib} ' \
                   'ARK_ETS_INTEROP_JS_GTEST_ABC_PATH={test_zip} ' \
                   'VMB_BENCH_UNIT_NAME={name} ' \
                   'VMB_BENCH_UNIT_ITERATIONS=10000 ' \
                   f'{self.node} {self.custom} ' \
                   '{test_js}'

    @property
    def name(self) -> str:
        return 'NodeJS Interop'

    @property
    def version(self) -> str:
        return self.sh.run(
            f'{self.node} --version').grep(r'v([0-9\.]+)')

    def exec(self, bu: BenchUnit) -> None:
        js = bu.src('.js')
        if not js.is_file():
            js = self.runner_script
        test_zip = bu.src('.zip')
        formatted_cmd = self.cmd.format(test_zip=test_zip,
                                        test_js=js,
                                        name=bu.name
                                        )
        res = self.x_run(formatted_cmd,
                         measure_time=True)
        bu.parse_run_output(res)
