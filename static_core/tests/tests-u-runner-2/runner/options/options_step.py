#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

import shutil
from collections.abc import Sequence
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, cast

from runner import utils
from runner.common_exceptions import IncorrectEnumValue, MalformedStepConfigurationException
from runner.enum_types.base_enum import BaseEnum
from runner.extensions.validators.ivalidator import IValidator
from runner.logger import Log
from runner.options.options import IOptions
from runner.options.step_reqs import AllowedPostReqs, AllowedPreReqs, StepRequirements
from runner.utils import get_validator_class, to_bool

_LOGGER = Log.get_logger(__file__)

PropListType = str | StepRequirements
RawStepData = dict[str, Any]  # type: ignore[explicit-any]
PropType = str | int | bool | RawStepData | BaseEnum | list[PropListType]


class StepKind(BaseEnum):
    COMPILER = "compiler"
    VERIFIER = "verifier"
    AOT = "aot"
    RUNTIME = "runtime"
    GTEST_RUNNER = "gtest-runner"
    DECLGEN = "declgen"
    TSC = "tsc"
    OTHER = "other"
    NONE = ""


@dataclass(frozen=True)
class Step(IOptions):
    """
    Represents a single execution step in a test workflow.
    
    The Step class encapsulates all configuration and execution parameters for a single
    step within a test workflow, including executable path, arguments, environment variables,
    timeout settings, and validation requirements. Each step can be of different kinds
    (compiler, verifier, AOT, runtime, etc.) and supports pre/post requirements validation.
    
    Note: This class replaces the deprecated BinaryParams class which was removed.
    All functionality previously provided by BinaryParams is now handled by
    the Step class, providing a more comprehensive and unified approach to step configuration.
    
    The Step class provides methods for:
    - Configuration parsing and validation from raw dictionary data
    - Requirement checking (pre and post execution)
    - String representation for logging and debugging
    - Pretty formatting for command-line display
    """
    name: str
    timeout: int
    args: list[str]
    env: dict[str, str]
    step_kind: StepKind
    step_filter: str = "*"
    enabled: bool = True
    executable_path: Path | None = None
    can_be_instrumented: bool = False
    skip_qemu: bool = False
    pre_requirements: list[StepRequirements] = field(default_factory=list)
    post_requirements: list[StepRequirements] = field(default_factory=list)
    validator_class: str | None = None
    validator: IValidator | None = None
    stdout: Path | None = None
    stderr: Path | None = None
    passed: bool | None = None

    __DEFAULT_TIMEOUT = 30
    __MIN_ARG_LENGTH = 15
    __MAX_ARG_LENGTH = 150
    __INDENT_STEP = "\t"
    __INDENT_ARG = __INDENT_STEP * 2
    __INDENT_SUB_ARG = __INDENT_STEP * 3

    def __str__(self) -> str:
        indent = 3
        result = [f"{utils.indent(indent)}{self.name}:"]
        indent += 1
        if self.pre_requirements:
            result += self.__reqs_to_str(indent, "pre-reqs", self.pre_requirements)
        result += Step.__nullable_prop_to_str(indent, "executable-path",
                                              self.executable_path.as_posix() if self.executable_path else None)
        result += [f"{utils.indent(indent)}enabled: {self.enabled}"]
        result += [f"{utils.indent(indent)}step-type: {self.step_kind.value}"]
        result += [f"{utils.indent(indent)}timeout: {self.timeout}"]
        result += [f"{utils.indent(indent)}can-be-instrumented: {self.can_be_instrumented}"]
        result += [f"{utils.indent(indent)}skip-qemu: {self.skip_qemu}"]
        result += [f"{utils.indent(indent)}step-filter: '{self.step_filter}'"]
        result += Step.__nullable_prop_to_str(indent, "validator", self.validator_class)
        result += Step.__nullable_prop_to_str(indent, "stdout", self.stdout.as_posix() if self.stdout else None)
        result += Step.__nullable_prop_to_str(indent, "stderr", self.stderr.as_posix() if self.stderr else None)

        result += self.__args_to_str(indent)
        result += self.__env_to_str(indent)

        if self.post_requirements:
            result += self.__reqs_to_str(indent, "post-reqs", self.post_requirements)

        return "\n".join(result)

    @staticmethod
    def create(name: str, step_body: RawStepData) -> 'Step':
        validator_class, validator = Step.__get_validator(step_body, 'validator')
        return Step(
            name,
            timeout=Step.__get_int_property(step_body, 'timeout', Step.__DEFAULT_TIMEOUT),
            args=Step.__get_list_of_str(step_body, 'args', []),
            env=Step.__get_dict_property(step_body, 'env', {}),
            step_kind=Step.__get_kind_property(step_body, 'step-type', StepKind.OTHER),
            step_filter=Step.__get_str_property(step_body, 'step-filter', '*'),
            enabled=Step.__get_bool_property(step_body, 'enabled', True),
            executable_path=Step.__get_path_property(step_body, 'executable-path'),
            can_be_instrumented=Step.__get_bool_property(step_body, 'can-be-instrumented', False),
            skip_qemu=Step.__get_bool_property(step_body, 'skip-qemu', False),
            pre_requirements=Step.__get_pre_reqs(step_body),
            post_requirements=Step.__get_post_reqs(step_body),
            validator_class=validator_class,
            validator=validator,
            stdout=Step.__get_path_property(step_body, 'stdout'),
            stderr=Step.__get_path_property(step_body, 'stderr')
        )

    @staticmethod
    def _check_requirements(reqs: list[StepRequirements]) -> tuple[bool, list[str]]:
        failures: list[str] = []
        final_result = True
        for req in reqs:
            result, failure = req.check(Path(req.value))
            final_result = final_result and result
            if failure:
                failures.append(failure)
        return len(failures) == 0, failures

    @staticmethod
    def __get_int_property(step_body: RawStepData, name: str, default: int) -> int:
        value = Step.__get_property(step_body, name, default)
        try:
            return int(cast(str, value))
        except ValueError as exc:
            raise MalformedStepConfigurationException(f"Incorrect value '{value}' for property '{name}'") from exc

    @staticmethod
    def __get_bool_property(step_body: RawStepData, name: str, default: bool) -> bool:
        value = Step.__get_property(step_body, name, default)
        try:
            return to_bool(value)
        except ValueError as exc:
            raise MalformedStepConfigurationException(f"Incorrect value '{value}' for property '{name}'") from exc

    @staticmethod
    def __get_list_of_str(step_body: RawStepData, name: str, default: list[PropListType]) -> list[str]:
        return cast(list[str], Step.__get_list_property(step_body, name, default, str))

    @staticmethod
    def __get_pre_reqs(step_body: RawStepData) -> list[StepRequirements]:
        reqs = Step.__get_list_of_req(step_body, 'pre-reqs', [])
        for req in reqs:
            if req.req not in AllowedPreReqs:
                raise MalformedStepConfigurationException(f"Requirement {req.req.value} is not allowed as pre-req")
        return reqs

    @staticmethod
    def __get_post_reqs(step_body: RawStepData) -> list[StepRequirements]:
        reqs = Step.__get_list_of_req(step_body, 'post-reqs', [])
        for req in reqs:
            if req.req not in AllowedPostReqs:
                raise MalformedStepConfigurationException(f"Requirement {req.req.value} is not allowed as post-req")
        return reqs

    @staticmethod
    def __get_list_of_req(step_body: RawStepData, name: str, default: list[PropListType]) -> \
            list[StepRequirements]:
        result = Step.__get_list_property(step_body, name, default, StepRequirements)
        return cast(list[StepRequirements], result)

    @staticmethod
    def __get_validator(step_body: RawStepData, name: str) -> \
            tuple[str | None, IValidator | None]:
        clazz = Step.__get_nullable_str_property(step_body, name)
        if clazz is not None:
            return clazz, get_validator_class(clazz)()
        return None, None

    @staticmethod
    def __get_list_property(step_body: RawStepData, name: str, default: list[PropListType],
                            real_type: type[PropListType] = str) -> Sequence[PropListType]:
        value = Step.__get_property(step_body, name, default)
        if not isinstance(value, list):
            raise MalformedStepConfigurationException(f"Incorrect value '{value}' for property '{name}'. "
                                                      "Expected list.")
        if real_type is StepRequirements:
            return [StepRequirements.create(**v) for v in cast(dict, value)]

        return cast(Sequence[str], value)

    @staticmethod
    def __get_dict_property(step_body: RawStepData, name: str, default: RawStepData) -> dict[str, str]:
        value = Step.__get_property(step_body, name, default)
        if not isinstance(value, dict):
            raise MalformedStepConfigurationException(f"Incorrect value '{value}' for property '{name}'. "
                                                      "Expected dict.")
        return cast(dict[str, str], value)

    @staticmethod
    def __get_str_property(step_body: RawStepData, name: str, default: str) -> str:
        value = Step.__get_property(step_body, name, default)
        if not isinstance(value, str):
            raise MalformedStepConfigurationException(f"Incorrect value '{value}' for property '{name}'. "
                                                      "Expected str.")
        return value

    @staticmethod
    def __get_nullable_str_property(step_body: RawStepData, name: str, default: str | None = None) -> str | None:
        value = Step.__get_nullable_property(step_body, name, default)
        if value is not None and not isinstance(value, str):
            raise MalformedStepConfigurationException(f"Incorrect value '{value}' for property '{name}'. Expected str.")
        return value

    @staticmethod
    def __get_path_property(step_body: RawStepData, name: str) -> Path | None:
        """
        Extract and resolve path property from step configuration.
        
        If the value is just an executable name (no directory separators), searches for it
        in the PATH environment variable and returns the full path if found.
        If the value contains path separators or is not found in PATH, returns it as-is.
        
        Args:
            step_body: Dictionary containing step configuration data
            name: Name of the property to extract
            
        Returns:
            Path object if value exists, None otherwise. For executable names without
            path separators, returns the full path from PATH if found.
        """
        value: str | None = Step.__get_nullable_str_property(step_body, name)
        if not value:
            return None

        p_value = Path(value)
        if p_value.name == p_value.as_posix():
            if (from_path := shutil.which(p_value.name)) is None:
                raise MalformedStepConfigurationException(f"Path '{value}' does not exist or is not found in PATH.")
            return Path(from_path)
        return p_value

    @staticmethod
    def __get_kind_property(step_body: RawStepData, name: str, default: StepKind = StepKind.OTHER) -> StepKind:
        value = Step.__get_property(step_body, name, default)
        try:
            if isinstance(value, StepKind):
                return value
            return StepKind.is_value(cast(str, value), option_name="'step-type' from workflow config")
        except IncorrectEnumValue as exc:
            raise MalformedStepConfigurationException(f"Incorrect value '{value}' for property '{name}'. "
                                                      f"Expected one of {StepKind.values()}.") from exc

    @staticmethod
    def __get_property(step_body: RawStepData, name: str, default: PropType) -> PropType:
        value: PropType | None = Step.__get_nullable_property(step_body, name, default)
        if value is None:
            raise MalformedStepConfigurationException(f"missed required field '{name}'")
        return value

    @staticmethod
    def __get_nullable_property(step_body: RawStepData, name: str, default: PropType | None = None) -> PropType | None:
        if name in step_body:
            value = step_body[name]
            if value is None and default is not None:
                value = default
        else:
            value = default
        return cast(PropType, value) if value is not None else None

    @staticmethod
    def __reqs_to_str(indent: int, name: str, reqs: list[StepRequirements]) -> list[str]:
        result = [f"{utils.indent(indent)}{name}:"]
        for req in reqs:
            result += [
                f"{utils.indent(indent + 1)}- req: {req.req.value}",
                f"{utils.indent(indent + 1)}  value: {req.value}"
            ]
        return result

    @staticmethod
    def __nullable_prop_to_str(indent: int, name: str, value: str | None) -> list[str]:
        if not value:
            return []
        return [f"{utils.indent(indent)}{name}: {value}"]

    def check_pre_requirements(self) -> tuple[bool, list[str]]:
        return self._check_requirements(self.pre_requirements)

    def check_post_requirements(self) -> tuple[bool, list[str]]:
        return self._check_requirements(self.post_requirements)

    def executable_path_exists(self) -> bool:
        if self.executable_path is None:
            return False
        is_exist = self.executable_path.exists() and self.executable_path.is_file()
        is_in_path = (self.executable_path.name == self.executable_path.as_posix() and
                      shutil.which(self.executable_path.name) is not None)
        return is_exist or is_in_path

    def pretty_str(self) -> str:
        if not str(self.executable_path):
            return ''
        result = f"Step '{self.name}':"
        args: list[str] = [self.__pretty_arg_str(arg) for arg in self.args]
        args_str = '\n'.join(args)
        result += f"\n{self.__INDENT_STEP}{self.executable_path}\n{args_str}"
        return result

    def __pretty_arg_str(self, arg: str) -> str:
        args: list[str] = []
        arg = str(arg)
        if len(arg) < self.__MAX_ARG_LENGTH:
            args.append(f"{self.__INDENT_ARG}{arg}")
        else:
            sub_args = arg.split(" ")
            args.extend(self.__pretty_sub_arg(sub_args))
        return '\n'.join(args)

    def __pretty_sub_arg(self, sub_args: list[str]) -> list[str]:
        acc = ''
        result: list[str] = []
        for sub_arg in sub_args:
            if len(sub_arg) > self.__MIN_ARG_LENGTH:
                if len(acc) > 0:
                    result.append(f"{self.__INDENT_SUB_ARG}{acc}")
                    acc = ""
                result.append(f"{self.__INDENT_SUB_ARG}{sub_arg}")
            else:
                acc += sub_arg + " "
        if len(acc) > 0:
            result.append(f"{self.__INDENT_SUB_ARG}{acc}")
        return result

    def __env_to_str(self, indent: int) -> list[str]:
        if not self.env:
            return []
        result = [f"{utils.indent(indent)}env:"]
        for key in self.env:
            env_value: str | list[str] = self.env[key]
            if isinstance(env_value, list):
                result += [f"{utils.indent(indent + 1)}{key}:"]
                for item in env_value:
                    result += [f"{utils.indent(indent + 2)}- {item}"]
            else:
                result += [f"{utils.indent(indent + 1)}{key}: {env_value}"]
        return result

    def __args_to_str(self, indent: int) -> list[str]:
        if not self.args:
            return []
        result = [f"{utils.indent(indent)}args:"]
        for arg in self.args:
            if arg.strip():
                result += [f"{utils.indent(indent + 1)}- '{arg}'"]
        return result
