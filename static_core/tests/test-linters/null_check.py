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

import yaml
from yamllint.linter import LintProblem

ID = "null-check"
TYPE = "token"
CONF = {}

WARNING_VALS = {"null", "~"}

def check(conf, token, prev, next, nextnext, context):

    # token.plain - check unquoted vals
    if isinstance(token, yaml.tokens.ScalarToken) and token.plain:
        if isinstance(prev, yaml.tokens.KeyToken):
            #  don't check keys
            return
        val = token.value
        if isinstance(val, str) and val.lower() in WARNING_VALS:
            yield LintProblem(
                token.start_mark.line + 1,
                token.start_mark.column + 1, "Unquoted null value", rule=ID)
