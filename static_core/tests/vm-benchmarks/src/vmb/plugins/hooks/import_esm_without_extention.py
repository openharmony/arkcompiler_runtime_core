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

from vmb.unit import BenchUnit
from vmb.hook import HookBase

log = logging.getLogger('vmb')


class Hook(HookBase):
    """Create symlink libX -> libX.mjs for Node."""

    @property
    def name(self) -> str:
        return 'Symlink .mjs to ESM'

    def before_unit(self, bu: BenchUnit) -> None:
        for lib in {js.with_suffix('')
                    for js in bu.libs('.mjs', '.ts')}:
            lib.symlink_to(lib.with_suffix('.mjs').resolve())
