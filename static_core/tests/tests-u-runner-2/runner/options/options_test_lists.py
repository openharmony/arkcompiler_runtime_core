#!/usr/bin/env python3
# -- coding: utf-8 --
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
from typing import Any, cast

from runner.enum_types.configuration_kind import ArchitectureKind, BuildTypeKind, OSKind, SanitizerKind
from runner.options.options import IOptions


class TestListsOptions(IOptions):
    __DEFAULT_ARCH = ArchitectureKind.AMD64
    __DEFAULT_SAN = SanitizerKind.NONE
    __DEFAULT_OS = OSKind.LIN
    __DEFAULT_BUILD_TYPE = BuildTypeKind.FAST_VERIFY

    __PARAMETERS = "parameters"
    __TEST_LIST = "test-list"
    __TEST_FILE = "test-file"
    __SKIP_TEST_LISTS = "skip-test-lists"
    __UPDATE_EXCLUDED = "update-excluded"
    __UPDATE_EXPECTED = "update-expected"
    __TEST_LIST_ARCH = "test-list-arch"
    __TEST_LIST_SAN = "test-list-san"
    __TEST_LIST_OS = "test-list-os"
    __TEST_LIST_BUILD = "test-list-build"

    def __init__(self, parameters: dict[str, Any]): # type: ignore[explicit-any]
        super().__init__(parameters)
        self.__parameters = parameters

    def __str__(self) -> str:
        return self._to_str(indent=2)

    @staticmethod
    def add_cli_args(parser: argparse.ArgumentParser) -> None:
        # Test lists options
        parser.add_argument(
            f'--{TestListsOptions.__TEST_LIST}', default=None,
            help='run only the tests listed in this file')
        parser.add_argument(
            f'--{TestListsOptions.__TEST_FILE}', default=None,
            help='run only one test specified here')
        parser.add_argument(
            f'--{TestListsOptions.__SKIP_TEST_LISTS}', action='store_true', default=False,
            help='do not use ignored or excluded lists, run all available tests, report all found failures')
        parser.add_argument(
            f'--{TestListsOptions.__UPDATE_EXCLUDED}', action='store_true', default=False,
            help='update list of excluded tests - put all failed tests into default excluded test list')
        parser.add_argument(
            f'--{TestListsOptions.__UPDATE_EXPECTED}', action='store_true', default=False,
            help='update files with expected results')
        parser.add_argument(
            f'--{TestListsOptions.__TEST_LIST_ARCH}', action='store',
            default=TestListsOptions.__DEFAULT_ARCH,
            type=lambda arg: ArchitectureKind.is_value(arg, f"--{TestListsOptions.__TEST_LIST_ARCH}"),
            help='load specified architecture specific test lists. '
                 f'One of: {ArchitectureKind.values()}')
        parser.add_argument(
            f'--{TestListsOptions.__TEST_LIST_SAN}', action='store',
            default=TestListsOptions.__DEFAULT_SAN,
            type=lambda arg: SanitizerKind.is_value(arg, f"--{TestListsOptions.__TEST_LIST_SAN}"),
            help='load specified sanitizer specific test lists. '
                 f'One of {SanitizerKind.values()} where '
                 'asan - used on running against build with ASAN and UBSAN sanitizers), '
                 'tsan - used on running against build with TSAN sanitizers).')
        parser.add_argument(
            f'--{TestListsOptions.__TEST_LIST_OS}', action='store',
            default=TestListsOptions.__DEFAULT_OS,
            type=lambda arg: OSKind.is_value(arg, f"--{TestListsOptions.__TEST_LIST_OS}"),
            help='load specified operating system specific test lists. '
                 f'One of {OSKind.values()}')
        parser.add_argument(
            f'--{TestListsOptions.__TEST_LIST_BUILD}', action='store',
            default=TestListsOptions.__DEFAULT_BUILD_TYPE,
            type=lambda arg: BuildTypeKind.is_value(arg, f"--{TestListsOptions.__TEST_LIST_BUILD}"),
            help='load specified build type specific test lists. '
                 f'One of {BuildTypeKind.values()}')

    @cached_property
    def architecture(self) -> ArchitectureKind:
        if isinstance(self.__parameters[self.__TEST_LIST_ARCH], str):
            self.__parameters[self.__TEST_LIST_ARCH] = ArchitectureKind.is_value(
                value=self.__parameters[self.__TEST_LIST_ARCH],
                option_name=f"--{self.__TEST_LIST_ARCH}")
        return cast(ArchitectureKind, self.__parameters[self.__TEST_LIST_ARCH])

    @cached_property
    def build_type(self) -> BuildTypeKind:
        if isinstance(self.__parameters[self.__TEST_LIST_BUILD], str):
            self.__parameters[self.__TEST_LIST_BUILD] = BuildTypeKind.is_value(
                value=self.__parameters[self.__TEST_LIST_BUILD],
                option_name=f"--{self.__TEST_LIST_BUILD}")
        return cast(BuildTypeKind, self.__parameters[self.__TEST_LIST_BUILD])

    @cached_property
    def explicit_file(self) -> str | None:
        value = self.__parameters[self.__TEST_FILE]
        return str(value) if value is not None else value

    @cached_property
    def explicit_list(self) -> str | None:
        value = self.__parameters[self.__TEST_LIST]
        return str(value) if value is not None else value

    @cached_property
    def skip_test_lists(self) -> bool:
        return bool(self.__parameters[self.__SKIP_TEST_LISTS])

    @cached_property
    def update_excluded(self) -> bool:
        return bool(self.__parameters[self.__UPDATE_EXCLUDED])

    @cached_property
    def update_expected(self) -> bool:
        return bool(self.__parameters[self.__UPDATE_EXPECTED])

    def get_command_line(self) -> str:
        options = [
            f'--test-file="{self.explicit_file}"' if self.explicit_file is not None else '',
            f'--test-list="{self.explicit_list}"' if self.explicit_list is not None else '',
            '--skip-test-lists' if self.skip_test_lists else '',
            '--update-excluded' if self.update_excluded else '',
            '--update-expected' if self.update_expected else ''
        ]
        return ' '.join(options)
