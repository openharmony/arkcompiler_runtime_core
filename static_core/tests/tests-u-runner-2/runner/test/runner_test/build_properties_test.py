#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2026 Huawei Device Co., Ltd.
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

from unittest import TestCase
from unittest.mock import Mock

from runner.test import test_utils
from runner.suites.test_lists import TestLists
from runner.common_exceptions import IncorrectEnumValue
from runner.options.config import Config
from runner.enum_types.configuration_kind import BuildTypeKind


class BuildPropTest(TestCase):

    @staticmethod
    def create_testlists(gn_build: bool, build_type: str) -> TestLists:
        build_prop = [f"CMAKE_BUILD_TYPE:STRING={build_type}"]
        obj = TestLists.__new__(TestLists)
        config = Mock(spec=Config)
        config.general = Mock()
        config.general.gn_build = gn_build
        obj.config = config
        obj.build_properties = build_prop
        return obj

    @test_utils.parametrized_test_cases([
        ("fast_verify", BuildTypeKind.FAST_VERIFY),
        ("fastverify", BuildTypeKind.FAST_VERIFY),
        ("fast-verify", BuildTypeKind.FAST_VERIFY),
        ("debug", BuildTypeKind.DEBUG),
        ("release", BuildTypeKind.RELEASE),
        ("RelWithDebInfo", BuildTypeKind.RELWITHDEBINFO),
        ("DebugDetailed", BuildTypeKind.DEBUGDETAILED),
        ("DEBUGDETAILED", BuildTypeKind.DEBUGDETAILED),
        ("relwithdebinfo", BuildTypeKind.RELWITHDEBINFO),
    ])
    def test_build_type(self, build_type: str, expected_build_type: BuildTypeKind) -> None:
        test_lists = BuildPropTest.create_testlists(gn_build=False, build_type=build_type)

        result = test_lists.search_build_type()
        self.assertEqual(result, expected_build_type)

    @test_utils.parametrized_test_cases([
        ("fast+verify",),
        ("",)
    ])
    def test_incorrect_build_type(self, build_type: str) -> None:
        test_lists = BuildPropTest.create_testlists(gn_build=False, build_type=build_type)

        with self.assertRaises(IncorrectEnumValue):
            test_lists.search_build_type()
