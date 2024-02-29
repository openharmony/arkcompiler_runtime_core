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

from vmb.tool import ToolBase, OptFlags
from vmb.unit import BenchUnit
from vmb.target import Target


class Tool(ToolBase):

    def __init__(self, *args):
        super().__init__(*args)
        opts = self.custom
        if OptFlags.INT in self.flags:
            opts += ' --no-opt --jitless --use-ic --no-expose_wasm '
        if self.target == Target.HOST:
            d8 = ToolBase.get_cmd_path('d8', 'D8')
            self.d8 = f'{d8} {opts}'
        elif self.target == Target.DEVICE:
            self.d8 = f'{self.dev_dir}/d8/d8 {opts}'
        elif self.target == Target.OHOS:
            self.d8 = 'LD_LIBRARY_PATH=/data/local/and' \
                      f'roid {self.dev_dir}/d8/d8 {opts}'
        else:
            raise NotImplementedError(f'Not supported "{self.target}"!')

    @property
    def name(self) -> str:
        return 'D8 JavaScript Engine'

    @property
    def version(self) -> str:
        return self.x_run(
            f'echo "quit();"|{self.d8}').grep(r'version\s*([0-9\.]+)')

    def exec(self, bu: BenchUnit) -> None:
        mjs = self.x_src(bu, '.mjs')
        res = self.x_run(f'{self.d8} {mjs}')
        bu.parse_run_output(res)

    def kill(self) -> None:
        self.x_sh.run('pkill d8')
