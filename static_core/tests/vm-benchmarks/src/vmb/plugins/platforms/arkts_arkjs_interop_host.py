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

# pylint: disable=duplicate-code
import logging
from pathlib import Path
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
        self.arkjs_interop = self.tools_get('arkjs_interop')
        self.ark = self.tools_get('ark')

    @property
    def name(self) -> str:
        return 'Interop ArkTS from ArkJS VM host'

    @property
    def target(self) -> Target:
        return Target.HOST

    @property
    def required_tools(self) -> List[str]:
        return ['es2panda', 'arkjs_interop', 'ark']

    @property
    def langs(self) -> List[str]:
        """Use this lang plugins."""
        return ['interop_arkjs']

    @property
    def template(self) -> Optional[GenSettings]:
        """Special template."""
        # NOTE these are only to trace mis-use of the API, real settings
        #  are dependent on interop mode and established later when it gets known.
        return GenSettings(src={'interop_src'},
                           template='interop_template',
                           out='interop_out',
                           link_to_src=False)

    def run_unit(self, bu: BenchUnit) -> None:
        log.trace('Run bench unit tags %s path: %s', bu.tags if hasattr(bu, 'tags') else None, bu.path)
        if not hasattr(bu, 'tags'):
            log.debug('Bench unit does not have tags, looking for freestyle benchmarks to run')
            self.run_generated(bu)
            return

        if not bu.doclet_src:
            raise ValueError(f'Sources for unit {bu.name} are not set')
        if 'bu_a2a' in bu.tags:
            self.es2panda(bu)
            self.ark(bu)
            return
        if 'bu_a2j' in bu.tags:
            self.establish_arkjs_module()
            self.sh.run(f'cp -f {str(bu.doclet_src.parent)}/*.js {bu.path}')
            self.es2panda(bu)
            self.es2abc('InteropRunner', str(bu.path))
            self.es2abc('test_import', str(bu.path))
            self.run_generated(bu)
            return
        if 'bu_j2j' in bu.tags:
            self.establish_arkjs_module()
            self.sh.run(f'cp -f -n {str(bu.doclet_src.parent)}/*.js {bu.path}')
            self.es2abc(str(bu.src('.js').stem), str(bu.path))
            self.es2abc('test_import', str(bu.path))
            if self.dry_run_stop(bu):
                return
            self.arkjs_interop(bu)
            return
        if 'bu_j2a' in bu.tags:
            self.establish_arkjs_module()
            self.sh.run(f'cp -f {str(bu.doclet_src.parent)}/*.ets {bu.path}')
            bu.src_for_es2panda_override = Path(f'{bu.path}/*.ets')  # wildcard here assumes single ets file
            self.make_classes_abc(bu)
            self.es2abc(str(bu.src('.js').stem), str(bu.path))
            if self.dry_run_stop(bu):
                return
            self.arkjs_interop(bu)
            return

        log.warning('Valid generated interop mode not found in tags: %s',
                    ','.join(bu.tags))
        return

    def establish_arkjs_module(self) -> None:
        self.sh.run('mkdir -p module')
        self.sh.run('ln -sf $PANDA_BUILD/lib/module/ets_interop_js_napi_arkjsvm.so ' +
                    '$PANDA_BUILD/lib/arkjsvm_interop/libinterop_test_helper.so' +
                    ' -t module')

    def es2abc(self, js_file_name: str, path: str) -> None:
        self.sh.run('$PANDA_BUILD/bin/arkjsvm_interop/es2abc --module --merge-abc ' +
                    path + '/' + js_file_name + '.js' + ' --output=' + path + '/' + js_file_name + '.abc')

    def make_classes_abc(self, bu: BenchUnit) -> None:
        abc = bu.src('.abc')
        if not abc.is_file():
            self.es2panda(bu)
            abc = bu.src('.abc')
        abc.rename(abc.parent.joinpath('classes.abc'))
        self.sh.run(f'zip -r -0 {abc.with_suffix(".zip").name} classes.abc',
                    cwd=str(bu.path))

    def run_generated(self, bu: BenchUnit) -> None:
        self.make_classes_abc(bu)
        if self.dry_run_stop(bu):
            return
        self.arkjs_interop(bu)
