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

from pathlib import Path
from typing import Any, Dict

from runner.plugins.ets.utils.test_parameters import parse_yaml

YAML_EXTENSIONS = ".yaml"
PARAM_SUFFIX = ".params"


class Params:
    def __init__(self, input_path: Path, bench_name: str) -> None:
        param_name = f"{bench_name}{PARAM_SUFFIX}{YAML_EXTENSIONS}"
        self.__param_path = input_path.parent / param_name

    def generate(self) -> Dict[str, Any]:
        if not self.__param_path.exists():
            return {}
        params: Dict[str, Any] = parse_yaml(str(self.__param_path))
        return params
