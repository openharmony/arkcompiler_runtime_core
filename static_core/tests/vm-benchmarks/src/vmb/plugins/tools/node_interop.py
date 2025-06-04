#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

from vmb.tool import ToolBase
from vmb.unit import BenchUnit


class Tool(ToolBase):

    def __init__(self, *args):
        super().__init__(*args)
        panda_build = self.ensure_dir_env('PANDA_BUILD')
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
        def ets_vm_opts() -> str:
            # convert to string for JSON parse: '{"name1": "value1", "name2": "value2"}'
            # and names of the options should be without minus signs "name1" not "--name1"
            if not self.custom_opts or not self.custom_opts.get('ark'):
                return '{"load-runtimes": "ets"}'
            ets_vm_opts: str = str(self.custom_opts.get('ark'))
            return ((ets_vm_opts.replace('[\'', '{\"').replace('\']', '\"}')
                    .replace('=', '\": \"').replace('\', \'', '\", \"'))
                    .replace('\"--', '\"'))

        js = bu.src('.js')
        if not js.is_file():
            # for bu_a2j
            js = bu.path.joinpath('InteropRunner.js')
        test_zip = bu.src('.zip')
        formatted_cmd = self.cmd.format(test_zip=test_zip,
                                        test_js=js
                                        )
        with_ets_vm_opts: str = f'ETS_VM_OPTS=\'{ets_vm_opts()}\' '
        res = self.x_run(f'{with_ets_vm_opts} {formatted_cmd}',
                         measure_time=True)
        bu.parse_run_output(res)
