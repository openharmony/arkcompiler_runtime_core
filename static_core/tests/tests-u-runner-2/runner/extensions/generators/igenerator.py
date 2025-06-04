#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

from abc import ABC, abstractmethod
from pathlib import Path
from typing import List

from runner.common_exceptions import InvalidConfiguration
from runner.logger import Log
from runner.options.config import Config

_LOGGER = Log.get_logger(__file__)


class IGenerator(ABC):
    def __init__(self, source: Path, target: Path, config: Config) -> None:
        self._source: Path = Path(source)
        self._target: Path = Path(target)
        self._config: Config = config
        self.generated_tests: List[str] = []

        if not self._source.is_dir():
            raise InvalidConfiguration(f"{str(self._source.absolute())} must be a directory")

    @abstractmethod
    def generate(self) -> List[str]:
        pass
