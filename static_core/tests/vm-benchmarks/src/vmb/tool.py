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
import shutil
from pathlib import Path
from typing import Optional, Iterable
from abc import ABC, abstractmethod
from enum import Flag, auto
from subprocess import TimeoutExpired
from vmb.unit import BenchUnit
from vmb.helpers import StringEnum
from vmb.shell import ShellDevice, ShellUnix, ShellResult
from vmb.target import Target
from vmb.x_shell import CrossShell

log = logging.getLogger('vmb')


class VmbToolExecError(Exception):
    """VMB Error. Tool execution failed."""

    def __init__(self, message: str) -> None:
        super().__init__(message)


class ToolMode(StringEnum):
    AOT = 'aot'
    INT = 'int'
    JIT = 'jit'
    DEFAULT = 'default'


class OptFlags(Flag):
    NONE = auto()
    GC_STATS = auto()
    JIT_STATS = auto()
    AOT_SKIP_LIBS = auto()
    DRY_RUN = auto()
    # these 3 flags are mutually exclusive (this is guarantied by ToolMode)
    AOT = auto()
    INT = auto()
    JIT = auto()


class ToolBase(CrossShell, ABC):

    sh_: ShellUnix
    andb_: ShellDevice
    hdc_: ShellDevice
    dev_dir: Path
    libs: Path

    def __init__(self,
                 target: Target = Target.HOST,
                 flags: OptFlags = OptFlags.NONE,
                 custom: str = ''):
        self._target = target
        self.flags = flags
        self.custom = custom

    def __call__(self, bu: BenchUnit) -> None:
        self.exec(bu)

    @abstractmethod
    def exec(self, bu: BenchUnit) -> None:
        pass

    @property
    @abstractmethod
    def name(self) -> str:
        return ''

    @property
    def version(self) -> str:
        return 'version n/a'

    @property
    def target(self) -> Target:
        return self._target

    @property
    def sh(self) -> ShellUnix:
        return ToolBase.sh_

    @property
    def andb(self) -> ShellDevice:
        return ToolBase.andb_

    @property
    def hdc(self) -> ShellDevice:
        return ToolBase.hdc_

    @staticmethod
    def rename_suffix(old: Path, new_suffix: str) -> Path:
        new = old.with_suffix(new_suffix)
        old.rename(new)
        return new

    @staticmethod
    def get_cmd_path(cmd: str, env_var: str = '') -> Optional[str]:
        # use specifically requested
        p: Optional[str] = os.environ.get(env_var, '')
        # . or use default
        if not p:
            p = shutil.which(cmd)
        if not p or (not os.path.isfile(p)):
            extra_msg = f' or set via {env_var} env var' if env_var else ''
            raise RuntimeError(
                f'{cmd} not found. Add it to PATH{extra_msg}')
        log.info('Using %s as %s', p, cmd)
        return p

    @staticmethod
    def ensure_file(*args, err: str = '') -> str:
        f = os.path.join(*args)
        if not os.path.isfile(f):
            raise RuntimeError(f'File "{f}" not found! {err}')
        return str(f)

    @staticmethod
    def ensure_dir(*args, err: str = '') -> str:
        d = os.path.join(*args)
        if not os.path.isdir(d):
            raise RuntimeError(f'Dir "{d}" not found! {err}')
        return str(d)

    def kill(self) -> None:
        """Kill tool process(es).

        For host target there is os.killpg() in Shell,
        but on device tool process needs remote pkill <tool>
        """

    def x_run(self, cmd: str, measure_time: bool = True,
              timeout: Optional[float] = None, cwd: str = '') -> ShellResult:
        try:
            res = self.x_sh.run(
                cmd, measure_time=measure_time, timeout=timeout, cwd=cwd)
        except TimeoutExpired as e:
            self.kill()
            raise e
        if not res or res.ret != 0:
            raise VmbToolExecError(f'{self.name} failed')
        return res

    def x_src(self, bu: BenchUnit, *ext) -> Path:
        if self.target == Target.HOST:
            return bu.src(*ext)
        return bu.src_device(*ext)

    def x_libs(self, bu: BenchUnit, *ext) -> Iterable[Path]:
        if self.target == Target.HOST:
            return bu.libs(*ext)
        return bu.libs_device(*ext)
