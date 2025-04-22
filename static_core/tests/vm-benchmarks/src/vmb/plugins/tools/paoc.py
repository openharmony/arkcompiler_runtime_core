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

import logging
from typing import Union, Iterable, Optional
from pathlib import Path

from vmb.tool import ToolBase, OptFlags, VmbToolExecError
from vmb.unit import BenchUnit
from vmb.result import BuildResult, BUStatus, AOTStats
from vmb.target import Target
from vmb.shell import ShellResult

log = logging.getLogger('vmb')


class Tool(ToolBase):

    bin_name = 'ark_aot'

    def __init__(self, *args) -> None:
        super().__init__(*args)
        if Target.HOST == self.target:
            panda_root = self.ensure_dir_env('PANDA_BUILD')
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
        if OptFlags.AOT_STATS in self.flags:
            aot_stats = '--compiler-dump-stats-csv={an}.dump.csv '
        else:
            aot_stats = ''
        if OptFlags.LLVMAOT in self.flags:
            aot_mode = '--paoc-mode=llvm '
        else:
            aot_mode = '--paoc-mode=aot '

        self.cmd = f'LD_LIBRARY_PATH={self.ark_lib} {self.paoc} ' \
                   f'--boot-panda-files={self.etsstdlib} {aot_mode} ' \
                   '--load-runtimes=ets {opts} ' \
                   f'{self.custom} {aot_stats}' \
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

    def do_exec(self, bu: BenchUnit, profdata: bool = False) -> None:
        _, bu_opts = self.get_bu_opts(bu)
        libs = self.x_libs(bu, '.abc')
        opts = self.panda_files(libs)
        if bu_opts:
            opts += ' ' + bu_opts
        for lib in libs:
            res = self.run_paoc(lib,
                                lib.with_suffix('.an'),
                                opts=opts)
            if 0 != res.ret:
                bu.status = BUStatus.COMPILATION_FAILED
                raise VmbToolExecError(f'{self.name} failed', res)
        abc = self.x_src(bu, '.abc')
        an = abc.with_suffix('.an')
        if profdata:
            opts += f' --paoc-use-profile:path={abc}.profdata,force '
        res = self.run_paoc(abc, an, opts=opts)
        if 0 != res.ret:
            bu.status = BUStatus.COMPILATION_FAILED
            raise VmbToolExecError(f'{self.name} failed', res)
        an_size = self.x_sh.get_filesize(an)
        bu.result.build.append(
            BuildResult(self.bin_name, an_size, res.tm, res.rss))
        bu.result.aot_stats = self.get_aot_stats(an, bu.path)
        bu.binaries.append(an)

    def exec(self, bu: BenchUnit) -> None:
        self.do_exec(bu)

    def run_paoc(self,
                 abc: Union[str, Path],
                 an: Union[str, Path],
                 opts: str = '',
                 timeout: Optional[float] = None) -> ShellResult:
        return self.x_run(self.cmd.format(abc=abc, an=an, opts=opts),
                          timeout=timeout)

    def get_aot_stats(self, an: Union[str, Path],
                      local_dir: Union[str, Path]) -> Optional[AOTStats]:
        if OptFlags.AOT_STATS not in self.flags:
            return None
        csv = Path(f'{an}.dump.csv')
        if self.target != Target.HOST:
            self.x_sh.pull(csv, local_dir)
            csv = Path(local_dir).joinpath(csv.name)
        if not csv.exists():
            log.error('AOT stats dump missed: %s', str(csv))
            return None
        return AOTStats.from_csv(csv)

    def kill(self) -> None:
        self.x_sh.run(f'pkill {self.bin_name}')
