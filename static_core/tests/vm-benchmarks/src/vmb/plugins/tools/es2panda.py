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

import os
from pathlib import Path

from vmb.tool import ToolBase, VmbToolExecError
from vmb.unit import BenchUnit
from vmb.shell import ShellResult
from vmb.result import BuildResult, BUStatus


class Tool(ToolBase):

    def __init__(self, *args):
        super().__init__(*args)
        self.panda_root = self.ensure_dir(
            os.environ.get('PANDA_BUILD', ''),
            err='Please set $PANDA_BUILD env var'
        )
        self.opts = '--gen-stdlib=false --extension=ets --opt-level=2 ' \
            f'--arktsconfig={self.panda_root}' \
            f'/tools/es2panda/generated/arktsconfig.json'
        self.es2panda = self.ensure_file(self.panda_root, 'bin', 'es2panda')

    @property
    def name(self) -> str:
        return 'ES to Panda compiler'

    def exec(self, bu: BenchUnit) -> None:
        for lib in bu.libs('.ts', '.ets'):
            abc = lib.with_suffix('.abc')
            if abc.is_file():
                continue
            opts = '--ets-module ' + self.opts
            self.run_es2panda(lib, abc, opts, bu)
        src = bu.src('.ts', '.ets')
        abc = src.with_suffix('.abc')
        res = self.run_es2panda(src, abc, self.opts, bu)
        abc_size = self.sh.get_filesize(abc)
        bu.result.build.append(
            BuildResult('es2panda', abc_size, res.tm, res.rss))

    def run_es2panda(self,
                     src: Path,
                     abc: Path,
                     opts: str,
                     bu: BenchUnit) -> ShellResult:
        res = self.sh.run(
            f'LD_LIBRARY_PATH={self.panda_root}/lib '
            f'{self.es2panda} {opts} '
            f'--output={abc} {src}',
            measure_time=True)
        if res.ret != 0 or not abc.is_file():
            bu.status = BUStatus.COMPILATION_FAILED
            raise VmbToolExecError(f'{self.name} failed: {src}')
        return res
