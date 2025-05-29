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

import argparse
import sys
from glob import glob
from pathlib import Path
from typing import List, Optional, Dict, Union, Tuple

from runner import utils
from runner.common_exceptions import InvalidConfiguration
from runner.enum_types.config_type import ConfigType
from runner.logger import Log
from runner.options.options_general import GeneralOptions
from runner.options.options_test_suite import TestSuiteOptions

_LOGGER = Log.get_logger(__file__)

CliOptionType = Union[str, int, float, bool, list]
OptionType = Optional[Union[CliOptionType, dict, Path]]
ArgListType = List[str]


class CliOptions:
    __CFG_TYPE = "type"
    __CFG_PARAMETERS = "parameters"
    __CFG_RUNNER = "runner"

    def __init__(self, argv: ArgListType):
        self.data: Dict[str, OptionType] = {}
        if not argv or len(argv) < 2:
            raise InvalidConfiguration("No parameters specified. Expected at least workflow and test-suite")
        self.argv: ArgListType = argv
        self.__extract_configs()
        self.__process_runner_options(self.argv)

    @staticmethod
    def __parse_option(arg_list: ArgListType, option_name: str, default_value: Optional[CliOptionType]) \
            -> Tuple[Optional[CliOptionType], ArgListType]:
        """
        Search an option with `option_name` in `arg_list`.
        :param arg_list: list of strings incoming from argv (CLI)
        :param option_name: the name of the searched option
        :param default_value: the default value
        :return: If the option is found, its value is returned.
        The type of the returned value is the type of `default_value`.
        In other case the `default_value` is returned.
        """
        option_values: List[Optional[CliOptionType]] = []
        while CliOptions.__has_option(option_name, arg_list):
            option_value, arg_list = CliOptions.__parse_option_with_space(arg_list, option_name)
            if option_value is None:
                option_value, arg_list = CliOptions.__parse_option_with_equal(arg_list, option_name)
            option_values.append(option_value)
        if len(option_values) == 0 or (len(option_values) == 1 and option_values[0] is None):
            return default_value, arg_list
        if len(option_values) > 1 and not isinstance(default_value, list):
            return option_values[-1], arg_list
        if isinstance(default_value, list) and default_value:
            option_values.extend(default_value)
        return option_values[0] if len(option_values) == 1 else option_values, arg_list

    @staticmethod
    def __is_option_value_int(value: Optional[CliOptionType]) -> Tuple[bool, Optional[int]]:
        """
        Detects whether the value is int,
        if not tries to convert to int
        :param value: value get from CLI
        :return: the first item: True if returned value is int,
        the second item: the returned value if it's int or None
        """
        if isinstance(value, int):
            return True, int(value)
        if isinstance(value, str):
            try:
                return True, int(value)
            except (TypeError, ValueError):
                return False, None
        return False, None

    @staticmethod
    def __is_option_value_str(value: Optional[CliOptionType]) -> Tuple[bool, Optional[str]]:
        """
        Detects whether the value is str,
        :param value: value get from CLI
        :return: the first item: True if returned value is str,
        the second item: the returned value if it's str or None
        """
        if isinstance(value, str):
            return True, str(value)
        return False, None

    @staticmethod
    def __is_option_value_bool(value: Optional[CliOptionType]) -> Tuple[bool, Optional[bool]]:
        """
        Detects whether the value is bool,
        if not tries to convert to bool
        :param value: value get from CLI
        :return: the first item: True if returned value is bool,
        the second item: the returned value if it's bool or None
        """
        if isinstance(value, bool):
            return True, bool(value)
        if isinstance(value, str):
            if value.lower() == "true":
                return True, True
            if value.lower() == "false":
                return True, False
        return False, None

    @staticmethod
    def __get_option_value(option_name: str, value: Optional[CliOptionType]) -> CliOptionType:
        """
        Converts the value from any/str to specific type
        """
        result_tuple = CliOptions.__is_option_value_int(value)
        result: bool = result_tuple[0]
        res_value: Optional[CliOptionType] = result_tuple[1]
        if result and res_value is not None:
            return res_value
        result, res_value = CliOptions.__is_option_value_bool(value)
        if result and res_value is not None:
            return res_value
        result, res_value = CliOptions.__is_option_value_str(value)
        if result and res_value is not None:
            return res_value
        message = f"Invalid '{res_value}' value for the option '{option_name}'."
        raise InvalidConfiguration(message)

    @staticmethod
    def __has_option(option_name: str, arg_list: ArgListType) -> bool:
        for arg in arg_list:
            if arg == option_name or arg.startswith(f"{option_name}="):
                return True
        return False

    @staticmethod
    def __parse_option_with_space(arg_list: ArgListType, option_name: str) \
            -> Tuple[Optional[CliOptionType], ArgListType]:
        """
        Search the option `option_name` in the list of arguments and
        tries to get its value supposing it's divided with a space
        :param arg_list: list of arguments
        :param option_name: the searched option name
        :return: option value if found, in other case None as the first item,
        and the remaining list of arguments as the second item.
        """
        if option_name in arg_list:
            option_index = arg_list.index(option_name)
            if len(arg_list) > option_index + 1:
                option_value = CliOptions.__get_option_value(option_name, arg_list[option_index + 1])
                arg_list = arg_list[:option_index] + arg_list[option_index + 2:]
                return option_value, arg_list
            message = f"Missing {option_name} value."
            raise InvalidConfiguration(message)

        return None, arg_list

    @staticmethod
    def __parse_option_with_equal(arg_list: ArgListType, option_name: str) \
            -> Tuple[Optional[CliOptionType], ArgListType]:
        """
        Search the option `option_name` in the list of arguments and
        tries to get its value supposing it's divided with an equal sign `=`
        :param arg_list: list of arguments
        :param option_name: the searched option name
        :return: option value if found, in other case None as the first item,
        and the remaining list of arguments as the second item.
        """
        for index, arg in enumerate(arg_list):
            local_option_name = f"{option_name}="
            if arg.startswith(local_option_name):
                option_value = arg[len(local_option_name):]
                if option_value:
                    converted_option_value = CliOptions.__get_option_value(option_name, option_value)
                    arg_list = arg_list[:index] + arg_list[index + 1:]
                    return converted_option_value, arg_list
                message = f"Missing {option_name} value."
                raise InvalidConfiguration(message)
        return None, arg_list

    @staticmethod
    def __runner_options(runner_cli: ArgListType) -> argparse.Namespace:
        parser = argparse.ArgumentParser(description="General options parser")

        GeneralOptions.add_cli_args(parser)

        runner_args = parser.parse_args(runner_cli)
        return runner_args

    @staticmethod
    def __test_suite_options(options: ArgListType) -> argparse.Namespace:
        parser = argparse.ArgumentParser(description="Test suite options parser")

        TestSuiteOptions.add_cli_args(parser)

        test_suite_args = parser.parse_args(options)
        return test_suite_args

    def __process_workflow(self, cfg_name: str) -> None:
        self.__process_command(utils.get_config_workflow_folder(), cfg_name)

    def __process_test_suite(self, cfg_name: str) -> None:
        self.__process_command(utils.get_config_test_suite_folder(), cfg_name)

    def __process_command(self, folder: Path, cfg_name: str) -> None:
        cfg_path = folder.joinpath(cfg_name + '.yaml')
        try:
            data = utils.load_config(str(cfg_path))
        except FileNotFoundError as exc:
            existing_files = [Path(cfg).stem for cfg in glob(str(folder.joinpath("**/*.yaml")), recursive=True)]
            message = (f"Configuration file '{cfg_name}' not found. "
                       f"Expected one of: {sorted(existing_files)}")
            raise FileNotFoundError(message) from exc
        if (cfg_type := data.get(self.__CFG_TYPE)) in self.data:
            raise InvalidConfiguration(f"{cfg_name} redefines {cfg_type} config.")
        cli_params = data.get(self.__CFG_PARAMETERS, {})

        self.data[cfg_type] = cfg_name
        self.data[f"{cfg_name}.path"] = cfg_path
        self.data[f"{cfg_name}.data"] = data

        if cfg_type == ConfigType.TEST_SUITE.value:
            self.__init_test_suite_defaults(cfg_name)

        for key, default_value in cli_params.items():
            value, self.argv = self.__parse_option(self.argv, f"--{key}", default_value)
            self.data[f"{cfg_name}.{self.__CFG_PARAMETERS}.{key}"] = value

        if cfg_type == ConfigType.TEST_SUITE.value:
            self.__process_options_from_expected(cfg_name)

    def __process_options_from_expected(self, cfg_name: str) -> None:
        key = ""
        is_key = False
        n_processed = 0
        for index, arg in enumerate(self.argv[:]):
            if arg.startswith("--") and is_key:
                self.data[key] = True
                is_key = False
            if arg.startswith("--") and not is_key:
                key = f"{cfg_name}.{self.__CFG_PARAMETERS}.{utils.convert_underscore(arg[2:])}"
                if key not in self.data:
                    continue
                is_key = True
                self.argv.pop(index - n_processed)
                n_processed += 1
            elif not arg.startswith("--") and is_key:
                self.data[key] = arg
                is_key = False
                self.argv.pop(index - n_processed)
                n_processed += 1
            else:
                continue
        if is_key:
            self.data[key] = True

    def __init_test_suite_defaults(self, test_suite_name: str) -> None:
        test_suite_args = self.__test_suite_options([])
        for option in test_suite_args.__dict__:
            option_with_minus = utils.convert_underscore(option)
            key_with_minus = f"{test_suite_name}.{self.__CFG_PARAMETERS}.{option_with_minus}"
            self.data[key_with_minus] = test_suite_args.__dict__[option]

    def __process_runner_options(self, not_processed_args: ArgListType) -> None:
        runner_options = self.__runner_options(not_processed_args)
        for option in runner_options.__dict__:
            self.data[f"{self.__CFG_RUNNER}.{option}"] = runner_options.__dict__[option]

    def __extract_configs(self) -> None:
        workflow_cli = self.argv.pop(0)
        self.__process_workflow(workflow_cli)
        test_suite_cli = self.argv.pop(0)
        self.__process_test_suite(test_suite_cli)


def get_args() -> Dict[str, OptionType]:
    cli = CliOptions(sys.argv[1:])
    return cli.data
