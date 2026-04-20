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

import json
from pathlib import Path
from typing import Iterable

from vmb.tool import ToolBase, VmbToolExecError, OptFlags
from vmb.unit import BenchUnit
from vmb.shell import ShellResult
from vmb.result import BuildResult, BUStatus
from vmb.helpers import create_file, force_link
from vmb.target import Target


class Es2PandaToolBase(ToolBase):

    def __init__(self, *args):
        super().__init__(*args)
        self.panda_root = ""
        self.default_arktsconfig = ""
        self.es2panda = ""
        self.opts = ""
        self.etsstdlib = ""

    def update_config(self, bu: BenchUnit) -> str:
        static: Iterable[Path] = bu.libs('.ets')
        dynamic: Iterable[Path] = bu.libs('.ts')  # Note: no js; no mjs
        if not dynamic and not static:
            return self.default_arktsconfig
        with open(self.default_arktsconfig, 'r', encoding="utf-8") as f:
            j = json.load(f)
        for s in static:
            lib = s.with_suffix('')
            j['compilerOptions']['paths'][lib.name] = [str(lib), ]
        for d in dynamic:
            lib = d.with_suffix('')
            ohm = f'{self.dev_dir.as_posix()}/{lib.name}' if Target.OHOS == self.target else str(lib)
            j['compilerOptions']['dependencies'][lib.name] = {'language': 'js', 'ohmUrl': ohm}
        for libname, libpath in bu.seamless_paths.items():
            entry = j['compilerOptions']['dependencies'].get(libname, None)
            if entry is not None:
                entry['path'] = str(libpath)

        etsstdlib_link = bu.path.joinpath(j['compilerOptions']['baseUrl'], 'plugins', 'ets', 'etsstdlib.abc')
        etsstdlib_link.parent.mkdir(parents=True, exist_ok=True)
        force_link(etsstdlib_link, self.etsstdlib)

        conf = bu.path.joinpath('arktsconfig.json')
        with create_file(conf) as f:
            json.dump(j, f, indent=4)
        return str(conf)

    def exec(self, bu: BenchUnit) -> None:
        for lib in bu.libs('.ets'):  # compile ETS imports
            abc = lib.with_suffix('.abc')
            if abc.is_file():
                continue
            self.run_es2panda(lib, abc, self.opts, bu)
        # check if arktsconfig should be updated
        conf = self.update_config(bu)
        # compile bench
        src = bu.src('.ets')
        abc = src.with_suffix('.abc')
        res = self.run_es2panda(src, abc, self.opts, bu, conf)
        abc_size = self.sh.get_filesize(abc)
        bu.result.build.append(
            BuildResult('es2panda', abc_size, res.tm, res.rss))
        bu.binaries.append(abc)

    def run_es2panda(self,
                     src: Path,
                     abc: Path,
                     opts: str,
                     bu: BenchUnit,
                     conf: str = '') -> ShellResult:
        # Do not re-build if specifically asked so
        if OptFlags.SKIP_COMPILATION in self.flags and abc.exists():
            return ShellResult(ret=0, out="Compilation skipped")
        arktsconfig = conf if conf else self.default_arktsconfig
        res = self.sh.run(
            f'{self.es2panda} {opts} '
            f'--arktsconfig={arktsconfig} '
            f'--output={abc} {src}',
            measure_time=True)
        if self.no_run:
            bu.status = BUStatus.NOT_RUN
            return res
        if res.ret != 0 or not abc.is_file():
            bu.status = BUStatus.COMPILATION_FAILED
            raise VmbToolExecError(f'{self.name} failed: {src}', res)
        return res
