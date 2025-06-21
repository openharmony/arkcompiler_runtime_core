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
import os
import shutil
from copy import deepcopy
from pathlib import Path
from typing import List, Optional, Union, Dict, Any

from vmb.helpers import copy_file, copy_files, create_file_from_template
from vmb.gensettings import GenSettings
from vmb.platform import PlatformBase
from vmb.tool import ToolBase
from vmb.target import Target
from vmb.unit import BenchUnit
from vmb.cli import Args, OptFlags
from vmb.plugins.tools.es2panda import make_arktsconfig, fix_arktsconfig

log = logging.getLogger('vmb')

module_config: Dict[str, Any] = {
    "module": {
        "name": "entry",
        "type": "entry",
        "description": "$string:module_desc",
        "mainElement": "EmptyAbility",
        "deviceTypes": [
          "default",
          "tablet"
        ],
        "deliveryWithInstall": True,
        "installationFree": False,
        "pages": "$profile:main_pages",
        "virtualMachine": "ark",
        "abilities": [
        ]
    }
}

ability_config = {
    "name": "EmptyAbility",
    "srcEntry": "./ets/entryability/EmptyAbility.ts",
    "description": "$string:EntryAbility_desc",
    "icon": "$media:icon",
    "label": "$string:EntryAbility_label",
    "startWindowIcon": "$media:icon",
    "startWindowBackground": "$color:start_window_background",
    "exported": True,
    "skills": [
        {
            "entities": [
              "entity.system.home"
            ],
            "actions": [
              "ohos.want.action.home"
            ]
        }
    ]
}

ohos_dynamic_paths = {
    '@ohos.hilog': {
        'language': 'js',
        'ohmUrl': '@ohos.hilog'},
    "@ohos.app.ability.AbilityConstant": {
        'language': 'js',
        'ohmUrl': '@ohos.app.ability.AbilityConstant'},
    "@ohos.app.ability.UIAbility": {
        'language': 'js',
        'ohmUrl': '@ohos.app.ability.UIAbility'},
    "@ohos.app.ability.Want": {
        'language': 'js',
        'ohmUrl': '@ohos.app.ability.Want'},
    "@ohos.window": {
        'language': 'js',
        'ohmUrl': '@ohos.window'},
}


