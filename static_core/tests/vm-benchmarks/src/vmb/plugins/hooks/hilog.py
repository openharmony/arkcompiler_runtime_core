#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2026 Huawei Device Co., Ltd.
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
from typing import Optional  # noqa
from vmb.cli import Args
from vmb.unit import BenchUnit, BENCH_PREFIX
from vmb.hook import HookBase
from vmb.platform import PlatformBase
from vmb.target import Target
from vmb.shell import ShellDevice

log = logging.getLogger('vmb')


class Hook(HookBase):
    """Attach Hilog logs."""

    def __init__(self, args: Args) -> None:
        super().__init__(args)
        self.target: Target = Target.HOST
        self.hdc: Optional[ShellDevice] = None
        self.hilog_name: str = ''

    @property
    def name(self) -> str:
        return 'Attach Hilog logs.'

    def before_suite(self, platform: PlatformBase) -> None:
        self.target = platform.target
        self.hdc = platform.hdc
        if self.hdc is None:
            raise RuntimeError('Hdc has not been set!')
        self.hdc.run('hilog -p off')  # disable privacy
        self.hdc.run('hilog -k on')  # enable kernel logging

    def before_unit(self, bu: BenchUnit) -> None:
        if self.hdc is None:
            raise RuntimeError('Hdc has not been set!')
        self.hilog_name = f'{BENCH_PREFIX}{bu.name}.hilog.txt'
        self.hdc.run('hilog -r')  # clear log buffer

    def after_unit(self, bu: BenchUnit) -> None:
        if self.target == Target.HOST:
            return
        if self.hdc is None:
            raise RuntimeError('Hdc has not been set!')
        if bu.device_path is None:
            raise RuntimeError(f'Hilog: no device path for "{bu.name}"')
        hilog_src = bu.device_path.joinpath(self.hilog_name)
        hilog_dst = bu.path.joinpath(self.hilog_name)
        self.hdc.run(f'hilog -x --format nsec > {hilog_src.as_posix()}')
        cur_size = 0
        while cur_size == 0:
            cur_size = self.hdc.get_filesize(hilog_src)
        self.hdc.pull(hilog_src, hilog_dst)
        log.debug('[Hilog] Copy of Hilog for %s starts here', bu.name)
        with hilog_dst.open("r", encoding="utf-8", errors="replace") as f:
            for line in f:
                line = line.rstrip("\n")
                if line:
                    log.debug('[Hilog]%s', line)
