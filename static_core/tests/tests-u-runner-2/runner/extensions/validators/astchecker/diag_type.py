# -*- coding: utf-8 -*-

# Copyright (c) 2026 Huawei Device Co., Ltd.
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

import re
from enum import unique

from runner.enum_types.base_enum import BaseEnum


# keep in sync with DiagnosticType in ets_frontend/ets2panda/util/diagnostic/diagnostic.h.erb
@unique
class DiagType(BaseEnum):
    ARKTS_CONF = "ArkTS config error"
    ERROR = "Error"
    FATAL = "Fatal"
    SEMANTIC = "Semantic error"
    SUGGESTION = "SUGGESTION"
    SYNTAX = "Syntax error"
    WARNING = "Warning"

    @staticmethod
    def pattern() -> re.Pattern[str]:
        values = "|".join(DiagType.values())
        pattern_line = rf"(?P<location>\[(?P<file>[^\s:]+):(?P<line>\d+):(?P<col>\d+)\]\s*)?(({values})\b.*)"
        return re.compile(pattern_line)

    def is_error(self) -> bool:
        return self != DiagType.WARNING
