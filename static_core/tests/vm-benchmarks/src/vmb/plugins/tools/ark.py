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

import logging
from pathlib import Path
from vmb.target import Target
from vmb.tool import ToolBase, OptFlags
from vmb.unit import BenchUnit
from vmb.result import JITStat

log = logging.getLogger('vmb')


class Tool(ToolBase):

    def __init__(self, *args) -> None:
        super().__init__(*args)
        if Target.HOST == self.target:
            panda_root = self.ensure_dir_env('PANDA_BUILD')
            self.ark = self.ensure_file(panda_root, 'bin', 'ark')
            self.ark_lib = self.ensure_dir(panda_root, 'lib')
            self.etsstdlib = self.ensure_file(
                panda_root, 'plugins', 'ets', 'etsstdlib.abc')
        elif self.target in (Target.DEVICE, Target.OHOS):
            self.ark = f'{self.dev_dir}/ark'
            self.ark_lib = f'{self.dev_dir}/lib'
            self.etsstdlib = f'{self.dev_dir}/etsstdlib.abc'
        else:
            raise NotImplementedError(f'Wrong target: {self.target}!')
        opts = '--gc-trigger-type=heap-trigger '
        an_files = []
        if OptFlags.AOT_SKIP_LIBS not in self.flags:
            stdlib = str(ToolBase.libs / 'etsstdlib.an') \
                if Target.HOST == self.target \
                else str(self.dev_dir / 'etsstdlib.an')
            an_files.append(stdlib)
        if OptFlags.INT in self.flags:
            opts += '--compiler-enable-jit=false '
        if OptFlags.GC_STATS in self.flags:
            opts += '--print-gc-statistics --log-components=gc ' \
                    '--log-level=info --log-stream=file ' \
                    '--log-file={gclog} '
        if OptFlags.JIT_STATS in self.flags:
            opts += '--compiler-dump-jit-stats-csv={abc}.dump.csv '
        if OptFlags.AOT in self.flags:
            an_files.append('{an}')
        if an_files:
            opts += '--enable-an:force --aot-files=' + \
                    ":".join(an_files) + ' '
        self.cmd = f'LD_LIBRARY_PATH={self.ark_lib} {self.ark} ' \
                   f'--boot-panda-files={self.etsstdlib} ' \
                   f'--load-runtimes=ets {opts} {self.custom} ' \
                   '{options} {abc} ETSGLOBAL::main'

    @property
    def name(self) -> str:
        return 'Ark VM'

    def exec(self, bu: BenchUnit) -> None:
        bu_flags, _ = self.get_bu_opts(bu)
        gclog = ''
        libs = ':'.join([str(f) for f in self.x_libs(bu, '.abc')])
        options = f'--panda-files={libs} ' if libs else ''
        abc = self.x_src(bu, '.abc')
        an_files = [str(f) for f in self.x_libs(bu, '.an')] + \
                   [str(abc.with_suffix('.an'))]
        an = ':'.join(an_files) if an_files else ''
        if OptFlags.DISABLE_INLINING in bu_flags:
            options += '--compiler-inlining=false '
        if OptFlags.GC_STATS in bu_flags:
            gclog = str(abc.with_suffix('.gclog.txt'))
        arkts_cmd = self.cmd.format(
            abc=abc, options=options, gclog=gclog, an=an)
        res = self.x_run(arkts_cmd)
        bu.parse_run_output(res)
        if OptFlags.JIT_STATS in bu_flags:
            csv = Path(f'{abc}.dump.csv')
            if self.target != Target.HOST:
                self.x_sh.pull(csv, bu.path)
                csv = bu.path.joinpath(csv.name)
            if csv.exists():
                bu.result.jit_stats = JITStat.from_csv(csv)
            else:
                log.error('JIT stats dump missed: %s', str(csv))

    def kill(self) -> None:
        self.x_sh.run('pkill ark')
