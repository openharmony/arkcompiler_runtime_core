#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
#

import os
from vmb.tool import ToolBase
from vmb.unit import BenchUnit


class Tool(ToolBase):

    def __init__(self, *args):
        super().__init__(*args)
        panda_build = self.ensure_dir_env('PANDA_BUILD')
        # assuming PANDA_BUILD is PANDA_ROOT/build
        modules = self.ensure_dir(panda_build, 'lib', 'module')
        self.ark_js_napi_cli = self.ensure_file(
            panda_build, 'bin', 'arkjsvm_interop', 'ark_js_napi_cli')
        etsstdlib = self.ensure_file(
            panda_build, 'plugins', 'ets', 'etsstdlib.abc')
        self.ld_library_path = f'LD_LIBRARY_PATH={panda_build}/lib/arkjsvm_interop/:{panda_build}/lib/ '
        vmb_bench_unit_iterations = os.environ.get('VMB_BENCH_UNIT_ITERATIONS', '')
        if not vmb_bench_unit_iterations:
            vmb_bench_unit_iterations = 10000
        self.cmd = f'MODULE_PATH={modules} ' \
                   f'ARK_ETS_STDLIB_PATH={etsstdlib} ' \
                   'ARK_ETS_INTEROP_JS_GTEST_ABC_PATH={test_zip} ' \
                   f'VMB_BENCH_UNIT_ITERATIONS={vmb_bench_unit_iterations} ' \
                   f'{self.ark_js_napi_cli} ' \
                   '--entry-point={entry_point} ' \
                   f' {self.custom} ' \
                   '{test_abc}'

    @property
    def name(self) -> str:
        return 'ArkJS VM Interop Host'

    @property
    def version(self) -> str:
        return 'version n/a'

    def exec(self, bu: BenchUnit) -> None:
        def ets_vm_opts() -> str:
            # convert to string for JSON parse: '{"name1": "value1", "name2": "value2"}'
            # and names of the options should be without minus signs "name1" not "--name1"
            if not self.custom_opts or not self.custom_opts.get('ark'):
                return '{"load-runtimes": "ets"}'
            ets_vm_opts_str: str = str(self.custom_opts.get('ark'))
            return ((ets_vm_opts_str.replace('[\'', '{\"').replace('\']', '\"}')
                    .replace('=', '\": \"').replace('\', \'', '\", \"'))
                    .replace('\"--', '\"'))

        js = bu.src('.js')
        if not js.is_file():
            # for bu_s2d
            js = bu.path.joinpath('InteropRunner.js')
        test_zip = bu.src('.zip')
        formatted_cmd = self.cmd.format(test_zip=test_zip,
                                        entry_point=js.stem,
                                        test_abc=str(js)[:-3] + '.abc'
                                        )
        with_ets_vm_opts: str = f'ETS_VM_OPTS=\'{ets_vm_opts()}\' '
        res = self.x_run(f'{self.ld_library_path} {with_ets_vm_opts} {formatted_cmd}',
                         measure_time=True)
        bu.parse_run_output(res)
