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

# pylint: disable=duplicate-code
import logging
from typing import List, Optional
from vmb.helpers import ensure_env_var
from vmb.platform import PlatformBase
from vmb.target import Target
from vmb.unit import BenchUnit
from vmb.cli import Args
from vmb.gensettings import GenSettings

log = logging.getLogger('vmb')


class Platform(PlatformBase):

    def __init__(self, args: Args) -> None:
        ensure_env_var('PANDA_BUILD')
        super().__init__(args)
        self.es2panda = self.tools_get('es2panda')
        self.node = self.tools_get('node_interop')

    @property
    def name(self) -> str:
        return 'Interop ArkTS from NodeJS'

    @property
    def target(self) -> Target:
        return Target.HOST

    @property
    def required_tools(self) -> List[str]:
        return ['es2panda', 'node_interop', 'ark']

    @property
    def langs(self) -> List[str]:
        """Use this lang plugins."""
        return ['sts']

    @property
    def template(self) -> Optional[GenSettings]:
        """Special template."""
        return GenSettings(src={'.sts'},
                           template='Template-ArkTS-NodeJS-Interop.js',
                           out='.js',
                           link_to_src=True)

    def run_unit(self, bu: BenchUnit) -> None:
        abc = bu.src('.abc')
        if not abc.is_file():
            self.es2panda(bu)
            abc = bu.src('.abc')
        if self.dry_run_stop(bu):
            return
        abc.rename(abc.parent.joinpath('classes.abc'))
        self.sh.run(f'zip -r -0 {abc.with_suffix(".zip").name} classes.abc',
                    cwd=str(bu.path))
        self.node(bu)
