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

import logging
import json
import os
from pathlib import Path
from vmb.platform import PlatformBase
from vmb.hook import HookBase
from vmb.helpers import create_file
from vmb.plugins.tools.es2panda import make_arktsconfig

log = logging.getLogger('vmb')


class Hook(HookBase):
    """Update paths in arktsconfig.

    This is for the case when there is no ark source tree
    and stdlib is included inside build directory.
    """

    @property
    def name(self) -> str:
        return 'Update arktsconfig.json'

    # pylint: disable-next=unused-argument
    def before_suite(self, platform: PlatformBase) -> None:
        ark_root_env = os.environ.get('PANDA_BUILD', '')
        if not ark_root_env:
            raise RuntimeError('PANDA_BUILD not set!')
        ark_root = Path(ark_root_env).resolve()
        if not ark_root.is_dir():
            raise RuntimeError(f'PANDA_BUILD "{ark_root}" does not exist!')
        config = ark_root.joinpath(
            'tools', 'es2panda', 'generated', 'arktsconfig.json')
        if config.is_file():
            log.info('Updating %s with %s', config, ark_root)
            with open(config, 'r', encoding="utf-8") as f:
                t = f.read()
                j = json.loads(t)
            old_root = j.get('compilerOptions', {}).get('baseUrl', 'failed')
            with create_file(config) as f:
                f.write(t.replace(old_root, str(ark_root)))
            return
        log.warning('%s does not exist! Creating it "manually"!', config)
        config.parent.mkdir(parents=True, exist_ok=True)
        make_arktsconfig(config, ark_root, [])
