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


class ConfigurationKind(Enum):
    INT = "INT"
    OTHER_INT = "INT-OTHER"
    AOT = "AOT"
    AOT_FULL = "AOT-FULL"
    JIT = "JIT"
    QUICK = "QUICK"


class ArchitectureKind(Enum):
    ARM32 = "ARM32"
    ARM64 = "ARM64"
    AMD32 = "AMD32"
    AMD64 = "AMD64"  # default value


class SanitizerKind(Enum):
    NONE = ""
    ASAN = "ASAN"
    TSAN = "TSAN"


class OSKind(Enum):
    LIN = "LIN"  # default value
    WIN = "WIN"
    MAC = "MAC"


class BuildTypeKind(Enum):
    FAST_VERIFY = "FAST-VERIFY"  # default value
    DEBUG = "DEBUG"
    RELEASE = "RELEASE"
