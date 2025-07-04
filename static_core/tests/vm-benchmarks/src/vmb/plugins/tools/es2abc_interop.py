#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2025 Huawei Device Co., Ltd.
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
from typing import Union
from pathlib import Path
from vmb.tool import ToolBase
from vmb.unit import BenchUnit

log = logging.getLogger('vmb')


class Tool(ToolBase):
    """Wrapper for es2abc hybrid."""

    def __init__(self, *args):
        super().__init__(*args)
        self.panda_root = self.ensure_dir_env('PANDA_BUILD')
        self.es2abc = self.ensure_file(self.panda_root, 'bin/interop_js/es2abc')
        self.cmd = f'{self.es2abc} --module --merge-abc ' \
                   '--extension={lang} {src} --output={out}'

    @property
    def name(self) -> str:
        return 'ES to ABC compiler Hybrid'

    def exec(self, bu: BenchUnit) -> None:
        src = bu.src('.ts', '.js', '.mjs')
        self.run_es2abc(src, bu.main_bin)

    def run_es2abc(self,
                   src: Union[str, Path],
                   out: Union[str, Path]):
        lang = Path(src).suffix.lstrip('.')
        if 'mjs' == lang:
            lang = 'js'  # arkjs doesn't know mjs
        res = self.sh.run(self.cmd.format(src=str(src), out=str(out), lang=lang))
        # Note: not gathering sizes and compile time
        if res.ret != 0:
            raise RuntimeError(f'Compilation es2abc failed: `{src}` -> `{out}`')
