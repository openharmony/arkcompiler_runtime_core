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
import re
from functools import cached_property
from os import path
from typing import Dict, Any, List, Optional, Union

from runner.common_exceptions import InvalidConfiguration
from runner.logger import Log
from runner.options.macros import Macros, ParameterNotFound, MacroNotExpanded
from runner.options.options import IOptions
from runner.options.options_test_suite import TestSuiteOptions
from runner.options.step import Step, StepKind
from runner.utils import load_config, indent as utils_indent, has_macro, get_config_workflow_folder

_LOGGER = Log.get_logger(__file__)


class WorkflowOptions(IOptions):
    __WORKFLOW = "workflow"
    __WORKFLOW_NAME = "workflow-name"
    __PROPERTIES = "properties"
    __PARAMETERS = "parameters"
    __TEST_SUITE = "test-suite"
    __STEPS = "steps"
    __IMPORTS = "imports"
    __TYPE = "type"

    def __init__(self, cfg_content: Dict[str, Any], parent_test_suite: TestSuiteOptions,
                 *, parent_workflow: Optional['WorkflowOptions'] = None) -> None:
        super().__init__(None)
        self._parent = parent_test_suite if parent_workflow is None else parent_workflow
        inner = parent_workflow is not None
        self.__name = cfg_content[self.__WORKFLOW] if not inner else cfg_content[self.__WORKFLOW_NAME]
        self.__data = cfg_content[f"{self.__name}.data"] if not inner else cfg_content
        self.__test_suite = parent_test_suite
        self.__steps: List[Step] = []
        if not inner:
            self.__parameters = dict((arg_key[(len(self.__name) + len(self.__PARAMETERS) + 2):], arg_value)
                                     for arg_key, arg_value in cfg_content.items()
                                     if arg_key.startswith(f"{self.__name}.{self.__PARAMETERS}."))
            for param, value_in_workflow in self.__parameters.items():
                value_in_test_suite = self.__test_suite.get_parameter(param)
                if value_in_test_suite is not None:
                    self.__parameters[param] = value_in_test_suite
                    value_in_workflow = value_in_test_suite
                self.__parameters[param] = self.__expand_macro_for_param(value_in_workflow, param)
        else:
            self.__parameters = cfg_content[self.__PARAMETERS]
        self.__load_steps(self.__data[self.__STEPS])

    def __str__(self) -> str:
        indent = 2
        result = [f"{self.__name}\n{utils_indent(indent)}{self.__PARAMETERS}:"]
        for param_name in sorted(self.__parameters.keys()):
            param_value = self.__parameters[param_name]
            result.append(f"{utils_indent(indent + 1)}{param_name}: {param_value}")
        result.append(f"{utils_indent(indent)}{self.__STEPS}:")
        for step in self.__steps:
            result.append(str(step))

        return "\n".join(result)

    @staticmethod
    def __check_type(step_type: StepKind, actual_count: int, expected_max_count: int) -> None:
        if actual_count > expected_max_count:
            raise InvalidConfiguration(
                f"Property 'step-type: {step_type.value}' can be set at only one step, "
                f"but it is set at {actual_count} steps.")

    @cached_property
    def name(self) -> str:
        return str(self.__name)

    @cached_property
    def steps(self) -> List[Step]:
        return self.__steps

    @cached_property
    def parameters(self) -> Dict[str, Any]:
        return self.__parameters

    def get_command_line(self) -> str:
        options = ' '.join([
        ])
        options_str = re.sub(r'\s+', ' ', options, re.IGNORECASE | re.DOTALL)

        return options_str

    def get_parameter(self, key: str, default: Optional[Any] = None) -> Optional[Any]:
        return self.__parameters.get(key, default)

    def check_binary_artifacts(self) -> None:
        for step in self.steps:
            if step.executable_path is not None and not step.executable_path.is_file():
                raise FileNotFoundError(
                    f"Specified binary at {step.executable_path} was not found")

    def check_types(self) -> None:
        types: Dict[StepKind, int] = {}
        for step in self.steps:
            types[step.step_kind] = types.get(step.step_kind, 0) + 1
        self.__check_type(StepKind.COMPILER, types.get(StepKind.COMPILER, 0), 1)
        self.__check_type(StepKind.COMPILER, types.get(StepKind.COMPILER, 0), 1)
        self.__check_type(StepKind.VERIFIER, types.get(StepKind.VERIFIER, 0), 1)
        self.__check_type(StepKind.RUNTIME, types.get(StepKind.RUNTIME, 0), 1)

    def pretty_str(self) -> str:
        result: List[str] = [step.pretty_str() for step in self.steps if str(step.executable_path) and step.enabled]
        return '\n'.join(result)

    def __expand_macro_for_param(self, value_in_workflow: Union[str, List], param: str) -> Union[str, List]:
        if (isinstance(value_in_workflow, str) and has_macro(value_in_workflow) and
                not self.__test_suite.is_defined_in_collections(param)):
            return self.__expand_macro_for_str(value_in_workflow)
        if isinstance(value_in_workflow, list):
            return self.__expand_macro_for_list(value_in_workflow)
        return value_in_workflow

    def __prepare_imported_configs(self, imported_configs: Dict[str, Any]) -> None:
        for config_name, config_content in imported_configs.items():
            config_name = str(path.join(get_config_workflow_folder(), f"{config_name}.yaml"))
            args = {}
            for param, param_value in config_content.items():
                args.update(self.__prepare_imported_config(param, param_value))
            self.__load_imported_config(config_name, args)

    def __prepare_imported_config(self, param: str, param_value: Any) -> Dict[str, Any]:
        args = {}
        if isinstance(param_value, str) and param_value.find(self.__PARAMETERS) >= 0:
            param_value = param_value.replace(f"${{{self.__PARAMETERS}.", "").replace("}", "")
            args[param] = self.__parameters[param_value]
        elif isinstance(param_value, list):
            args[param] = self.__prepare_list(param_value)
        return args

    def __prepare_list(self, param_value: List[str]) -> List[str]:
        result_list = []
        for item in param_value:
            item = Macros.correct_macro(item, self)
            for sub_item in (item if isinstance(item, list) else [item]):
                if sub_item and sub_item in self.__parameters and self.__parameters[sub_item]:
                    result_list.append(self.__parameters[sub_item])
                elif item:
                    result_list.append(sub_item)
        return result_list

    def __load_imported_config(self, cfg_path: str, actual_params: Dict[str, Any]) -> None:
        cfg_content = load_config(str(cfg_path))
        params = cfg_content.get(self.__PARAMETERS, {})
        for param, _ in params.items():
            if param in actual_params:
                params[param] = actual_params[param]
        workflow_options = WorkflowOptions(cfg_content, self.__test_suite, parent_workflow=self)
        for step in workflow_options.steps:
            names = [st.name for st in self.__steps]
            if step.name not in [names]:
                self.__steps.append(step)
        for param_name, param_value in workflow_options.parameters.items():
            if param_name not in self.__parameters:
                self.__parameters[param_name] = param_value

    def __load_steps(self, steps: Dict[str, Any]) -> None:
        for step_name, step_content in steps.items():
            if step_name == self.__IMPORTS:
                self.__prepare_imported_configs(step_content)
            else:
                self.__load_step(step_name, step_content)

    def __load_step(self, step_name: str, step_content: Dict[str, Any]) -> None:
        _LOGGER.all(f"Going to load step '{step_name}'")
        for (step_item, step_value) in step_content.items():
            if isinstance(step_value, str):
                step_content[step_item] = Macros.correct_macro(step_value, self)
        new_args = []
        for arg in step_content['args']:
            arg = Macros.correct_macro(arg, self) \
                if not self.__test_suite.is_defined_in_collections(arg) else arg
            if isinstance(arg, list):
                new_args.extend(arg)
            else:
                new_args.append(arg)
        step_content['args'] = new_args
        step = Step(step_name, step_content)
        self.__steps.append(step)

    def __expand_macro_for_str(self, value_in_workflow: str) -> Union[str, List[str]]:
        try:
            return Macros.correct_macro(value_in_workflow, self)
        except ParameterNotFound as pnf:
            _LOGGER.all(str(pnf))
        except MacroNotExpanded as pnf:
            _LOGGER.all(str(pnf))
        return value_in_workflow

    def __expand_macro_for_list(self, value_in_workflow: list) -> Union[str, List[str]]:
        expanded_in_workflow: List[str] = []
        for value in value_in_workflow:
            try:
                expanded_value = Macros.correct_macro(value, self)
            except (ParameterNotFound, MacroNotExpanded) as pnf:
                _LOGGER.all(str(pnf))
                return value_in_workflow
            if isinstance(expanded_value, list):
                expanded_in_workflow.extend(expanded_value)
            else:
                expanded_in_workflow.append(expanded_value)
        return expanded_in_workflow
