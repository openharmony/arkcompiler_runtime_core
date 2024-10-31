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
from typing import List

from vmb.platform import PlatformBase
from vmb.tool import ToolBase
from vmb.target import Target
from vmb.unit import BenchUnit
from vmb.cli import Args, OptFlags

log = logging.getLogger('vmb')


class Platform(PlatformBase):

    def __init__(self, args: Args) -> None:
        super().__init__(args)
        self.es2abc = self.tools_get('es2abc')
        self.ark_js_vm = self.tools_get('ark_js_vm')
        self.aot_compiler = self.tools_get('ark_aot_compiler')
        sdk = self.es2abc.sdk
        libark_ts = f'{sdk}/arkcompiler/ets_runtime/' \
            'ecmascript/ts_types/lib_ark_builtins.d.ts'
        self.libark_abc = ToolBase.libs.joinpath('lib_ark_builtins.d.abc')
        res = self.es2abc.run_es2abc(libark_ts, self.libark_abc)
        if not res or res.ret != 0:
            raise RuntimeError('lib_ark_builtins compilation failed')
        self.lang = list(self.langs)[0]
        self.push_libs()

    @property
    def name(self) -> str:
        return 'ArkHz on OHOS device'

    @property
    def target(self) -> Target:
        return Target.OHOS

    @property
    def required_tools(self) -> List[str]:
        return ['es2abc', 'ark_js_vm', 'ark_aot_compiler']

    @property
    def langs(self) -> List[str]:
        return self.args_langs if self.args_langs else ['ts']

    def run_unit(self, bu: BenchUnit) -> None:
        self.es2abc.exec_lang(bu, lang=self.lang)
        if self.dry_run_stop(bu):
            return
        self.push_unit(bu, '.abc')
        if OptFlags.AOT in self.flags:
            self.ark_js_vm.profile(bu)
            self.aot_compiler(bu)
            self.ark_js_vm.profile(bu, with_aot=True)
            self.aot_compiler(bu)
        self.ark_js_vm.exec(bu)
