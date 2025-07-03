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

import os
from pathlib import Path
from typing import ClassVar

from dotenv import load_dotenv

from runner.init_runner import InitRunner, MandatoryProps, MustExist, PropName
from runner.logger import Log

_LOGGER = Log.get_logger(__file__)

MandatoryPropDescription = list[tuple[PropName, MustExist]]


class RunnerEnv:
    mandatory_props: ClassVar[MandatoryPropDescription] = [
        ('ARKCOMPILER_RUNTIME_CORE_PATH', True),
        ('ARKCOMPILER_ETS_FRONTEND_PATH', True),
        ('PANDA_BUILD', True),
        ('WORK_DIR', False)
    ]
    urunner_path_name: ClassVar[str] = 'URUNNER_PATH'

    def __init__(self, *,
                 local_env: Path | None = None,
                 global_env: Path = InitRunner.urunner_env_default,
                 urunner_path: Path | None = None):
        self.global_env_path: Path = global_env
        self.local_env_path: Path | None = local_env
        self.urunner_path: Path | None = urunner_path

    @staticmethod
    def check_expand_mandatory_prop(var_name: PropName, must_exist: MustExist) -> None:
        var_value = os.getenv(var_name)
        if var_value is None:
            raise OSError(f"Mandatory environment variable '{var_name}' is not set. "
                          "Either specify it in the system environment or "
                          "run the runner with the only option `--init`")
        expanded = Path(var_value).expanduser().resolve()
        if must_exist and not expanded.exists():
            raise OSError(f"Mandatory environment variable '{var_name}' is set "
                          f"to value {var_value} which does not exist.")
        os.environ[var_value] = expanded.as_posix()

    @classmethod
    def get_mandatory_props(cls) -> MandatoryProps:
        result: MandatoryProps = {}
        for (prop_name, must_exist) in cls.mandatory_props:
            result[prop_name] = os.getenv(prop_name), must_exist
        return result

    def load_environment(self) -> None:
        self.load_home_env()
        self.load_local_env()

        for (prop, must_exist) in self.mandatory_props:
            self.check_expand_mandatory_prop(prop, must_exist)

        if self.urunner_path:
            os.environ[self.urunner_path_name] = self.urunner_path.as_posix()

    def load_local_env(self) -> None:
        """
        Loads the local environment file .env located near the main.py
        """
        if self.local_env_path and self.local_env_path.exists():
            load_dotenv(self.local_env_path)

    def load_home_env(self) -> None:
        """
        Loads the global environment file .urunner.env located at the user's home
        """
        if self.global_env_path.exists():
            load_dotenv(self.global_env_path)
