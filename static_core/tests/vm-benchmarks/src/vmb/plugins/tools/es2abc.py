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
from typing import Union
from pathlib import Path

from vmb.tool import ToolBase, VmbToolExecError
from vmb.unit import BenchUnit
from vmb.result import BuildResult, BUStatus
from vmb.shell import ShellResult


class Tool(ToolBase):
    """Wrapper for es2abc."""

    def __init__(self, *args):
        super().__init__(*args)
        self.sdk = self.ensure_dir_env('OHOS_SDK')
        self.opts = f'--module --merge-abc --extension=ts {self.custom}'
        if not os.path.isdir(self.sdk):
            raise RuntimeError('OHOS_SDK not found.')
        self.es2abc = os.path.join(
            self.sdk, 'out/x64.release/arkcompiler/ets_frontend/es2abc')
        if not os.path.isfile(self.es2abc):
            raise RuntimeError('es2abc not found.')

    @property
    def name(self) -> str:
        return 'ES to ABC compiler'

    def exec(self, bu: BenchUnit) -> None:
        ts_files = [bu.src('.ts')] + list(bu.libs('.ts'))
        for ts in ts_files:
            if not ts.is_file():
                continue
            abc = ts.with_suffix('.abc')
            abc.unlink(missing_ok=True)
            res = self.run_es2abc(ts, abc)
            if res.ret != 0 or not abc.is_file():
                bu.status = BUStatus.COMPILATION_FAILED
                raise VmbToolExecError(f'{self.name} failed: {ts}', res)
            abc_size = self.sh.get_filesize(abc)
            bu.result.build.append(
                BuildResult('es2abc', abc_size, res.tm, res.rss))
            bu.binaries.append(abc)

    def run_es2abc(self,
                   src: Union[str, Path],
                   out: Union[str, Path]) -> ShellResult:
        return self.sh.run(
            f'{self.es2abc} {self.opts} '
            f'--output={out} {src}',
            measure_time=True)