class Platform(PlatformBase):

    def __init__(self, args: Args) -> None:
        super().__init__(args)
        self.es2panda = self.tools_get('es2panda')
        self.hvigor = self.tools_get('hvigor')
        self.panda_sdk = ToolBase.ensure_dir_env('PANDA_SDK')
        self.etsstdlib = Path(self.panda_sdk).joinpath('ets', 'etsstdlib.abc')
        self.mode = args.mode
        self.sdk_version = 12  # hardcoded for now
        self.panda_root = self.es2panda.panda_root
        self.app_name = self.hvigor.app_name
        self.mod_name = self.hvigor.mod_name
        self.resource_dir = Path(
            os.path.realpath(os.path.dirname(__file__))).parent.parent.joinpath('resources', 'hap')
        self.hap_dir = ToolBase.libs.joinpath('hap')
        self.ability_dir = self.hap_dir.joinpath(self.mod_name, 'src', 'main', 'ets', 'entryability')
        self.lib_dir = self.hap_dir.joinpath(self.mod_name, 'libs', 'arm64-v8a')
        arktsconfig_json = self.es2panda.default_arktsconfig
        panda_src = ToolBase.ensure_dir_env('PANDA_SRC')
        make_arktsconfig(arktsconfig_json, panda_src, [])
        fix_arktsconfig(ohos_dynamic_paths)

    @property
    def name(self) -> str:
        return 'Application (hap) on OHOS'

    @property
    def target(self) -> Target:
        return Target.OHOS

    @property
    def template(self) -> Optional[GenSettings]:
        return GenSettings(src={'.ets'},
                           template='Hap.ets',
                           out='.ets',
                           link_to_src=False)

    @property
    def required_tools(self) -> List[str]:
        return ['es2panda', 'hvigor']

    @property
    def langs(self) -> List[str]:
        return ['ets']

    def run_ark_aot(self, abc: List[Union[str, Path]], an: Union[str, Path]) -> None:
        # not using Tool::paoc because of special case on Target.OHOS
        cmd = f'LD_LIBRARY_PATH={self.panda_root}/lib {self.panda_root}/bin/ark_aot ' \
              f'--boot-panda-files={self.etsstdlib} --paoc-mode=aot ' \
              '--load-runtimes=ets --compiler-cross-arch=arm64 ' \
              f'--paoc-panda-files={",".join([str(f) for f in abc])} --paoc-output={an}'
        ret = self.sh.run(cmd)
        if ret.ret != 0:
            raise RuntimeError(f"Paoc failed!\n{ret.out}\n{ret.err}")

    def copy_panda_libs(self, dst: Union[str, Path]) -> None:
        src = Path(self.panda_sdk).joinpath('ohos_arm64', 'lib')
        copy_files(src, dst, "*.so")
        copy_file(self.etsstdlib, Path(dst).joinpath('etsstdlib.abc.so'))

    def run_unit(self, bu: BenchUnit) -> None:
        ability_name = f'{bu.name}_Ability'
        self.hdc.run('power-shell wakeup')
        # unlock device
        self.hdc.run('uinput -T -m 1000 1000 1000 100')
        # hilog privacy off
        self.hdc.run('hilog -p off')
        finish_re = '|'.join((f'(Benchmark result: {bu.name})',
                              '(VMB MAIN FINISHED)',
                              '(ETS RUNTIME CREATE ERROR)'))
        res = self.hdc.run_syslog(
            cmd=f'aa start -a {ability_name} -b {self.app_name}',
            finished_marker=finish_re,
            timeout=20)
        self.hdc.run(f'aa force-stop {self.app_name}')
        bu.parse_run_output(res)

    def run_batch(self, bus: List[BenchUnit]) -> None:
        shutil.rmtree(self.hap_dir, ignore_errors=True)
        self.hap_dir.mkdir(exist_ok=True, parents=True)
        self.copy_panda_libs(self.lib_dir)
        # EtsRuntime
        runtime_ts = self.resource_dir.joinpath('EtsRuntime.ts')
        copy_file(runtime_ts, self.ability_dir.joinpath(runtime_ts.name))
        # EmptyAbility
        empty_src = self.resource_dir.joinpath('EmptyAbility.ts')
        empty_ts = self.ability_dir.joinpath(empty_src.name)
        copy_file(empty_src, empty_ts)
        # Add EmptyAbility to module.json5
        ability_template = self.resource_dir.joinpath('EntryAbility.ts.tpl')
        module_config_copy = deepcopy(module_config)
        try:
            module_config_copy['module']['abilities'].append(ability_config)
        except KeyError as e:
            raise RuntimeError('Corrupted module config') from e
        hap_unit = BenchUnit(self.hap_dir)
        abc_for_aot: List[Union[str, Path]] = []
        for bu in bus:
            self.es2panda(bu)
            abc = bu.src('.abc', die_on_zero_matches=True)
            so = ToolBase.rename_suffix(abc, '.abc.so')
            copy_file(so, self.lib_dir.joinpath(so.name))
            if OptFlags.AOT in self.flags:
                abc_for_aot.append(so)
            # Create bench ability file
            ability_ts = self.ability_dir.joinpath(f'{bu.name}_Ability.ts')
            create_file_from_template(ability_template,
                                      ability_ts,
                                      BENCH_NAME=bu.name,
                                      BENCH_FILE_SO=f'bench_{bu.name}.abc.so',
                                      BENCH_MODE=self.mode)
            # Add bench ability to module.json5
            bu_ability = deepcopy(ability_config)
            bu_ability['name'] = f'{bu.name}_Ability'
            bu_ability['srcEntry'] = f'./ets/entryability/{bu.name}_Ability.ts'
            try:
                module_config_copy['module']['abilities'].append(bu_ability)
            except KeyError as e:
                raise RuntimeError('Corrupted module config') from e
        if OptFlags.AOT in self.flags:
            self.run_ark_aot(abc=abc_for_aot, an=self.lib_dir.joinpath('aot_file.an.so'))
        # flush module.json5
        self.hvigor.emit_config(module_config_copy,
                                self.hap_dir.joinpath(self.mod_name, 'src', 'main', 'module.json5'))
        self.hvigor(hap_unit)
