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
#

import argparse
from functools import cached_property
from typing import Dict, Any, List

from runner.options.options import IOptions


class ETSOptions(IOptions):
    __PARAMETERS = "parameters"
    __FORCE_GENERATE = "force-generate"
    __COMPARE_FILES = "compare-files"
    __COMPARE_FILES_ITERATIONS = "compare-files-iterations"
    __DEFAULT_COMPARE_FILES_ITERATIONS = 2

    def __init__(self, parameters: Dict[str, Any]):
        super().__init__(parameters)
        self.__parameters = parameters

    def __str__(self) -> str:
        return self._to_str(indent=2)

    @staticmethod
    def add_cli_args(parser: argparse.ArgumentParser) -> None:
        parser.add_argument(
            f'--{ETSOptions.__FORCE_GENERATE}', action='store_true', default=False,
            help='force generating the ETS test cases from templates')
        parser.add_argument(
            f'--{ETSOptions.__COMPARE_FILES}', action='store_true', default=False,
            help='switch on mode of comparing compiled abc files')
        parser.add_argument(
            f'--{ETSOptions.__COMPARE_FILES_ITERATIONS}', action='store_true',
            default=ETSOptions.__DEFAULT_COMPARE_FILES_ITERATIONS,
            help='number of comparing abc files. By default 2')

    @cached_property
    def force_generate(self) -> bool:
        return bool(self.__parameters[self.__FORCE_GENERATE])

    @cached_property
    def compare_files(self) -> bool:
        return bool(self.__parameters[self.__COMPARE_FILES])

    @cached_property
    def compare_files_iterations(self) -> int:
        return int(self.__parameters[self.__COMPARE_FILES_ITERATIONS])

    def get_command_line(self) -> str:
        options: List[str] = [
        ]
        return ' '.join(options)
