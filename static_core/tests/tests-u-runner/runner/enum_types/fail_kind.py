#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

from enum import Enum


class FailKind(Enum):
    ES2PANDA_FAIL = 1
    ES2PANDA_TIMEOUT = 4
    ES2PANDA_OTHER = 7

    RUNTIME_FAIL = 2
    RUNTIME_TIMEOUT = 5
    RUNTIME_OTHER = 8

    AOT_FAIL = 3
    AOT_TIMEOUT = 6
    AOT_OTHER = 9

    QUICK_FAIL = 10
    QUICK_TIMEOUT = 11
    QUICK_OTHER = 12

    VERIFIER_FAIL = 13
    VERIFIER_TIMEOUT = 14
    VERIFIER_OTHER = 15

    COMPARE_FAIL = 16
    COMPARE_OUTPUT_FAIL = 17

    TS_NODE_FAIL = 20
    TS_NODE_TIMEOUT = 21
    TS_NODE_OTHER = 22

    NODE_FAIL = 26
    NODE_TIMEOUT = 27
    NODE_OTHER = 28

    SEGFAULT_FAIL = 29
    ABORT_FAIL = 30
    IRTOC_ASSERT_FAIL = 31
    ES2PANDA_NEG_FAIL = 32
