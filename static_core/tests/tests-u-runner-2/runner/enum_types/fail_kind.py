#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
from runner.enum_types.base_enum import BaseEnum


class FailureReturnCode(BaseEnum):
    SEGFAULT_RETURN_CODE = (139, -11)
    ABORT_RETURN_CODE = (134, -6)
    IRTOC_ASSERT_RETURN_CODE = (133, -5)


class FailKind(BaseEnum):
    FAIL = "fail"
    NEG_FAIL = "neg_fail"
    SEGFAULT = "segfault_fail"
    ABORT = "abort_fail"
    IRTOC_ASSERT = "irtoc_assert_fail"
    SUBPROCESS = "subprocess_fail"
    TIMEOUT = "timeout"
    PASSED = "passed"
    NOT_RUN = "not_run"
    OTHER = "other"

    def make_fail_kind(self, name: str) -> str:
        return make_fail_kind(name, self.value)


def make_fail_kind(name: str, suffix: str) -> str:
    """
    Make a fail kind from name and suffix.
    Name: usually a name of a step
    Suffix: usually a name of a step execution result (failure)
    """
    if not name:
        return suffix.upper()
    if not suffix:
        return name.upper()
    return f"{name.upper()}_{suffix.upper()}"
