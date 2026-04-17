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

import os
import logging
from pathlib import Path
from typing import List, Optional
from vmb.target import Target
from vmb.unit import BenchUnit
from vmb.cli import Args
from vmb.gensettings import GenSettings
from vmb.tool import ToolBase
from vmb.plugins.platforms.interop_base import InteropPlatformBase

log = logging.getLogger('vmb')


class Platform(InteropPlatformBase):

    def __init__(self, args: Args) -> None:
        super().__init__(args)
        self.aot_lib_opts = ' '.join(args.get('aot_lib_compiler_options', ''))
        here = os.path.realpath(os.path.dirname(__file__))
        runner_js = Path(here).parent.parent.joinpath('resources', 'interop', 'InteropRunner.js')
        host_interop_runner = ToolBase.libs.joinpath('InteropRunner.abc')
        self.es2abc_interop.run_es2abc(runner_js, host_interop_runner)
        self.interop_runner = f'{self.dev_dir.as_posix()}/InteropRunner.abc'
        self.x_sh.push(host_interop_runner, self.interop_runner)

    @property
    def name(self) -> str:
        return 'Interop ArkJS/ArkTS (s2d) device'

    @property
    def target(self) -> Target:
        return Target.OHOS

    @property
    def langs(self) -> List[str]:
        """Use this lang plugins."""
        return ['ets']

    @property
    def template(self) -> Optional[GenSettings]:
        return None

    def push_unit(self, bu: BenchUnit, *ext) -> None:
        """Override PlatformBase::Push bench unit to device."""
        super().push_unit(bu, *ext)
        native_dir = bu.path.joinpath('native')
        if not native_dir.exists():
            return
        if not bu.device_path:
            return  # make linter happy
        for f in native_dir.glob('*.so'):
            p = f.resolve()
            self.x_sh.push(p, bu.device_path.joinpath(f.name))

    # pylint: disable=duplicate-code
    def run_unit(self, bu: BenchUnit) -> None:
        libs = bu.libs('.ts', '.mjs', '.js')
        if not libs:
            raise RuntimeError('No dynamic imports in ets bench!')
        for lib in libs:
            # treat all src as ts to simplify arktsconfig generation
            lib = ToolBase.rename_suffix(lib, '.ts')
            self.es2abc_interop.run_es2abc(lib, lib.with_suffix('.abc'))
            self.x_sh.push(lib.with_suffix('.abc'), f'{self.dev_dir.as_posix()}/{lib.with_suffix(".abc").name}')
        self.es2panda(bu)
        zip_path = self.zip_classes(bu)
        zip_path_device = f'{self.dev_dir.as_posix()}/{zip_path.name}'
        self.x_sh.push(zip_path, zip_path_device)
        self.arkjs_interop.run(bu,
                               abc=str(self.interop_runner),
                               entry_point='InteropRunner',
                               bench_name=bu.name,
                               zip_path=zip_path_device)
