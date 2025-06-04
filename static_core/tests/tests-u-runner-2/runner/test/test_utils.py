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

import unittest
from typing import Dict, Any, Union, TypeVar, Type


from runner import utils

MethodType = Any
CLASSTYPE = TypeVar("CLASSTYPE", bound='object')


def compare_dicts(test_case: unittest.TestCase, actual_dict: Dict[str, Any], expected_dict: Dict[str, Any]) -> None:
    test_case.assertEqual(len(actual_dict), len(expected_dict),
                          f"Actual dict: {actual_dict.keys()}, Expected dict: {expected_dict.keys()}")
    for key in actual_dict:
        test_case.assertTrue(key in expected_dict)
        actual_value = actual_dict[key]
        expected_value = expected_dict[key]
        type_actual_value = type(actual_value)
        type_expected_value = type(expected_value)
        if type_actual_value != type_expected_value and isinstance(actual_value, str):
            actual_value = try_to_cast_one_type(actual_value, expected_value)
            type_actual_value = type(actual_value)
        test_case.assertEqual(type_expected_value, type_actual_value,
                              f"Key '{key}': type of expected value = {type_expected_value}, "
                              f"type of actual value = {type_actual_value}. Expected to be equal")

        if actual_value and expected_value and isinstance(actual_value, dict):
            compare_dicts(test_case, actual_value, expected_value)
        elif isinstance(actual_value, list):
            test_case.assertListEqual(sorted(actual_value), sorted(expected_value))
        else:
            test_case.assertEqual(
                expected_value,
                actual_value,
                f"Key '{key}': expected value = {expected_value}, "
                f"actual value = {actual_value}. Expected to be equal")


def try_to_cast_one_type(value1: str, value2: Any) -> Union[int, bool, str]:
    if isinstance(value2, int):
        return int(value1)
    if isinstance(value2, bool):
        return utils.to_bool(value1)
    return value1


def get_method(cls: Type[CLASSTYPE], name: str) -> MethodType:
    for key, value in cls.__dict__.items():
        if key in [name, f"_{cls.__name__}{name}"] and callable(value):
            return value
    raise ValueError(f"Cannot find method '{name}' at class CliOptions")
