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
import logging
from typing import Union, Iterable, Optional
from pathlib import Path

from vmb.tool import ToolBase, VmbToolExecError
from vmb.unit import BenchUnit
from vmb.result import BuildResult, BUStatus
from vmb.target import Target
from vmb.shell import ShellResult

log = logging.getLogger('vmb')


class Tool(ToolBase):

    bin_name = 'ark_aot'

    def __init__(self, *args) -> None:
        super().__init__(*args)
        if Target.HOST == self.target:
            panda_root = self.ensure_dir(
                os.environ.get('PANDA_BUILD', ''))
            self.paoc = self.ensure_file(panda_root, 'bin', self.bin_name)
            self.ark_lib = self.ensure_dir(panda_root, 'lib')
            self.etsstdlib = self.ensure_file(
                panda_root, 'plugins', 'ets', 'etsstdlib.abc')
        elif self.target in (Target.DEVICE, Target.OHOS):
            self.paoc = f'{self.dev_dir}/{self.bin_name}'
            self.ark_lib = f'{self.dev_dir}/lib'
            self.etsstdlib = f'{self.dev_dir}/etsstdlib.abc'
        else:
            raise NotImplementedError(f'Wrong target: {self.target}!')
        self.cmd = f'LD_LIBRARY_PATH={self.ark_lib} {self.paoc} ' \
                   f'--boot-panda-files={self.etsstdlib} ' \
                   '--load-runtimes=ets {opts} ' \
                   f'{self.custom} ' \
                   '--paoc-panda-files={abc} ' \
                   '--paoc-output={an}'

    @property
    def name(self) -> str:
        return 'Ark AOT Compiler'

    @staticmethod
    def panda_files(files: Iterable[Path]) -> str:
        if not files:
            return ''
        return '--panda-files=' + ':'.join([str(f.with_suffix('.abc'))
                                            for f in files])

    def exec(self, bu: BenchUnit) -> None:
        libs = self.x_libs(bu, '.abc')
        opts = self.panda_files(libs)
        for lib in libs:
            res = self.run_paoc(lib,
                                lib.with_suffix('.an'),
                                opts=opts)
            if 0 != res.ret:
                bu.status = BUStatus.COMPILATION_FAILED
                raise VmbToolExecError(f'{self.name} failed')
        abc = self.x_src(bu, '.abc')
        an = abc.with_suffix('.an')
        res = self.run_paoc(abc, an, opts=opts)
        if 0 != res.ret:
            bu.status = BUStatus.COMPILATION_FAILED
            raise VmbToolExecError(f'{self.name} failed')
        an_size = self.x_sh.get_filesize(an)
        bu.result.build.append(
            BuildResult(self.bin_name, an_size, res.tm, res.rss))

    def run_paoc(self,
                 abc: Union[str, Path],
                 an: Union[str, Path],
                 opts: str = '',
                 timeout: Optional[float] = None) -> ShellResult:
        return self.x_run(self.cmd.format(abc=abc, an=an, opts=opts),
                          timeout=timeout)

    def kill(self) -> None:
        self.x_sh.run(f'pkill {self.bin_name}')
