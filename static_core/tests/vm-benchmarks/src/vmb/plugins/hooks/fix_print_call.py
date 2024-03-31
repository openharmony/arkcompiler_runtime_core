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

import re
import logging

from vmb.cli import Args
from vmb.unit import BenchUnit
from vmb.hook import HookBase

log = logging.getLogger('vmb')


class Hook(HookBase):
    """Change 'print()' -> 'console.log()' for Node."""

    def __init__(self, args: Args) -> None:
        super().__init__(args)
        self.regex = re.compile(r'(\W)(print\s*\()', re.S)

    @property
    def name(self) -> str:
        return 'Fix print function'

    def before_unit(self, bu: BenchUnit) -> None:
        for f in bu.path.glob('*'):
            if not (f.is_file() and f.suffix in {'.js', '.mjs', '.ts'}):
                continue
            src = f.read_text()
            patched = re.sub(
                self.regex,
                # pylint: disable-next=consider-using-f-string
                lambda match: '{0}console.log('.format(*match.group()),
                src)
            f.write_text(patched)
