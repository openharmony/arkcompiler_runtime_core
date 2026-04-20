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
from pathlib import Path

from vmb.target import Target
from vmb.plugins.tools.es2panda_base import Es2PandaToolBase


class Tool(Es2PandaToolBase):

    def __init__(self, *args):
        super().__init__(*args)
        # This only supports panda binaries in the way they exist in ark standalone GN build
        self.panda_root = self.ensure_dir(
            os.environ.get('PANDA_BUILD', ''),
            err='Please set $PANDA_BUILD env var'
        )

        # There's also an identical one in `clang_x64` dir, but if feels more appropriate to use
        # this one
        self.etsstdlib = self.ensure_file(self.panda_root, 'gen', 'arkcompiler', 'runtime_core',
                                          'static_core', 'plugins', 'ets', 'etsstdlib.abc')
        if self.target == Target.OHOS:
            self.panda_root = self.ensure_dir(self.panda_root, "clang_x64")
        self.default_arktsconfig = str(Path(self.panda_root).joinpath(
            'arkcompiler', 'ets_frontend', 'arktsconfig.json'))
        self.opts = f'--extension=ets --opt-level=2 {self.custom}'
        self.es2panda = self.ensure_file(self.panda_root, 'arkcompiler', 'ets_frontend', 'es2panda')

    @property
    def name(self) -> str:
        return 'ES to Panda compiler (ArkTS hybrid mode)'

    @property
    def version(self) -> str:
        res = self.sh.run(f'{self.es2panda} --version')
        if res.ret == 1:
            return ' '.join(str(res.out or res.err).split())
        return super().version
