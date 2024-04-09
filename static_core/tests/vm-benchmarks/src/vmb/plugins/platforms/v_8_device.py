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
from typing import List
from vmb.unit import BenchUnit
from vmb.platform import PlatformBase
from vmb.target import Target
from vmb.cli import Args, OptFlags

log = logging.getLogger('vmb')


class Platform(PlatformBase):

    def __init__(self, args: Args) -> None:
        super().__init__(args)
        self.tsc = self.tools['tsc'] if 'tsc' in self.required_tools else None
        self.v_8 = self.tools['v_8']

    def run_unit(self, bu: BenchUnit) -> None:
        if self.tsc:
            self.tsc(bu)
        if OptFlags.DRY_RUN in self.flags:
            return
        self.push_unit(bu)
        self.v_8(bu)

    @property
    def name(self) -> str:
        return 'V_8 engine on HOS device'

    @property
    def target(self) -> Target:
        return Target.DEVICE

    @property
    def required_tools(self) -> List[str]:
        return ['tsc', 'v_8'] if 'ts' in self.langs else ['v_8']

    @property
    def required_hooks(self) -> List[str]:
        return ['fix_print_call']

    @property
    def langs(self) -> List[str]:
        return self.args_langs if self.args_langs else ['ts', 'js']
