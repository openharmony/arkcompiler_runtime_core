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
from typing import Dict, Optional, Any, cast

from runner.options.options import IOptions
from runner.utils import make_dir_if_not_exist


class CoverageOptions(IOptions):
    __USE_LLVM_COV = "use-llvm-cov"
    __LLVM_PROFDATA_OUT_PATH = "llvm-cov-profdata-out-path"
    __LLVM_COV_HTML_OUT_PATH = "llvm-cov-html-out-path"

    def __init__(self, args: Dict[str, Any]):
        super().__init__(args)
        self.__parameters = args

    def __str__(self) -> str:
        return self._to_str(indent=2)

    @staticmethod
    def add_cli_args(parser: argparse.ArgumentParser) -> None:
        parser.add_argument(
            f'--{CoverageOptions.__USE_LLVM_COV}', action='store_true', default=False)
        parser.add_argument(
            f'--{CoverageOptions.__LLVM_PROFDATA_OUT_PATH}', default=None,
            type=make_dir_if_not_exist,
            help='Directory where coverage intermediate files (*.profdata) are created.')
        parser.add_argument(
            f'--{CoverageOptions.__LLVM_COV_HTML_OUT_PATH}', default=None,
            type=make_dir_if_not_exist,
            help='Stacks files in the specified directory')

    @cached_property
    def use_llvm_cov(self) -> bool:
        return cast(bool, self.__parameters[self.__USE_LLVM_COV])

    @cached_property
    def llvm_profdata_out_path(self) -> Optional[str]:
        value = self.__parameters[self.__LLVM_PROFDATA_OUT_PATH]
        return str(value) if value is not None else value

    @cached_property
    def llvm_cov_html_out_path(self) -> Optional[str]:
        value = self.__parameters[self.__LLVM_COV_HTML_OUT_PATH]
        return str(value) if value is not None else value
