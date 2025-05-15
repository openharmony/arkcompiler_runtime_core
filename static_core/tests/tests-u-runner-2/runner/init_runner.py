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
from typing import ClassVar

from runner.utils import write_2_file

MustExist = bool
PropName = str
PropValue = str
MandatoryProps = dict[PropName, tuple[PropValue | None, MustExist]]


class InitRunner:
    # Number of repeats to type a value for one variable
    attempts: ClassVar[int] = 3
    init_option: ClassVar[str] = "--init"
    urunner_env_default: ClassVar[Path] = Path.home() / ".urunner.env"

    def __init__(self, urunner_env: Path = urunner_env_default):
        self.urunner_env_path: Path = urunner_env

    @staticmethod
    def input_value(prop: str, must_exist: bool, attempts: int) -> PropValue:
        for i in range(1, attempts + 1):
            main_text = f"Attempt #{i} of {attempts}: type the value of the mandatory variable {prop}: "
            next_text = f"Empty or not-existing path are NOT suitable!\n{main_text}"
            value = input(main_text if i == 1 else next_text)
            if value and (Path(value).expanduser().exists() or not must_exist):
                return value
        raise FileNotFoundError(f"All {attempts} attempts to enter value for '{prop}' are out.")

    @classmethod
    def should_runner_initialize(cls, argv: list[str]) -> bool:
        return cls.init_option in argv

    def is_runner_initialized(self) -> bool:
        return self.urunner_env_path.exists()

    def initialize(self, mandatory_props: MandatoryProps) -> None:
        """
        Create a global environment file and save there the mandatory environment variables.
        If any mandatory variable is not specified (None or empty) or does not exist but must,
        it will ask the user to type path from the console
        """
        props: dict[PropName, PropValue] = {}
        for prop, (value, must_exist) in mandatory_props.items():
            if value is None:
                props[prop] = self.init_property(prop, value, must_exist)
            else:
                props[prop] = value

        result = [f"{prop}={value}" for prop, value in props.items()]
        write_2_file(self.urunner_env_path, "\n".join(result))

    def init_property(self, prop: str, value: str | None, must_exist: bool) -> str:
        """
        Initialize the property under name `prop` with provided `value`
        if it is None - ask for user to type the value from the console
        """
        if value is None or len(value) == 0 or (not Path(value).expanduser().exists() and must_exist):
            value = self.input_value(prop, must_exist, self.attempts)
            path_value = Path(value).expanduser().resolve()
            value = path_value.as_posix()
        return value
