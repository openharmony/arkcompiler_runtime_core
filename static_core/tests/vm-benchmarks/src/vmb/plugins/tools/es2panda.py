#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
from string import Template
from typing import List, Any, Union

from vmb.helpers import load_file, create_file
from vmb.target import Target
from vmb.plugins.tools.es2panda_base import Es2PandaToolBase

log = logging.getLogger('vmb')


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
            f.write(json.dumps(parsed_template, indent=4))
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
        dynamic_paths[key] = {'language': 'js', 'path': path, 'ohmUrl': path}
        parsed_template['compilerOptions']['paths'] = {
            **parsed_template['compilerOptions']['paths'],
            **paths
        }
        parsed_template['compilerOptions']['dependencies'] = {
            **parsed_template['compilerOptions']['dependencies'],
            **dynamic_paths
        }
    with create_file(configpath) as f:
        f.write(json.dumps(parsed_template, indent=4))


def fix_arktsconfig(dynamic_paths: Any = None) -> None:
    """Update arktsconfig.json with actual paths."""
    ark_root_env = os.environ.get('PANDA_BUILD', '')
    if not ark_root_env:
        raise RuntimeError('PANDA_BUILD not set!')
    ark_root = Path(ark_root_env).resolve()
    if not ark_root.is_dir():
        raise RuntimeError(f'PANDA_BUILD "{ark_root}" does not exist!')
    ark_src_env = os.environ.get('PANDA_SRC', '')
    if not ark_src_env:
        raise RuntimeError('PANDA_SRC not set! Please point it to static_core dir.')
    ark_src = Path(ark_src_env).resolve()
    if not ark_src.is_dir():
        raise RuntimeError(f'PANDA_SRC "{ark_src}" does not exist!')
    config = ark_root.joinpath(
        'tools', 'es2panda', 'generated', 'arktsconfig.json')
    if config.is_file():
        log.info('Updating %s with %s', config, ark_src)
        with open(config, 'r', encoding="utf-8") as f:
            j = json.load(f)
        old_root = j.get('compilerOptions', {}).get('baseUrl', 'failed')
        if dynamic_paths:
            for k, v in dynamic_paths.items():
                j['compilerOptions']['dependencies'][k] = v
        t = json.dumps(j)
        with create_file(config) as f:
            f.write(t.replace(old_root, str(ark_src)))
        return
    log.warning('%s does not exist! Creating it "manually"!', config)
    make_arktsconfig(config, ark_src, [])


class Tool(Es2PandaToolBase):

    def __init__(self, *args):
        super().__init__(*args)
        # Panda binaries could be in form of:
        # 1) panda build artifact
        # 2) panda ohos sdk
        # PANDA_SDK and PANDA_BUILD could be set simultaneously
        panda_sdk = os.environ.get('PANDA_SDK', '')
        if panda_sdk and Target.OHOS == self.target:
            # NOTE: This could be also 'windows_host_tools'
            panda_root = os.path.join(panda_sdk, 'linux_host_tools')
            self.panda_root = self.ensure_dir(
                panda_root, err='Please point $PANDA_SDK to sdk/package')
            os.environ['PANDA_BUILD'] = panda_root
        elif self.custom_path:
            self.panda_root = Path(self.custom_path).parent.parent
        else:
            self.panda_root = self.ensure_dir(
                os.environ.get('PANDA_BUILD', ''),
                err='Please set $PANDA_BUILD env var'
            )
        self.default_arktsconfig = str(Path(self.panda_root).joinpath(
            'bin', 'arktsconfig.json'))
        panda_stdlib_src = os.environ.get('PANDA_STDLIB_SRC', None)
        stdlib_opt = f'--stdlib={panda_stdlib_src}' if panda_stdlib_src else '--gen-stdlib=false'
        self.opts = f'{stdlib_opt} --extension=ets --opt-level=2 ' \
            f'{self.custom}'
        if self.custom_path:
            self.es2panda = self.ensure_file(self.custom_path)
        else:
            self.es2panda = self.ensure_file(self.panda_root, 'bin', 'es2panda')
        self.etsstdlib = self.ensure_file(self.panda_root, 'plugins', 'ets', 'etsstdlib.abc')

    @property
    def name(self) -> str:
        return 'ES to Panda compiler (ArkTS mode)'

    @property
    def version(self) -> str:
        res = self.sh.run(f'{self.es2panda} --version')
        if res.ret == 1:
            return ' '.join(str(res.out or res.err).split())
        return super().version
