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

from functools import cached_property
from typing import Dict, List

from runner.options.decorator_value import value, _to_bool, _to_time_edges, _to_str


class TimeReportOptions:
    __DEFAULT_EDGES = [1, 5, 10]

    def __str__(self) -> str:
        return _to_str(self, 1)

    def to_dict(self) -> Dict[str, object]:
        return {
            "enable": self.enable,
            "time-edges": self.time_edges,
        }

    @cached_property
    @value(yaml_path="time-report.enable", cli_name="time_report", cast_to_type=_to_bool)
    def enable(self) -> bool:
        return False

    @cached_property
    @value(yaml_path="time-report.time-edges", cli_name="time_edges", cast_to_type=_to_time_edges)
    def time_edges(self) -> List[int]:
        return TimeReportOptions.__DEFAULT_EDGES

    def get_command_line(self) -> str:
        _edges = ','.join(str(self.time_edges))
        options = [
            '--time-report' if self.enable else '',
            f'--time-edges="{_edges}"' if self.time_edges != TimeReportOptions.__DEFAULT_EDGES else ''
        ]
        return ' '.join(options)
