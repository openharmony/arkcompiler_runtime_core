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
import sys
from pathlib import Path
from yamllint.cli import run
from yamllint.rules import _RULES
import null_check

_RULES[null_check.ID] = null_check

current_dir = Path(__file__).parent
default_cfg = current_dir / ".yamllint"
linter_args = sys.argv[1: ]
if "-c" not in linter_args and "--config-data" not in linter_args:
    linter_args.extend(["-c", str(default_cfg)])

run(linter_args)
