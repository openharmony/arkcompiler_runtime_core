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

import argparse
import logging
from typing import Optional, List, Union

from runner.logger import Log

_LOGGER = logging.getLogger("runner.options.cli_args_wrapper")


class CliArgsWrapper:
    _args = None

    @staticmethod
    def setup(args: argparse.Namespace) -> None:
        CliArgsWrapper._args = args

    @staticmethod
    def get_by_name(name: str) -> Optional[Union[str, List[str]]]:
        if name in dir(CliArgsWrapper._args):
            value = getattr(CliArgsWrapper._args, name)
            if value is None:
                return None
            if isinstance(value, list):
                return value
            return str(value)

        Log.exception_and_raise(_LOGGER, f"Cannot recognize CLI option {name} ")
