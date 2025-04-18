#!/usr/bin/env python3
# -- coding: utf-8 --
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


class RunnerException(Exception):
    pass


class DownloadException(RunnerException):
    pass


class UnzipException(RunnerException):
    pass


class InvalidConfiguration(RunnerException):
    pass


class ParameterNotFound(RunnerException):
    pass


class MacroNotExpanded(RunnerException):
    pass


class RunnerCustomSchemeException(RunnerException):
    pass


class TestNotExistException(RunnerException):
    pass


class StepUtilsException(RunnerException):
    pass


class MalformedStepConfigurationException(RunnerException):
    def __init__(self, message: str) -> None:
        self.full_message = f"Malformed configuration file: {message}"
        super().__init__(self, self.full_message)


class IncorrectEnumValue(RunnerException):
    pass


class IncorrectFileFormatChapterException(RunnerException):
    def __init__(self, chapters_name: str | Path) -> None:
        message = f"Incorrect file format: {chapters_name}"
        super().__init__(self, message)


class CyclicDependencyChapterException(RunnerException):
    def __init__(self, item: str) -> None:
        message = f"Cyclic dependency: {item}"
        super().__init__(self, message)


class UnknownException(RunnerException):
    pass


class SetupException(RunnerException):
    pass


class FileNotFoundException(RunnerException):
    pass


class YamlException(RunnerException):
    pass


class TimeoutException(RunnerException):
    pass
