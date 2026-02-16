#!/usr/bin/env python3
# -- coding: utf-8 --
#
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
#
from dataclasses import dataclass
from pathlib import Path

from runner.common_exceptions import MalformedStepConfigurationException
from runner.enum_types.base_enum import BaseEnum


class ReqKind(BaseEnum):
    FILE_EXIST = "FileExist"
    FOLDER_EXIST = "FolderExist"
    PARENT_EXIST_CREATE = "ParentExistCreate"


ReqResult = bool
ReqFailure = str
ReqCheck = tuple[ReqResult, ReqFailure]

AllowedPreReqs: frozenset[ReqKind] = frozenset([ReqKind.FILE_EXIST, ReqKind.FOLDER_EXIST, ReqKind.PARENT_EXIST_CREATE])
AllowedPostReqs: frozenset[ReqKind] = frozenset([ReqKind.FILE_EXIST, ReqKind.FOLDER_EXIST])


@dataclass
class StepRequirements:

    def __init__(self, req: str, value: str):
        if (req_parsed := ReqKind.from_str(req)) is None:
            raise MalformedStepConfigurationException(f"Incorrect value `{req}` in step requirements. "
                                                      f"Expected one of {ReqKind.values()}")
        self.req: ReqKind = req_parsed
        self.value: str = value

    def __str__(self) -> str:
        return f"{self.req.value}: {self.value}"

    def __repr__(self) -> str:
        return self.__str__()

    def check(self, real_value: Path) -> ReqCheck:
        if self.req == ReqKind.FILE_EXIST:
            return self._check_file_exist(real_value)
        if self.req == ReqKind.FOLDER_EXIST:
            return self._check_folder_exist(real_value)
        if self.req == ReqKind.PARENT_EXIST_CREATE:
            return self._check_parent_exist_create(real_value)
        raise MalformedStepConfigurationException(f"Cannot process unknown requirement '{self.req.value}'")

    def _check_file_exist(self, real_value: Path) -> ReqCheck:
        result = real_value.is_file()
        failure = "" if result else (f"Requirement '{self.req}' with value '{real_value}' has failed. "
                                     f"It is either not a file or does not exist.")
        return result, failure

    def _check_folder_exist(self, real_value: Path) -> ReqCheck:
        result = real_value.is_dir()
        failure = "" if result else (f"Requirement '{self.req}' with value '{real_value}' has failed. "
                                     f"It is either not a folder or does not exist.")
        return result, failure

    def _check_parent_exist_create(self, real_value: Path) -> ReqCheck:
        real_value.parent.mkdir(parents=True, exist_ok=True)
        result = real_value.parent.exists()
        failure = "" if result else (f"Requirement '{self.req}' with value '{real_value}' has failed. "
                                     f"The parent folder '{real_value.parent}' does not exist.")
        return result, failure
