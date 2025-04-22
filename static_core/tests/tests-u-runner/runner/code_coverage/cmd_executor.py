#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2021-2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the 'License');
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an 'AS IS' BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import subprocess
from pathlib import Path
from typing import List, TextIO, Union, Optional
from abc import ABC, abstractmethod


class CmdExecutor(ABC):
    @abstractmethod
    def run_command(self, command: List[str], stdout: Union[TextIO, int] = subprocess.PIPE) -> Union[str, int]:
        pass

    @abstractmethod
    def get_binary(self, binary_name: str, version: Optional[str] = None) -> Union[str, Path]:
        pass


class LinuxCmdExecutor(CmdExecutor):
    def run_command(self, command: List[str], stdout: Union[TextIO, int] = subprocess.PIPE) -> Union[str, int]:
        return subprocess.run(command, check=True, text=True, stdout=stdout, timeout=5400).stdout

    def get_binary(self, binary_name: str, version: Optional[str] = None) -> str:
        return f"{binary_name}-{version}" if version else binary_name
