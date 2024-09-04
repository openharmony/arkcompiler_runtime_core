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
from pathlib import Path
from shutil import copyfile
from json import load, dump
from glob import glob
from tempfile import mkstemp

from vmb.tool import ToolBase, VmbToolExecError
from vmb.unit import BenchUnit
from vmb.shell import ShellResult
from vmb.result import BuildResult, BUStatus

log = logging.getLogger('es2p')
JSON_PERMISSIONS = 0o664


class Tool(ToolBase):

    def __init__(self, *args):
        super().__init__(*args)
        self.panda_root = self.ensure_dir(
            os.environ.get('PANDA_BUILD', ''),
            err='Please set $PANDA_BUILD env var'
        )
        self.default_arktsconfig = f'{self.panda_root}' \
            f'/bin/arktsconfig.json'
        self.opts = '--gen-stdlib=false --extension=sts --opt-level=2 ' \
            f'{self.custom}'
        self.es2panda = self.ensure_file(self.panda_root, 'bin', 'es2panda')

    @property
    def name(self) -> str:
        return 'ES to Panda compiler (ArkTS mode)'

    def make_arktsconfig(self, bu: BenchUnit) -> str:
        js_files = glob(f'{str(bu.src().parent)}/*.js')
        js_files_includable = [x for x in js_files if x.find('bench_') == -1]
        [_, arktsconfig_path] = mkstemp('.json', 'arktsconfig_')
        sts_paths_includable = [str(x) for x in bu.libs()
                                if str(x).find('.abc') == -1]
        if len(js_files_includable) < 1:
            copyfile(self.default_arktsconfig, arktsconfig_path)
        else:
            self.extend_arktsconfig(js_files_includable,
                                    sts_paths_includable,
                                    arktsconfig_path)
        return arktsconfig_path

    def extend_arktsconfig(self,
                           extra_js_paths: list[str],
                           extra_sts_paths: list[str],
                           output_file: str
                           ) -> None:
        paths: dict = {}
        dynamic_paths: dict = {}

        for path in extra_js_paths:
            lib_name = Path(path).stem
            paths[lib_name] = [path]
            dynamic_paths[path] = {'language': 'js', 'hasDecl': False}

        for path in extra_sts_paths:
            lib_name = Path(path).stem
            paths[lib_name] = [path]
        config_fd = os.open(self.default_arktsconfig,
                            os.O_RDONLY | os.O_CREAT,
                            JSON_PERMISSIONS)
        output_fd = os.open(output_file,
                            os.O_WRONLY | os.O_CREAT | os.O_TRUNC,
                            0o664)
        raw_config = os.fdopen(config_fd, mode="r", encoding='utf-8')
        output = os.fdopen(output_fd, mode="w", encoding='utf-8')
        default_values = load(raw_config)
        raw_config.close()
        default_values['compilerOptions']['paths'] = {
            **default_values['compilerOptions']['paths'],
            **paths
        }
        default_values['compilerOptions']['dynamicPaths'] = {
            **default_values['compilerOptions']['dynamicPaths'],
            **dynamic_paths
        }
        dump(default_values, output)

    def exec(self, bu: BenchUnit) -> None:
        for lib in bu.libs('.ts', '.sts'):
            abc = lib.with_suffix('.abc')
            if abc.is_file():
                continue
            opts = '--ets-module ' + self.opts
            self.run_es2panda(lib, abc, opts, bu)
        src = bu.src('.ts', '.sts')
        abc = src.with_suffix('.abc')
        res = self.run_es2panda(src, abc, self.opts, bu)
        abc_size = self.sh.get_filesize(abc)
        bu.result.build.append(
            BuildResult('es2panda', abc_size, res.tm, res.rss))
        bu.binaries.append(abc)

    def run_es2panda(self,
                     src: Path,
                     abc: Path,
                     opts: str,
                     bu: BenchUnit) -> ShellResult:
        arktsconfig = self.make_arktsconfig(bu)
        res = self.sh.run(
            f'LD_LIBRARY_PATH={self.panda_root}/lib '
            f'{self.es2panda} {opts} '
            f'--arktsconfig={arktsconfig} '
            f'--output={abc} {src}',
            measure_time=True)
        if res.ret != 0 or not abc.is_file():
            bu.status = BUStatus.COMPILATION_FAILED
            raise VmbToolExecError(f'{self.name} failed: {src}', res)
        return res
