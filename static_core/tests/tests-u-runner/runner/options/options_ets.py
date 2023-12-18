#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
# This file does only contain a selection of the most common options. For a
# full list see the documentation:
# http://www.sphinx-doc.org/en/master/config

from functools import cached_property
from typing import Dict

from runner.options.decorator_value import value, _to_str, _to_bool


class ETSOptions:
    def __str__(self) -> str:
        return _to_str(self, 1)

    def to_dict(self) -> Dict[str, object]:
        return {
            "force-generate": self.force_generate,
        }

    @cached_property
    @value(yaml_path="ets.force-generate", cli_name="is_force_generate", cast_to_type=_to_bool)
    def force_generate(self) -> bool:
        return False

    def get_command_line(self) -> str:
        return '--force-generate' if self.force_generate else ''
