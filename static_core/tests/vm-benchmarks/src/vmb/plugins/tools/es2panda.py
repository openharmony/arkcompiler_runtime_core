#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
import os
import logging
from pathlib import Path
from glob import glob
from tempfile import mkstemp
from string import Template
from typing import List, Union

from vmb.tool import ToolBase, VmbToolExecError
from vmb.unit import BenchUnit
from vmb.shell import ShellResult
from vmb.result import BuildResult, BUStatus
from vmb.helpers import load_file, create_file

log = logging.getLogger('es2panda')


def make_arktsconfig(configpath: Union[str, Path],
                     ark_root: Union[str, Path],
                     extra_paths: List[str]) -> None:
    tpl_path = Path(__file__).parent.parent.parent.joinpath(
        'templates', 'arktsconfig.json').resolve()
    template = Template(
        load_file(tpl_path)).substitute(ROOT=str(ark_root))
    parsed_template = json.loads(template)
    if len(extra_paths) < 1:
        with create_file(configpath) as f:
            f.write(json.dumps(parsed_template))
        return

    paths: dict = {}
    dynamic_paths: dict = {}
    for path in extra_paths:
        lib_name = Path(path).stem
        is_compilable = Path(path).suffix == '.ets' or Path(path).is_dir()
        paths[lib_name] = [path]
        if is_compilable:
            continue

        key: str = path.replace("/", "_").replace(".", "_")
        dynamic_paths[key] = {'language': 'js', 'declPath': path, 'ohmUrl': path}
        parsed_template['compilerOptions']['paths'] = {
            **parsed_template['compilerOptions']['paths'],
            **paths
            }
        parsed_template['compilerOptions']['dynamicPaths'] = {
            **parsed_template['compilerOptions']['dynamicPaths'],
            **dynamic_paths
            }

    with create_file(configpath) as f:
        f.write(json.dumps(parsed_template))


class Tool(ToolBase):

    def __init__(self, *args):
        super().__init__(*args)
        self.panda_root = self.ensure_dir(
            os.environ.get('PANDA_BUILD', ''),
            err='Please set $PANDA_BUILD env var'
        )
        self.default_arktsconfig = str(Path(self.panda_root).joinpath(
            'tools', 'es2panda', 'generated', 'arktsconfig.json'))
        panda_stdlib_src = os.environ.get('PANDA_STDLIB_SRC', None)
        stdlib_opt = f'--stdlib={panda_stdlib_src}' if panda_stdlib_src else '--gen-stdlib=false'
        self.opts = f'{stdlib_opt} --extension=ets --opt-level=2 ' \
            f'{self.custom}'
        self.es2panda = self.ensure_file(self.panda_root, 'bin', 'es2panda')

    @property
    def name(self) -> str:
        return 'ES to Panda compiler (ArkTS mode)'

    def configure_paths(self, bu: BenchUnit) -> str:
        js_files = glob(f'{str(bu.src().parent)}/*.js')
        js_files_includable = [x for x in js_files if x.find('bench_') == -1]
        sts_paths_includable = [str(x) for x in bu.libs()
                                if str(x).find('.abc') == -1]
        extra_paths = js_files_includable + sts_paths_includable
        if len(extra_paths) < 1:
            return str(self.default_arktsconfig)
        _, arktsconfig_path = mkstemp('.json', 'arktsconfig_')
        make_arktsconfig(arktsconfig_path,
                         self.panda_root,
                         extra_paths)
        return str(arktsconfig_path)

    def exec(self, bu: BenchUnit) -> None:
        for lib in bu.libs('.ts', '.ets'):
            abc = lib.with_suffix('.abc')
            if abc.is_file():
                continue
            self.run_es2panda(lib, abc, self.opts, bu)
        src = bu.src('.ts', '.ets')
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
        arktsconfig = self.configure_paths(bu)
        res = self.sh.run(
            f'LD_LIBRARY_PATH={self.panda_root}/lib '
            f'{self.es2panda} {opts} '
            f'--arktsconfig={arktsconfig} '
            f'--output={abc} {src if not bu.src_for_es2panda_override else str(bu.src_for_es2panda_override)}',
            measure_time=True)
        if res.ret != 0 or not abc.is_file():
            bu.status = BUStatus.COMPILATION_FAILED
            raise VmbToolExecError(f'{self.name} failed: {src}', res)
        return res
