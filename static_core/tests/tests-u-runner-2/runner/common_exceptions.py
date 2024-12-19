#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
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

from pathlib import Path
from typing import Union


class DownloadException(Exception):
    pass


class UnzipException(Exception):
    pass


class InvalidConfiguration(Exception):
    pass


class ParameterNotFound(Exception):
    pass


class MacroNotExpanded(Exception):
    pass


class RunnerCustomSchemeException(Exception):
    pass


class TestNotExistException(Exception):
    pass


class StepUtilsException(Exception):
    pass


class MalformedStepConfigurationException(Exception):
    def __init__(self, message: str) -> None:
        self.full_message = f"Malformed configuration file: {message}"
        Exception.__init__(self, self.full_message)


class IncorrectEnumValue(Exception):
    pass


class IncorrectFileFormatChapterException(Exception):
    def __init__(self, chapters_name: Union[str, Path]) -> None:
        message = f"Incorrect file format: {chapters_name}"
        Exception.__init__(self, message)


class CyclicDependencyChapterException(Exception):
    def __init__(self, item: str) -> None:
        message = f"Cyclic dependency: {item}"
        Exception.__init__(self, message)


class UnknownException(Exception):
    pass


class SetupException(Exception):
    pass
