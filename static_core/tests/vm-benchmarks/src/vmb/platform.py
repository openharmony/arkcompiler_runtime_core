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

from __future__ import annotations
import logging
from typing import List, Optional, Type  # noqa
from abc import ABC, abstractmethod
from pathlib import Path
from vmb.tool import ToolBase
from vmb.target import Target
from vmb.unit import BenchUnit, UNIT_PREFIX
from vmb.helpers import get_plugins, get_plugin, die
from vmb.cli import Args
from vmb.shell import ShellUnix, ShellAdb, ShellHdc, ShellDevice
from vmb.result import ExtInfo
from vmb.x_shell import CrossShell


log = logging.getLogger('vmb')


class PlatformBase(CrossShell, ABC):
    """Platform Base."""

    def __init__(self, args: Args) -> None:
        self.__sh = ShellUnix(args.timeout)
        self.__andb = None
        self.__hdc = None
        self.__dev_dir = Path(args.get('device_dir', '/foo/bar'))
        self.args_langs = args.get('langs', set())
        self.ext_info: ExtInfo = {}
        if self.target == Target.DEVICE:
            self.__andb = ShellAdb(dev_serial=args.device,
                                   timeout=args.timeout,
                                   tmp_dir=args.device_dir)
        if self.target == Target.OHOS:
            self.__hdc = ShellHdc(dev_serial=args.device,
                                  timeout=args.timeout,
                                  tmp_dir=args.device_dir)
        ToolBase.sh_ = self.sh
        ToolBase.andb_ = self.andb
        ToolBase.hdc_ = self.hdc
        ToolBase.dev_dir = self.dev_dir
        # dir with shared libs (init before the suite)
        ToolBase.libs = Path(args.get_shared_path()).joinpath('libs')
        ToolBase.libs.mkdir(parents=True, exist_ok=True)
        # init all the tools
        tool_plugins = get_plugins(
            'tools',
            self.required_tools,
            extra=args.extra_plugins)
        self.flags = args.get_opts_flags()
        self.tools = {}
        for n, t in tool_plugins.items():
            tool: ToolBase = t.Tool(self.target,
                                    self.flags,
                                    args.get_custom_opts(n))
            self.tools[n] = tool
            log.info('%s %s', tool.name, tool.version)

    @classmethod
    def create(cls, args: Args) -> PlatformBase:
        try:
            platform_module = get_plugin(
                'platforms',
                args.platform,
                extra=args.extra_plugins)
            platform = platform_module.Platform(args)
        except Exception as e:  # pylint: disable=broad-exception-caught
            die(True, 'Plugin load error: %s', e)
        return platform

    @property
    @abstractmethod
    def name(self) -> str:
        return ''

    @property
    @abstractmethod
    def required_tools(self) -> List[str]:
        return []

    @property
    @abstractmethod
    def langs(self) -> List[str]:
        """Each platform should declare which templates it could use."""
        return []

    @property
    def required_hooks(self) -> List[str]:
        return []

    @property
    def dev_dir(self) -> Path:
        return self.__dev_dir

    @property
    def sh(self) -> ShellUnix:
        return self.__sh

    @property
    def andb(self) -> ShellDevice:
        if self.__andb is not None:
            return self.__andb
        # fake object for host platforms
        return ShellAdb.__new__(ShellAdb)

    @property
    def hdc(self) -> ShellDevice:
        if self.__hdc is not None:
            return self.__hdc
        # fake object for host platforms
        return ShellHdc.__new__(ShellHdc)

    @property
    @abstractmethod
    def target(self) -> Target:
        return Target.HOST

    @abstractmethod
    def run_unit(self, bu: BenchUnit) -> None:
        pass

    @property
    def gc_parcer(self) -> Optional[Type]:
        return None

    def cleanup(self, bu: BenchUnit) -> None:
        """Do default cleanup."""
        if Target.HOST != self.target:
            self.device_cleanup(bu)

    @staticmethod
    def search_units(paths: List[Path]) -> List[BenchUnit]:
        """Find bench units."""
        bus = []
        for bu_path in paths:
            if not bu_path.exists():
                log.warning('Requested unexisting path: %s', bu_path)
                continue
            # if file name provided add it unconditionally
            if bu_path.is_file():
                bus.append(BenchUnit(Path(bu_path).parent))
                continue
            # in case of dir search by file extention
            for p in bu_path.glob(f'**/{UNIT_PREFIX}*'):
                if p.is_dir():
                    bus.append(BenchUnit(p.resolve()))
        return bus

    def push_unit(self, bu: BenchUnit, *ext) -> None:
        """Push bench unit to device."""
        if Target.HOST == self.target:
            return
        bu.device_path = self.dev_dir.joinpath(bu.path.name)
        self.x_sh.run(f'mkdir -p {bu.device_path}')
        for f in bu.path.glob('*'):
            # skip if suffix filter provided
            if ext and f.suffix not in ext:
                continue
            p = f.resolve()
            self.x_sh.push(p, bu.device_path.joinpath(f.name))

    def push_libs(self) -> None:
        if Target.HOST == self.target:
            return
        self.x_sh.push(ToolBase.libs, self.dev_dir)

    def device_cleanup(self, bu: BenchUnit) -> None:
        if bu.device_path is None:
            return  # Bench Unit wasn't pused to device
        log.trace('Cleaning: %s', bu.device_path)
        self.x_sh.run(f'rm -rf {bu.device_path}')
